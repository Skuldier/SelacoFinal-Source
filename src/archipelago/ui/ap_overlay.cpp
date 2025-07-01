#include "ap_overlay.h"
#include "common/2d/v_draw.h"
#include "common/2d/v_text.h"
#include "common/fonts/v_font.h"
#include "g_game.h"

namespace Archipelago {

APOverlay::APOverlay()
    : visible_(true)
    , connection_status_(ConnectionStatus::Disconnected)
    , pos_x_(10)
    , pos_y_(10)
    , scale_(1.0f)
    , connection_blink_timer_(0.0f)
{
    current_popup_.active = false;
    current_popup_.timer = 0.0f;
}

APOverlay::~APOverlay()
{
}

void APOverlay::Show()
{
    visible_ = true;
}

void APOverlay::Hide()
{
    visible_ = false;
}

bool APOverlay::IsVisible() const
{
    return visible_;
}

void APOverlay::Update(float deltaTime)
{
    // Update connection blink timer
    connection_blink_timer_ += deltaTime;
    if (connection_blink_timer_ > 2.0f)
        connection_blink_timer_ = 0.0f;
    
    // Update item popup
    if (current_popup_.active)
    {
        current_popup_.timer -= deltaTime;
        if (current_popup_.timer <= 0.0f)
        {
            current_popup_.active = false;
        }
    }
}

void APOverlay::Draw()
{
    if (!visible_)
        return;
        
    // Draw connection status
    const char* status_text = "Archipelago: ";
    int color = CR_WHITE;
    
    switch (connection_status_)
    {
    case ConnectionStatus::Disconnected:
        status_text = "Archipelago: Disconnected";
        color = CR_RED;
        break;
    case ConnectionStatus::Connecting:
        status_text = "Archipelago: Connecting...";
        color = CR_YELLOW;
        break;
    case ConnectionStatus::Connected:
        status_text = "Archipelago: Connected";
        color = CR_YELLOW;
        break;
    case ConnectionStatus::Authenticating:
        status_text = "Archipelago: Authenticating...";
        color = CR_YELLOW;
        break;
    case ConnectionStatus::Authenticated:
        status_text = "Archipelago: Online";
        color = CR_GREEN;
        break;
    case ConnectionStatus::ConnectionRefused:
        status_text = "Archipelago: Connection Refused";
        color = CR_RED;
        break;
    case ConnectionStatus::Error:
        status_text = "Archipelago: Error";
        color = CR_RED;
        break;
    }
    
    // Blink when connecting
    if ((connection_status_ == ConnectionStatus::Connecting || 
         connection_status_ == ConnectionStatus::Authenticating) &&
        connection_blink_timer_ > 1.0f)
    {
        color = CR_DARKGRAY;
    }
    
    // Draw status text
    DrawText(NewSmallFont, color, pos_x_, pos_y_, status_text);
    
    // Draw item popup if active
    if (current_popup_.active)
    {
        int popup_y = pos_y_ + 20;
        float alpha = 1.0f;
        
        // Fade out
        if (current_popup_.timer < 0.5f)
        {
            alpha = current_popup_.timer / 0.5f;
        }
        
        FString item_text;
        item_text.Format("Received: Item %lld", current_popup_.item.item_id);
        
        DrawText(NewSmallFont, CR_WHITE, pos_x_, popup_y, item_text, 
                 DTA_Alpha, OPAQUE * alpha, TAG_DONE);
    }
    
    // Draw recent messages
    int message_y = pos_y_ + 40;
    for (size_t i = 0; i < message_history_.size() && i < 5; ++i)
    {
        const auto& msg = message_history_[i];
        int msg_color = CR_WHITE;
        
        switch (msg.type)
        {
        case MessageType::Chat:
            msg_color = CR_WHITE;
            break;
        case MessageType::ItemReceived:
            msg_color = CR_GREEN;
            break;
        case MessageType::Hint:
            msg_color = CR_YELLOW;
            break;
        case MessageType::Error:
            msg_color = CR_RED;
            break;
        default:
            msg_color = CR_GRAY;
            break;
        }
        
        DrawText(NewSmallFont, msg_color, pos_x_, message_y + (i * 10), 
                 msg.text.c_str());
    }
}

void APOverlay::SetConnectionStatus(ConnectionStatus status)
{
    connection_status_ = status;
}

void APOverlay::ShowItemReceived(const NetworkItem& item)
{
    current_popup_.item = item;
    current_popup_.timer = 3.0f;  // Show for 3 seconds
    current_popup_.active = true;
}

void APOverlay::AddMessage(const Message& message)
{
    message_history_.insert(message_history_.begin(), message);
    
    // Keep only recent messages
    if (message_history_.size() > MAX_MESSAGES)
    {
        message_history_.resize(MAX_MESSAGES);
    }
}

void APOverlay::SetPosition(int x, int y)
{
    pos_x_ = x;
    pos_y_ = y;
}

void APOverlay::SetScale(float scale)
{
    scale_ = scale;
}

} // namespace Archipelago