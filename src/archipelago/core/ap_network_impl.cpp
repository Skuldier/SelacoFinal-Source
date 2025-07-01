#include "ap_network.h"

namespace Archipelago {
// Stub implementations for missing methods
void APNetworkClient::SendLocationScouts(const std::vector<int64_t>& locations) {}
void APNetworkClient::SendBounce(const json& data) {}
void APNetworkClient::SendStatusUpdate(int status) {}
void APNetworkClient::GetData(const std::vector<std::string>& keys) {}
void APNetworkClient::SetData(const std::string& key, const json& value) {}
void APNetworkClient::StopNetworkThread() {}
}
