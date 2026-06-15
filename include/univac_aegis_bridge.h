// univac_aegis_bridge.h
#pragma once

#include <string>
#include <future>
#include <cstdint>
#include "telemetry_types.h" // Pulls in your SafetySystem::AlarmBits

namespace SafetySystem {

    class UnivacAegisBridge {
    public:
        // Initialize with the URL of the Python FastAPI server we built
        UnivacAegisBridge(const std::string& endpoint_url, const std::string& node_identifier);
        ~UnivacAegisBridge() = default;

        // Dispatches the network call to a background thread instantly
        void DispatchTelemetryAsync(uint8_t alarm_bitmask, double current_o2, double current_temp);

    private:
        std::string _endpoint_url;
        std::string _node_identifier;

        // Translation Matrix
        int MapToAegisSeverity(uint8_t alarm_bitmask);
        std::string MapToAegisEventType(uint8_t alarm_bitmask);
        std::string GenerateISOTimestamp();
        std::string GenerateUUID();

        // The isolated HTTP worker payload
        void ExecuteHttpPost(const std::string& json_payload);
    };

} // namespace SafetySystem
