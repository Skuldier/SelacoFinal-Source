#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <chrono>

namespace Archipelago {

// Connection status enum
enum class ConnectionStatus {
    Disconnected,
    Connecting,
    Connected,
    Authenticating,
    Authenticated,
    ConnectionRefused,
    Error
};

// Message type enum
enum class MessageType {
    Chat,
    Hint,
    ItemSend,
    ItemReceived,
    LocationChecked,
    GoalComplete,
    ServerInfo,
    Error
};

// Network item structure
struct NetworkItem {
    int64_t item_id;
    int64_t location_id;
    int player_id;
    std::string player_name;
    int flags;
    
    bool is_progression() const { return flags & 0x01; }
    bool is_important() const { return flags & 0x02; }
    bool is_trap() const { return flags & 0x04; }
};

// Network player structure
struct NetworkPlayer {
    int team;
    int slot;
    std::string name;
    std::string alias;
    std::string game;
};

// Message structure for UI display
struct Message {
    MessageType type;
    std::string text;
    std::chrono::system_clock::time_point timestamp;
    int priority;
};

// Configuration structure
struct Config {
    // Connection settings
    bool auto_reconnect = true;
    int reconnect_delay_ms = 5000;
    int max_reconnect_attempts = 10;
    
    // Feature flags
    bool enable_deathlink = false;
    bool enable_chat = true;
    bool enable_hints = true;
    bool enable_item_tracking = true;
    
    // UI settings
    bool show_connection_status = true;
    bool show_item_popups = true;
    float item_popup_duration = 3.0f;
    
    // Debug settings
    bool verbose_logging = false;
    bool log_network_traffic = false;
};

// Callback types
using ItemReceivedCallback = std::function<void(const NetworkItem&)>;
using LocationCheckedCallback = std::function<void(int64_t location_id)>;
using ConnectionStatusCallback = std::function<void(ConnectionStatus)>;
using MessageReceivedCallback = std::function<void(const Message&)>;
using DeathLinkCallback = std::function<void(const std::string& source)>;

} // namespace Archipelago
