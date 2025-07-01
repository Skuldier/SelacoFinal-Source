#pragma once

#include "ap_types.h"
#include <memory>
#include <atomic>
#include <mutex>

namespace Archipelago {

// Forward declarations
class APNetworkClient;
class APStateManager;
class APProtocolHandler;

class Manager {
public:
    // Singleton pattern with lazy initialization
    static Manager& GetInstance();
    
    // Delete copy/move constructors
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;
    Manager(Manager&&) = delete;
    Manager& operator=(Manager&&) = delete;
    
    // Initialization and shutdown
    bool Initialize(const Config& config = Config{});
    void Shutdown();
    
    // Connection management
    bool Connect(const std::string& server, const std::string& slot_name, 
                 const std::string& password = "");
    void Disconnect();
    bool IsConnected() const;
    ConnectionStatus GetConnectionStatus() const;
    
    // Game actions
    void CheckLocation(int64_t location_id);
    void CheckLocations(const std::vector<int64_t>& location_ids);
    void ScoutLocation(int64_t location_id);
    void ScoutLocations(const std::vector<int64_t>& location_ids);
    
    // Communication
    void SendChat(const std::string& message);
    void SendDeathLink(const std::string& cause = "");
    
    // Data storage API
    void GetData(const std::vector<std::string>& keys);
    void SetData(const std::string& key, const nlohmann::json& value);
    
    // Status updates
    void SetGoalComplete();
    void SetGameComplete();
    
    // Callbacks
    void SetItemReceivedCallback(ItemReceivedCallback callback);
    void SetLocationCheckedCallback(LocationCheckedCallback callback);
    void SetConnectionStatusCallback(ConnectionStatusCallback callback);
    void SetMessageReceivedCallback(MessageReceivedCallback callback);
    void SetDeathLinkCallback(DeathLinkCallback callback);
    
    // Message queue
    bool HasPendingMessages() const;
    Message GetNextMessage();
    std::vector<Message> GetAllMessages();
    
    // Player info
    const std::vector<NetworkPlayer>& GetConnectedPlayers() const;
    const NetworkPlayer* GetPlayer(int slot) const;
    const NetworkPlayer* GetLocalPlayer() const;
    
    // Configuration
    void UpdateConfig(const Config& config);
    const Config& GetConfig() const;
    
    // Frame update (call from game loop)
    void Update();
    
private:
    Manager();
    ~Manager();
    
    // Internal state
    std::unique_ptr<APNetworkClient> network_client_;
    std::unique_ptr<APStateManager> state_manager_;
    std::unique_ptr<APProtocolHandler> protocol_handler_;
    
    // Thread safety
    mutable std::mutex manager_mutex_;
    std::atomic<bool> initialized_{false};
    
    // Configuration
    Config config_;
    
    // Cached data
    std::vector<NetworkPlayer> connected_players_;
    int local_slot_ = -1;
    
    // Callbacks
    ItemReceivedCallback item_received_callback_;
    LocationCheckedCallback location_checked_callback_;
    ConnectionStatusCallback connection_status_callback_;
    MessageReceivedCallback message_received_callback_;
    DeathLinkCallback deathlink_callback_;
};

} // namespace Archipelago

// Convenience macro for accessing the manager
#define AP_Manager Archipelago::Manager::GetInstance()
