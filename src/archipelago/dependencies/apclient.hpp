#pragma once
#include "../archipelago_fixes.h"
#include <string>
#include <vector>
#include <functional>
#include <tuple>

class APClient {
public:
    struct NetworkItem {
        int64_t item;
        int64_t location;
        int player;
        int flags;
    };
    
    APClient(const std::string& uuid, const std::string& game, const std::string& uri) {}
    ~APClient() {}
    
    void poll() {}
    void ConnectSlot(const std::string& name, const std::string& password, int items_handling, 
                     const std::vector<std::string>& tags, const std::tuple<int,int,int>& version) {}
    void LocationChecks(const std::vector<int64_t>& locations) {}
    void Say(const std::string& text) {}
    
    // Callbacks
    void set_socket_connected_handler(std::function<void()> f) {}
    void set_socket_disconnected_handler(std::function<void()> f) {}
    void set_slot_connected_handler(std::function<void(const json&)> f) {}
    void set_slot_refused_handler(std::function<void(const std::vector<std::string>&)> f) {}
    void set_items_received_handler(std::function<void(const std::vector<NetworkItem>&)> f) {}
    void set_print_json_handler(std::function<void(const json&)> f) {}
};
