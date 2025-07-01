#pragma once

// WebSocket wrapper stub for Archipelago
// This file provides a minimal interface for WebSocket functionality

#include <string>
#include <functional>
#include <memory>

namespace wswrap {

class WebSocketClient {
public:
    WebSocketClient() = default;
    virtual ~WebSocketClient() = default;
    
    // Connection callbacks
    std::function<void()> on_open;
    std::function<void(const std::string&)> on_message;
    std::function<void(int, const std::string&)> on_close;
    std::function<void(const std::string&)> on_error;
    
    // Connection methods
    virtual bool connect(const std::string& url) = 0;
    virtual void disconnect() = 0;
    virtual bool send(const std::string& message) = 0;
    virtual bool is_connected() const = 0;
};

// Factory function to create platform-specific WebSocket client
std::unique_ptr<WebSocketClient> create_websocket_client();

} // namespace wswrap
