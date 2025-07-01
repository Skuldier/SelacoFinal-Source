#include "ap_manager.h"
#include "ap_network.h"
#include "ap_state.h"
#include "ap_protocol.h"

// Include nlohmann/json
#include "../dependencies/archipelago_fixes.h"

// Include Selaco headers
#include "common/console/c_console.h"
#include "d_main.h"
#include "g_game.h"

namespace Archipelago {

// Singleton instance
static Manager* s_instance = nullptr;
static std::mutex s_instance_mutex;

Manager& Manager::GetInstance() {
    std::lock_guard<std::mutex> lock(s_instance_mutex);
    if (!s_instance) {
        s_instance = new Manager();
    }
    return *s_instance;
}

Manager::Manager() 
    : config_{}
    , local_slot_(-1) {
    Printf("Archipelago: Manager created\n");
}

Manager::~Manager() {
    Shutdown();
}

bool Manager::Initialize(const Config& config) {
    std::lock_guard<std::mutex> lock(manager_mutex_);
    
    if (initialized_) {
        Printf("Archipelago: Already initialized\n");
        return false;
    }
    
    Printf("Archipelago: Initializing...\n");
    
    config_ = config;
    
    try {
        // Create components
        network_client_ = std::make_unique<APNetworkClient>();
        state_manager_ = std::make_unique<APStateManager>();
        protocol_handler_ = std::make_unique<APProtocolHandler>();
        
        // Set up network callbacks
        network_client_->on_connection_status_changed = [this](ConnectionStatus status) {
            std::lock_guard<std::mutex> lock(manager_mutex_);
            if (connection_status_callback_) {
                connection_status_callback_(status);
            }
            
            // Log status change
            const char* status_str = "Unknown";
            switch (status) {
                case ConnectionStatus::Disconnected: status_str = "Disconnected"; break;
                case ConnectionStatus::Connecting: status_str = "Connecting"; break;
                case ConnectionStatus::Connected: status_str = "Connected"; break;
                case ConnectionStatus::Authenticating: status_str = "Authenticating"; break;
                case ConnectionStatus::Authenticated: status_str = "Authenticated"; break;
                case ConnectionStatus::ConnectionRefused: status_str = "Connection Refused"; break;
                case ConnectionStatus::Error: status_str = "Error"; break;
            }
            Printf("Archipelago: Status changed to %s\n", status_str);
        };
        
        network_client_->on_items_received = [this](const std::vector<NetworkItem>& items) {
            std::lock_guard<std::mutex> lock(manager_mutex_);
            for (const auto& item : items) {
                if (item_received_callback_) {
                    item_received_callback_(item);
                }
                
                // Add to state manager
                state_manager_->AddReceivedItem(item);
                
                // Log item
                if (config_.verbose_logging) {
                    Printf("Archipelago: Received item %lld from player %d\n", 
                           item.item_id, item.player_id);
                }
            }
        };
        
        network_client_->on_locations_checked = [this](const std::vector<int64_t>& locations) {
            std::lock_guard<std::mutex> lock(manager_mutex_);
            for (int64_t loc : locations) {
                if (location_checked_callback_) {
                    location_checked_callback_(loc);
                }
                state_manager_->MarkLocationChecked(loc);
            }
        };
        
        network_client_->on_print_json = [this](const nlohmann::json& data) {
            protocol_handler_->HandlePrintJson(data, [this](const Message& msg) {
                std::lock_guard<std::mutex> lock(manager_mutex_);
                if (message_received_callback_) {
                    message_received_callback_(msg);
                }
                state_manager_->AddMessage(msg);
            });
        };
        
        network_client_->on_players_updated = [this](const std::vector<NetworkPlayer>& players) {
            std::lock_guard<std::mutex> lock(manager_mutex_);
            connected_players_ = players;
        };
        
        initialized_ = true;
        Printf("Archipelago: Initialization complete\n");
        return true;
        
    } catch (const std::exception& e) {
        Printf("Archipelago: Initialization failed: %s\n", e.what());
        return false;
    }
}

void Manager::Shutdown() {
    std::lock_guard<std::mutex> lock(manager_mutex_);
    
    if (!initialized_) {
        return;
    }
    
    Printf("Archipelago: Shutting down...\n");
    
    // Disconnect if connected
    if (network_client_) {
        network_client_->Disconnect();
    }
    
    // Clean up components
    protocol_handler_.reset();
    state_manager_.reset();
    network_client_.reset();
    
    initialized_ = false;
    Printf("Archipelago: Shutdown complete\n");
}

bool Manager::Connect(const std::string& server, const std::string& slot_name, 
                      const std::string& password) {
    std::lock_guard<std::mutex> lock(manager_mutex_);
    
    if (!initialized_) {
        Printf("Archipelago: Not initialized\n");
        return false;
    }
    
    if (!network_client_) {
        Printf("Archipelago: Network client not available\n");
        return false;
    }
    
    // Build URI if needed
    std::string uri = server;
    if (uri.find("://") == std::string::npos) {
        // Try wss first (secure), fallback to ws if it fails
        uri = "wss://" + uri;
    }
    
    Printf("Archipelago: Connecting to %s as '%s'...\n", server.c_str(), slot_name.c_str());
    
    // Connect
    if (!network_client_->Connect(uri, "Selaco")) {
        Printf("Archipelago: Failed to initiate connection\n");
        return false;
    }
    
    // Authenticate will be called after socket connects
    network_client_->Authenticate(slot_name, password);
    
    return true;
}

void Manager::Disconnect() {
    std::lock_guard<std::mutex> lock(manager_mutex_);
    
    if (network_client_) {
        network_client_->Disconnect();
    }
}

bool Manager::IsConnected() const {
    std::lock_guard<std::mutex> lock(manager_mutex_);
    return network_client_ && network_client_->IsConnected();
}

ConnectionStatus Manager::GetConnectionStatus() const {
    std::lock_guard<std::mutex> lock(manager_mutex_);
    return network_client_ ? network_client_->GetConnectionStatus() : ConnectionStatus::Disconnected;
}

void Manager::CheckLocation(int64_t location_id) {
    std::lock_guard<std::mutex> lock(manager_mutex_);
    
    if (!network_client_ || !network_client_->IsConnected()) {
        return;
    }
    
    // Check if already checked
    if (state_manager_ && state_manager_->IsLocationChecked(location_id)) {
        if (config_.verbose_logging) {
            Printf("Archipelago: Location %lld already checked\n", location_id);
        }
        return;
    }
    
    network_client_->SendLocationChecks({location_id});
}

void Manager::CheckLocations(const std::vector<int64_t>& location_ids) {
    std::lock_guard<std::mutex> lock(manager_mutex_);
    
    if (!network_client_ || !network_client_->IsConnected() || !state_manager_) {
        return;
    }
    
    // Filter out already checked locations
    std::vector<int64_t> to_check;
    for (int64_t id : location_ids) {
        if (!state_manager_->IsLocationChecked(id)) {
            to_check.push_back(id);
        }
    }
    
    if (!to_check.empty()) {
        network_client_->SendLocationChecks(to_check);
    }
}

void Manager::SendChat(const std::string& message) {
    std::lock_guard<std::mutex> lock(manager_mutex_);
    
    if (!network_client_ || !network_client_->IsConnected()) {
        Printf("Archipelago: Not connected\n");
        return;
    }
    
    network_client_->SendChat(message);
}

void Manager::Update() {
    if (!initialized_ || !network_client_) {
        return;
    }
    
    // Process network events
    network_client_->ProcessNetworkEvents();
    
    // Update state manager
    if (state_manager_) {
        state_manager_->Update();
    }
}

// Callback setters
void Manager::SetItemReceivedCallback(ItemReceivedCallback callback) {
    std::lock_guard<std::mutex> lock(manager_mutex_);
    item_received_callback_ = callback;
}

void Manager::SetLocationCheckedCallback(LocationCheckedCallback callback) {
    std::lock_guard<std::mutex> lock(manager_mutex_);
    location_checked_callback_ = callback;
}

void Manager::SetConnectionStatusCallback(ConnectionStatusCallback callback) {
    std::lock_guard<std::mutex> lock(manager_mutex_);
    connection_status_callback_ = callback;
}

void Manager::SetMessageReceivedCallback(MessageReceivedCallback callback) {
    std::lock_guard<std::mutex> lock(manager_mutex_);
    message_received_callback_ = callback;
}

bool Manager::HasPendingMessages() const {
    std::lock_guard<std::mutex> lock(manager_mutex_);
    return state_manager_ && state_manager_->HasPendingMessages();
}

Message Manager::GetNextMessage() {
    std::lock_guard<std::mutex> lock(manager_mutex_);
    if (state_manager_) {
        return state_manager_->GetNextMessage();
    }
    return Message{};
}

const std::vector<NetworkPlayer>& Manager::GetConnectedPlayers() const {
    return connected_players_;
}

} // namespace Archipelago