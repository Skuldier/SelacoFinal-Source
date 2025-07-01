#pragma once

#include "../core/ap_types.h"

namespace Archipelago {

class APOverlay {
public:
    APOverlay();
    ~APOverlay();
    
    // Show/hide overlay
    void Show();
    void Hide();
    bool IsVisible() const;
    
    // Update overlay
    void Update(float deltaTime);
    void Draw();
    
    // Connection status display
    void SetConnectionStatus(ConnectionStatus status);
    
    // Item popup
    void ShowItemReceived(const NetworkItem& item);
    
    // Chat/message display
    void AddMessage(const Message& message);
    
    // Settings
    void SetPosition(int x, int y);
    void SetScale(float scale);
    
private:
    bool visible_;
    ConnectionStatus connection_status_;
    
    // Position and scale
    int pos_x_;
    int pos_y_;
    float scale_;
    
    // Item popup
    struct ItemPopup {
        NetworkItem item;
        float timer;
        bool active;
    };
    ItemPopup current_popup_;
    
    // Message history
    static constexpr int MAX_MESSAGES = 10;
    std::vector<Message> message_history_;
    
    // Animation timers
    float connection_blink_timer_;
};

} // namespace Archipelago