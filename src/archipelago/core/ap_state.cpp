#include "ap_state.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace Archipelago {

APStateManager::APStateManager()
    : total_locations_checked_(0)
    , total_items_received_(0)
{
}

APStateManager::~APStateManager()
{
}

void APStateManager::MarkLocationChecked(int64_t location_id)
{
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (checked_locations_.insert(location_id).second)
    {
        total_locations_checked_++;
    }
}

bool APStateManager::IsLocationChecked(int64_t location_id) const
{
    std::lock_guard<std::mutex> lock(state_mutex_);
    return checked_locations_.find(location_id) != checked_locations_.end();
}

std::vector<int64_t> APStateManager::GetCheckedLocations() const
{
    std::lock_guard<std::mutex> lock(state_mutex_);
    return std::vector<int64_t>(checked_locations_.begin(), checked_locations_.end());
}

void APStateManager::ClearCheckedLocations()
{
    std::lock_guard<std::mutex> lock(state_mutex_);
    checked_locations_.clear();
    total_locations_checked_ = 0;
}

void APStateManager::AddReceivedItem(const NetworkItem& item)
{
    std::lock_guard<std::mutex> lock(state_mutex_);
    received_items_.push_back(item);
    total_items_received_++;
}

std::vector<NetworkItem> APStateManager::GetReceivedItems() const
{
    std::lock_guard<std::mutex> lock(state_mutex_);
    return received_items_;
}

void APStateManager::ClearReceivedItems()
{
    std::lock_guard<std::mutex> lock(state_mutex_);
    received_items_.clear();
    total_items_received_ = 0;
}

void APStateManager::AddMessage(const Message& message)
{
    std::lock_guard<std::mutex> lock(state_mutex_);
    message_queue_.push(message);
    
    // Limit queue size
    while (message_queue_.size() > 100)
    {
        message_queue_.pop();
    }
}

bool APStateManager::HasPendingMessages() const
{
    std::lock_guard<std::mutex> lock(state_mutex_);
    return !message_queue_.empty();
}

Message APStateManager::GetNextMessage()
{
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (message_queue_.empty())
    {
        return Message{};
    }
    
    Message msg = message_queue_.front();
    message_queue_.pop();
    return msg;
}

void APStateManager::ClearMessages()
{
    std::lock_guard<std::mutex> lock(state_mutex_);
    std::queue<Message> empty;
    std::swap(message_queue_, empty);
}

bool APStateManager::SaveState(const std::string& filename) const
{
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    try
    {
        nlohmann::json j;
        j["checked_locations"] = std::vector<int64_t>(checked_locations_.begin(), checked_locations_.end());
        j["total_locations_checked"] = total_locations_checked_;
        j["total_items_received"] = total_items_received_;
        
        // Save received items
        nlohmann::json items_array = nlohmann::json::array();
        for (const auto& item : received_items_)
        {
            nlohmann::json item_obj;
            item_obj["item_id"] = item.item_id;
            item_obj["location_id"] = item.location_id;
            item_obj["player_id"] = item.player_id;
            item_obj["player_name"] = item.player_name;
            item_obj["flags"] = item.flags;
            items_array.push_back(item_obj);
        }
        j["received_items"] = items_array;
        
        std::ofstream file(filename);
        file << j.dump(4);
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool APStateManager::LoadState(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    try
    {
        std::ifstream file(filename);
        if (!file.is_open())
            return false;
            
        nlohmann::json j;
        file >> j;
        
        // Load checked locations
        checked_locations_.clear();
        if (j.contains("checked_locations"))
        {
            for (const auto& loc : j["checked_locations"])
            {
                checked_locations_.insert(loc.get<int64_t>());
            }
        }
        
        total_locations_checked_ = j.value("total_locations_checked", 0);
        total_items_received_ = j.value("total_items_received", 0);
        
        // Load received items
        received_items_.clear();
        if (j.contains("received_items"))
        {
            for (const auto& item_obj : j["received_items"])
            {
                NetworkItem item;
                item.item_id = item_obj["item_id"].get<int64_t>();
                item.location_id = item_obj["location_id"].get<int64_t>();
                item.player_id = item_obj["player_id"].get<int>();
                item.player_name = item_obj.value("player_name", "");
                item.flags = item_obj.value("flags", 0);
                received_items_.push_back(item);
            }
        }
        
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

void APStateManager::Update()
{
    // Currently no time-based operations needed
}

} // namespace Archipelago