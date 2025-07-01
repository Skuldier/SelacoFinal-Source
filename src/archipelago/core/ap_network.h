#pragma once

#include "ap_types.h"
#include <memory>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "../dependencies/archipelago_fixes.h"`n`n`#include "../dependencies/archipelago_fixes.h"

#include "../dependencies/archipelago_fixes.h"

// Forward declaration of APClient from apclientpp
class APClient;
namespace nlohmann { class json; }

namespace Archipelago {

class APNetworkClient {
public:
    APNetworkClient();
    ~APNetworkClient();
    
    // Connection management
    bool Connect(const std::string& uri, const std::string& game_name);
    bool Authenticate(const std::string& slot_name, const std::string& password);
    void Disconnect();
    
    // Status
    ConnectionStatus GetConnectionStatus() const;
    bool IsConnected() const;
    
    // Send operations (thread-safe)
    void SendLocationChecks(const std::vector<int64_t>& locations);
    void SendLocationScouts(const std::vector<int64_t>& locations);
    void SendChat(const std::string& message);
    void SendBounce(const nlohmann::json& data);
    void SendStatusUpdate(int status);
    
    // Data storage API
    void GetData(const std::vector<std::string>& keys);
    void SetData(const std::string& key, const nlohmann::json& value);
    
    // Network processing
    void ProcessNetworkEvents();
    
    // Callbacks (called from network thread)
    std::function<void(ConnectionStatus)> on_connection_status_changed;
    std::function<void(const std::vector<NetworkItem>&)> on_items_received;
    std::function<void(const std::vector<int64_t>&)> on_locations_checked;
    std::function<void(const nlohmann::json&)> on_print_json;
    std::function<void(const nlohmann::json&)> on_bounce_received;
    std::function<void(const std::vector<NetworkPlayer>&)> on_players_updated;
    std::function<void(const nlohmann::json&)> on_data_received;
    
private:
    // Network thread management
    void NetworkThreadMain();
    void StopNetworkThread();
    
    // Connection state
    std::atomic<ConnectionStatus> connection_status_{ConnectionStatus::Disconnected};
    std::atomic<bool> should_stop_{false};
    
    // APClient instance
    std::unique_ptr<APClient> ap_client_;
    std::unique_ptr<std::thread> network_thread_;
    
    // Thread synchronization
    std::mutex network_mutex_;
    std::condition_variable network_cv_;
    
    // Command queue for thread-safe operations
    struct NetworkCommand {
        enum Type {
            Connect,
            Authenticate,
            Disconnect,
            CheckLocations,
            ScoutLocations,
            SendChat,
            SendBounce,
            GetData,
            SetData,
            StatusUpdate
        };
        
        Type type;
        nlohmann::json data;
    };
    
    std::queue<NetworkCommand> command_queue_;
    std::mutex command_mutex_;
    
    // Enqueue command for network thread
    void EnqueueCommand(const NetworkCommand& cmd);
    
    // Process commands in network thread
    void ProcessCommands();
    
    // Setup APClient callbacks
    void SetupCallbacks();
    
    // Connection details
    std::string server_uri_;
    std::string game_name_;
    std::string slot_name_;
    std::string password_;
    
    // Retry logic
    int reconnect_attempts_ = 0;
    std::chrono::steady_clock::time_point last_connect_attempt_;
};

} // namespace Archipelago
