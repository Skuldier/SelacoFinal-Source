#pragma once

#include "ap_types.h"
#include <nlohmann/json.hpp>
#include <functional>

namespace Archipelago {

class APProtocolHandler {
public:
    APProtocolHandler();
    ~APProtocolHandler();
    
    // Parse PrintJSON messages into readable format
    void HandlePrintJson(const nlohmann::json& data, 
                        std::function<void(const Message&)> callback);
    
    // Build command packets
    nlohmann::json BuildConnectPacket(const std::string& game, 
                                      const std::string& name,
                                      const std::string& password,
                                      const std::string& uuid,
                                      int items_handling = 2);
    
    nlohmann::json BuildLocationChecksPacket(const std::vector<int64_t>& locations);
    nlohmann::json BuildLocationScoutsPacket(const std::vector<int64_t>& locations);
    nlohmann::json BuildStatusUpdatePacket(int status);
    nlohmann::json BuildSayPacket(const std::string& message);
    nlohmann::json BuildBouncePacket(const nlohmann::json& data);
    
    // Parse received packets
    bool ParseRoomInfoPacket(const nlohmann::json& data);
    bool ParseConnectedPacket(const nlohmann::json& data);
    bool ParseReceivedItemsPacket(const nlohmann::json& data, 
                                  std::vector<NetworkItem>& items);
    
    // DeathLink support
    nlohmann::json BuildDeathLinkPacket(const std::string& source, 
                                        const std::string& cause = "");
    bool ParseDeathLinkPacket(const nlohmann::json& data, 
                             std::string& source, 
                             std::string& cause);
    
private:
    // Helper to parse text segments
    std::string ParseTextSegments(const nlohmann::json& segments);
    
    // Color parsing for messages
    std::string ParseColoredText(const nlohmann::json& segment);
    
    // Message type detection
    MessageType DetectMessageType(const nlohmann::json& data);
};

} // namespace Archipelago