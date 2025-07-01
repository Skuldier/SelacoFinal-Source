#pragma once
#include <string>
#include <chrono>

inline std::string ap_get_uuid(const std::string& game_name) {
    // Simple UUID generation for now
    return game_name + "_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
}
