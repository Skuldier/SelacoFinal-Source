#pragma once

#include "ap_types.h"
#include <unordered_set>
#include <queue>
#include <mutex>

namespace Archipelago {

class APStateManager {
public:
    APStateManager();
    ~APStateManager();
    
    // Location tracking
    void MarkLocationChecked(int64_t location_id);
    bool IsLocationChecked(int64_t location_id) const;
    std::vector<int64_t> GetCheckedLocations() const;
    void ClearCheckedLocations();
    
    // Item tracking
    void AddReceivedItem(const NetworkItem& item);
    std::vector<NetworkItem> GetReceivedItems() const;
    void ClearReceivedItems();
    
    // Message queue
    void AddMessage(const Message& message);
    bool HasPendingMessages() const;
    Message GetNextMessage();
    void ClearMessages();
    
    // Save/Load state
    bool SaveState(const std::string& filename) const;
    bool LoadState(const std::string& filename);
    
    // Update (for any time-based operations)
    void Update();
    
private:
    mutable std::mutex state_mutex_;
    
    // Tracked locations
    std::unordered_set<int64_t> checked_locations_;
    
    // Received items
    std::vector<NetworkItem> received_items_;
    
    // Message queue
    std::queue<Message> message_queue_;
    
    // Stats
    int total_locations_checked_ = 0;
    int total_items_received_ = 0;
};

} // namespace Archipelago