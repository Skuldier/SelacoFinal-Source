#include "selaco_integration.h"
#include "../core/ap_manager.h"

// Include Selaco game headers - use the same pattern as other source files
#include "common/console/c_console.h"
#include "common/console/c_dispatch.h"
#include "d_main.h"
#include "g_game.h"
#include "g_level.h"
#include "doomstat.h"
#include "d_player.h"
#include "p_local.h"

using namespace Archipelago;

// Console commands for Archipelago
CCMD(ap_connect) {
    if (argv.argc() < 3) {
        Printf("Usage: ap_connect <server:port> <slot_name> [password]\n");
        Printf("Example: ap_connect archipelago.gg:12345 MySlot\n");
        return;
    }
    
    std::string server = argv[1];
    std::string slot = argv[2];
    std::string password = argv.argc() > 3 ? argv[3] : "";
    
    // Initialize manager if needed
    auto& manager = Manager::GetInstance();
    if (!manager.IsConnected()) {
        manager.Initialize();
    }
    
    // Connect
    if (manager.Connect(server, slot, password)) {
        Printf("Archipelago: Connection initiated\n");
    } else {
        Printf("Archipelago: Failed to connect\n");
    }
}

CCMD(ap_disconnect) {
    auto& manager = Manager::GetInstance();
    manager.Disconnect();
    Printf("Archipelago: Disconnected\n");
}

CCMD(ap_status) {
    auto& manager = Manager::GetInstance();
    
    const char* status_str = "Unknown";
    switch (manager.GetConnectionStatus()) {
        case ConnectionStatus::Disconnected: status_str = "Disconnected"; break;
        case ConnectionStatus::Connecting: status_str = "Connecting"; break;
        case ConnectionStatus::Connected: status_str = "Connected"; break;
        case ConnectionStatus::Authenticating: status_str = "Authenticating"; break;
        case ConnectionStatus::Authenticated: status_str = "Authenticated"; break;
        case ConnectionStatus::ConnectionRefused: status_str = "Connection Refused"; break;
        case ConnectionStatus::Error: status_str = "Error"; break;
    }
    
    Printf("Archipelago Status: %s\n", status_str);
    
    if (manager.IsConnected()) {
        auto& players = manager.GetConnectedPlayers();
        Printf("Connected Players: %d\n", (int)players.size());
        for (const auto& player : players) {
            Printf("  [%d] %s (%s)\n", player.slot, player.name.c_str(), player.game.c_str());
        }
    }
}

CCMD(ap_say) {
    if (argv.argc() < 2) {
        Printf("Usage: ap_say <message>\n");
        return;
    }
    
    std::string message;
    for (int i = 1; i < argv.argc(); i++) {
        if (i > 1) message += " ";
        message += argv[i];
    }
    
    auto& manager = Manager::GetInstance();
    if (manager.IsConnected()) {
        manager.SendChat(message);
    } else {
        Printf("Archipelago: Not connected\n");
    }
}

// Initialize Archipelago on game start
static void InitializeArchipelago() {
    Printf("Archipelago: Initializing Selaco integration...\n");
    
    auto& manager = Manager::GetInstance();
    
    // Set up callbacks
    manager.SetItemReceivedCallback([](const NetworkItem& item) {
        Printf("Archipelago: Received item %lld from player %s\n", 
               item.item_id, item.player_name.c_str());
        
        // TODO: Grant item to player based on item_id
        // Example: GiveInventory(item_id_to_class_name(item.item_id));
    });
    
    manager.SetLocationCheckedCallback([](int64_t location_id) {
        Printf("Archipelago: Location %lld checked\n", location_id);
    });
    
    manager.SetMessageReceivedCallback([](const Message& msg) {
        // Display message in game
        if (msg.type == MessageType::Chat || msg.type == MessageType::ItemReceived) {
            Printf("%s\n", msg.text.c_str());
        }
    });
    
    Printf("Archipelago: Selaco integration ready\n");
}

// Register initialization
class ArchipelagoInitializer {
public:
    ArchipelagoInitializer() {
        // This will run when the game starts
        InitializeArchipelago();
    }
} g_archipelago_init;

// Update function to be called from game loop
void UpdateArchipelago() {
    auto& manager = Manager::GetInstance();
    manager.Update();
    
    // Process any pending messages
    while (manager.HasPendingMessages()) {
        auto msg = manager.GetNextMessage();
        // Handle message display in UI
    }
}