#pragma once

#include <cstdint>
#include <string>

// Selaco-specific location and item IDs
namespace SelacoAP {
    
    // Base IDs for Selaco
    constexpr int64_t LOCATION_BASE = 87420000;  // Unique base for Selaco locations
    constexpr int64_t ITEM_BASE = 87421000;      // Unique base for Selaco items
    
    // Location IDs
    namespace Locations {
        // Weapons
        constexpr int64_t PISTOL_PICKUP = LOCATION_BASE + 1;
        constexpr int64_t SHOTGUN_PICKUP = LOCATION_BASE + 2;
        constexpr int64_t ASSAULT_RIFLE_PICKUP = LOCATION_BASE + 3;
        constexpr int64_t GRENADE_LAUNCHER_PICKUP = LOCATION_BASE + 4;
        
        // Keycards
        constexpr int64_t RED_KEYCARD = LOCATION_BASE + 10;
        constexpr int64_t BLUE_KEYCARD = LOCATION_BASE + 11;
        constexpr int64_t YELLOW_KEYCARD = LOCATION_BASE + 12;
        
        // Level completion
        constexpr int64_t LEVEL_1_COMPLETE = LOCATION_BASE + 100;
        constexpr int64_t LEVEL_2_COMPLETE = LOCATION_BASE + 101;
        constexpr int64_t LEVEL_3_COMPLETE = LOCATION_BASE + 102;
        
        // Secrets
        constexpr int64_t SECRET_1 = LOCATION_BASE + 200;
        constexpr int64_t SECRET_2 = LOCATION_BASE + 201;
        constexpr int64_t SECRET_3 = LOCATION_BASE + 202;
    }
    
    // Item IDs
    namespace Items {
        // Weapons
        constexpr int64_t PISTOL = ITEM_BASE + 1;
        constexpr int64_t SHOTGUN = ITEM_BASE + 2;
        constexpr int64_t ASSAULT_RIFLE = ITEM_BASE + 3;
        constexpr int64_t GRENADE_LAUNCHER = ITEM_BASE + 4;
        
        // Ammo
        constexpr int64_t PISTOL_AMMO = ITEM_BASE + 10;
        constexpr int64_t SHOTGUN_AMMO = ITEM_BASE + 11;
        constexpr int64_t RIFLE_AMMO = ITEM_BASE + 12;
        constexpr int64_t GRENADE_AMMO = ITEM_BASE + 13;
        
        // Keycards
        constexpr int64_t RED_KEYCARD = ITEM_BASE + 20;
        constexpr int64_t BLUE_KEYCARD = ITEM_BASE + 21;
        constexpr int64_t YELLOW_KEYCARD = ITEM_BASE + 22;
        
        // Health/Armor
        constexpr int64_t HEALTH_SMALL = ITEM_BASE + 30;
        constexpr int64_t HEALTH_LARGE = ITEM_BASE + 31;
        constexpr int64_t ARMOR_SMALL = ITEM_BASE + 32;
        constexpr int64_t ARMOR_LARGE = ITEM_BASE + 33;
        
        // Progression items
        constexpr int64_t DOUBLE_JUMP = ITEM_BASE + 40;
        constexpr int64_t DASH_ABILITY = ITEM_BASE + 41;
        constexpr int64_t SECURITY_CLEARANCE = ITEM_BASE + 42;
    }
}

// Integration functions
void UpdateArchipelago();

// Helper functions for game integration
std::string GetItemName(int64_t item_id);
std::string GetLocationName(int64_t location_id);
void GrantItemToPlayer(int64_t item_id);
void CheckLocationInGame(int64_t location_id);

// Macros for easy integration
#ifdef ARCHIPELAGO_ENABLED
    #define AP_CHECK_LOCATION(loc_id) CheckLocationInGame(loc_id)
    #define AP_UPDATE() UpdateArchipelago()
#else
    #define AP_CHECK_LOCATION(loc_id) ((void)0)
    #define AP_UPDATE() ((void)0)
#endif