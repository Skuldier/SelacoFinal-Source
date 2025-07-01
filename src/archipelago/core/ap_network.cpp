#include "ap_network.h"
#include "apclient.hpp"
#include "apuuid.hpp"
#include <nlohmann/json.hpp>
#include "common/console/c_console.h"

namespace Archipelago {

APNetworkClient::APNetworkClient()
    : connection_status_(ConnectionStatus::Disconnected)
    , should_stop_(false)
{
}

APNetworkClient::~APNetworkClient()
{
    Disconnect();
}

bool APNetworkClient::Connect(const std::string& uri, const std::string& game_name)
{
    if (connection_status_ != ConnectionStatus::Disconnected)
    {
        return false;
    }
    
    server_uri_ = uri;
    game_name_ = game_name;
    
    try
    {
        // Generate UUID for this session
        std::string uuid = ap_get_uuid(game_name);
        
        // Create APClient instance
        ap_client_ = std::make_unique<APClient>(uuid, game_name, server_uri_);
        
        // Set up callbacks
        SetupCallbacks();
        
        // Start network thread
        should_stop_ = false;
        network_thread_ = std::make_unique<std::thread>(&APNetworkClient::NetworkThreadMain, this);
        
        return true;
    }
    catch (const std::exception& e)
    {
        Printf("Archipelago: Failed to create client: %s\n", e.what());
        return false;
    }
}

void APNetworkClient::Disconnect()
{
    should_stop_ = true;
    network_cv_.notify_all();
    
    if (network_thread_ && network_thread_->joinable())
    {
        network_thread_->join();
    }
    
    ap_client_.reset();
    connection_status_ = ConnectionStatus::Disconnected;
}

bool APNetworkClient::Authenticate(const std::string& slot_name, const std::string& password)
{
    if (!ap_client_ || connection_status_ != ConnectionStatus::Connected)
    {
        return false;
    }
    
    slot_name_ = slot_name;
    password_ = password;
    
    NetworkCommand cmd;
    cmd.type = NetworkCommand::Authenticate;
    cmd.data["slot"] = slot_name;
    cmd.data["password"] = password;
    
    EnqueueCommand(cmd);
    return true;
}

void APNetworkClient::ProcessNetworkEvents()
{
    // This is called from the main thread
    // Process any callbacks that need to happen on the main thread
}

void APNetworkClient::SendLocationChecks(const std::vector<int64_t>& locations)
{
    NetworkCommand cmd;
    cmd.type = NetworkCommand::CheckLocations;
    cmd.data["locations"] = locations;
    EnqueueCommand(cmd);
}

void APNetworkClient::SendChat(const std::string& message)
{
    NetworkCommand cmd;
    cmd.type = NetworkCommand::SendChat;
    cmd.data["message"] = message;
    EnqueueCommand(cmd);
}

void APNetworkClient::NetworkThreadMain()
{
    while (!should_stop_)
    {
        // Process commands
        ProcessCommands();
        
        // Poll APClient
        if (ap_client_)
        {
            ap_client_->poll();
        }
        
        // Small sleep to prevent busy waiting
        std::unique_lock<std::mutex> lock(network_mutex_);
        network_cv_.wait_for(lock, std::chrono::milliseconds(10),
            [this] { return should_stop_ || !command_queue_.empty(); });
    }
}

void APNetworkClient::ProcessCommands()
{
    std::unique_lock<std::mutex> lock(command_mutex_);
    
    while (!command_queue_.empty())
    {
        NetworkCommand cmd = command_queue_.front();
        command_queue_.pop();
        lock.unlock();
        
        // Process command
        switch (cmd.type)
        {
        case NetworkCommand::Authenticate:
            if (ap_client_ && connection_status_ == ConnectionStatus::Connected)
            {
                connection_status_ = ConnectionStatus::Authenticating;
                ap_client_->ConnectSlot(
                    cmd.data["slot"].get<std::string>(),
                    cmd.data["password"].get<std::string>(),
                    2, // items_handling (all items)
                    {"AP_VERSION"},
                    {0, 5, 0}  // version
                );
            }
            break;
            
        case NetworkCommand::CheckLocations:
            if (ap_client_ && connection_status_ == ConnectionStatus::Authenticated)
            {
                ap_client_->LocationChecks(cmd.data["locations"].get<std::vector<int64_t>>());
            }
            break;
            
        case NetworkCommand::SendChat:
            if (ap_client_ && connection_status_ == ConnectionStatus::Authenticated)
            {
                ap_client_->Say(cmd.data["message"].get<std::string>());
            }
            break;
        }
        
        lock.lock();
    }
}

void APNetworkClient::EnqueueCommand(const NetworkCommand& cmd)
{
    {
        std::lock_guard<std::mutex> lock(command_mutex_);
        command_queue_.push(cmd);
    }
    network_cv_.notify_one();
}

void APNetworkClient::SetupCallbacks()
{
    if (!ap_client_)
        return;
        
    // Socket connected
    ap_client_->set_socket_connected_handler([this]() {
        connection_status_ = ConnectionStatus::Connected;
        if (on_connection_status_changed)
            on_connection_status_changed(ConnectionStatus::Connected);
    });
    
    // Socket disconnected
    ap_client_->set_socket_disconnected_handler([this]() {
        connection_status_ = ConnectionStatus::Disconnected;
        if (on_connection_status_changed)
            on_connection_status_changed(ConnectionStatus::Disconnected);
    });
    
    // Slot connected (authenticated)
    ap_client_->set_slot_connected_handler([this](const nlohmann::json& slot_data) {
        connection_status_ = ConnectionStatus::Authenticated;
        if (on_connection_status_changed)
            on_connection_status_changed(ConnectionStatus::Authenticated);
    });
    
    // Slot refused
    ap_client_->set_slot_refused_handler([this](const std::vector<std::string>& errors) {
        connection_status_ = ConnectionStatus::ConnectionRefused;
        if (on_connection_status_changed)
            on_connection_status_changed(ConnectionStatus::ConnectionRefused);
    });
    
    // Items received
    ap_client_->set_items_received_handler([this](const std::vector<APClient::NetworkItem>& items) {
        std::vector<NetworkItem> converted_items;
        for (const auto& ap_item : items)
        {
            NetworkItem item;
            item.item_id = ap_item.item;
            item.location_id = ap_item.location;
            item.player_id = ap_item.player;
            item.flags = ap_item.flags;
            converted_items.push_back(item);
        }
        
        if (on_items_received)
            on_items_received(converted_items);
    });
    
    // Print/chat messages
    ap_client_->set_print_json_handler([this](const nlohmann::json& data) {
        if (on_print_json)
            on_print_json(data);
    });
}

bool APNetworkClient::IsConnected() const
{
    return connection_status_ == ConnectionStatus::Authenticated;
}

ConnectionStatus APNetworkClient::GetConnectionStatus() const
{
    return connection_status_;
}

} // namespace Archipelago