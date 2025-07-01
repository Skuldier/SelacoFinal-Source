#include "ap_manager.h"

namespace Archipelago {
// Stub implementations for missing methods
void Manager::ScoutLocation(int64_t location_id) {}
void Manager::ScoutLocations(const std::vector<int64_t>& location_ids) {}
void Manager::SendDeathLink(const std::string& cause) {}
void Manager::GetData(const std::vector<std::string>& keys) {}
void Manager::SetData(const std::string& key, const json& value) {}
void Manager::SetGoalComplete() {}
void Manager::SetGameComplete() {}
void Manager::SetDeathLinkCallback(DeathLinkCallback callback) {}
std::vector<Message> Manager::GetAllMessages() { return {}; }
const NetworkPlayer* Manager::GetPlayer(int slot) const { return nullptr; }
const NetworkPlayer* Manager::GetLocalPlayer() const { return nullptr; }
void Manager::UpdateConfig(const Config& config) { config_ = config; }
const Config& Manager::GetConfig() const { return config_; }
}
