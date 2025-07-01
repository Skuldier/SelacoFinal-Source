#include "ap_protocol.h"
#include <sstream>

namespace Archipelago {

APProtocolHandler::APProtocolHandler()
{
}

APProtocolHandler::~APProtocolHandler()
{
}

void APProtocolHandler::HandlePrintJson(const nlohmann::json& data, 
                                       std::function<void(const Message&)> callback)
{
    if (!callback || !data.contains("data"))
        return;
        
    Message msg;
    msg.timestamp = std::chrono::system_clock::now();
    msg.type = DetectMessageType(data);
    msg.priority = 0;
    
    // Parse the message segments
    if (data["data"].is_array())
    {
        msg.text = ParseTextSegments(data["data"]);
    }
    
    callback(msg);
}

std::string APProtocolHandler::ParseTextSegments(const nlohmann::json& segments)
{
    std::stringstream result;
    
    for (const auto& segment : segments)
    {
        if (segment.is_object())
        {
            if (segment.contains("text"))
            {
                result << segment["text"].get<std::string>();
            }
            else if (segment.contains("player_name"))
            {
                result << segment["player_name"].get<std::string>();
            }
            else if (segment.contains("item_name"))
            {
                result << segment["item_name"].get<std::string>();
            }
            else if (segment.contains("location_name"))
            {
                result << segment["location_name"].get<std::string>();
            }
        }
        else if (segment.is_string())
        {
            result << segment.get<std::string>();
        }
    }
    
    return result.str();
}

MessageType APProtocolHandler::DetectMessageType(const nlohmann::json& data)
{
    if (!data.contains("type"))
        return MessageType::Chat;
        
    std::string type = data["type"].get<std::string>();
    
    if (type == "ItemSend")
        return MessageType::ItemSend;
    else if (type == "ItemCheat")
        return MessageType::ItemReceived;
    else if (type == "Hint")
        return MessageType::Hint;
    else if (type == "Join")
        return MessageType::ServerInfo;
    else if (type == "Part")
        return MessageType::ServerInfo;
    else if (type == "Chat")
        return MessageType::Chat;
    else if (type == "Goal")
        return MessageType::GoalComplete;
        
    return MessageType::Chat;
}

nlohmann::json APProtocolHandler::BuildConnectPacket(const std::string& game, 
                                                    const std::string& name,
                                                    const std::string& password,
                                                    const std::string& uuid,
                                                    int items_handling)
{
    nlohmann::json packet;
    packet["cmd"] = "Connect";
    packet["password"] = password;
    packet["game"] = game;
    packet["name"] = name;
    packet["uuid"] = uuid;
    packet["version"] = nlohmann::json::object({
        {"major", 0},
        {"minor", 5},
        {"build", 0},
        {"class", "Version"}
    });
    packet["items_handling"] = items_handling;
    packet["tags"] = nlohmann::json::array({"AP"});
    
    return packet;
}

nlohmann::json APProtocolHandler::BuildLocationChecksPacket(const std::vector<int64_t>& locations)
{
    nlohmann::json packet;
    packet["cmd"] = "LocationChecks";
    packet["locations"] = locations;
    return packet;
}

nlohmann::json APProtocolHandler::BuildLocationScoutsPacket(const std::vector<int64_t>& locations)
{
    nlohmann::json packet;
    packet["cmd"] = "LocationScouts";
    packet["locations"] = locations;
    packet["create_as_hint"] = 0;
    return packet;
}

nlohmann::json APProtocolHandler::BuildStatusUpdatePacket(int status)
{
    nlohmann::json packet;
    packet["cmd"] = "StatusUpdate";
    packet["status"] = status;
    return packet;
}

nlohmann::json APProtocolHandler::BuildSayPacket(const std::string& message)
{
    nlohmann::json packet;
    packet["cmd"] = "Say";
    packet["text"] = message;
    return packet;
}

nlohmann::json APProtocolHandler::BuildBouncePacket(const nlohmann::json& data)
{
    nlohmann::json packet;
    packet["cmd"] = "Bounce";
    packet["data"] = data;
    return packet;
}

bool APProtocolHandler::ParseRoomInfoPacket(const nlohmann::json& data)
{
    // Validate required fields
    return data.contains("version") && 
           data.contains("seed_name") &&
           data.contains("players");
}

bool APProtocolHandler::ParseConnectedPacket(const nlohmann::json& data)
{
    // Validate required fields
    return data.contains("team") && 
           data.contains("slot") &&
           data.contains("players");
}

bool APProtocolHandler::ParseReceivedItemsPacket(const nlohmann::json& data, 
                                                std::vector<NetworkItem>& items)
{
    if (!data.contains("items"))
        return false;
        
    items.clear();
    
    for (const auto& item_data : data["items"])
    {
        NetworkItem item;
        item.item_id = item_data["item"].get<int64_t>();
        item.location_id = item_data["location"].get<int64_t>();
        item.player_id = item_data["player"].get<int>();
        item.flags = item_data.value("flags", 0);
        
        items.push_back(item);
    }
    
    return true;
}

nlohmann::json APProtocolHandler::BuildDeathLinkPacket(const std::string& source, 
                                                      const std::string& cause)
{
    nlohmann::json packet;
    packet["cmd"] = "Bounce";
    
    nlohmann::json deathlink;
    deathlink["time"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    deathlink["source"] = source;
    if (!cause.empty())
    {
        deathlink["cause"] = cause;
    }
    
    packet["data"] = {
        {"type", "DeathLink"},
        {"data", deathlink}
    };
    
    return packet;
}

bool APProtocolHandler::ParseDeathLinkPacket(const nlohmann::json& data, 
                                            std::string& source, 
                                            std::string& cause)
{
    if (!data.contains("data") || !data["data"].contains("type"))
        return false;
        
    if (data["data"]["type"] != "DeathLink")
        return false;
        
    const auto& deathlink = data["data"]["data"];
    if (!deathlink.contains("source"))
        return false;
        
    source = deathlink["source"].get<std::string>();
    cause = deathlink.value("cause", "");
    
    return true;
}

} // namespace Archipelago