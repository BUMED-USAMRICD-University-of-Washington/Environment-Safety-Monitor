// univac_aegis_bridge.cpp
#include "univac_aegis_bridge.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <random>

// If compiling on Linux/Ubuntu servers, ensure libcurl4-openssl-dev is linked
#include <curl/curl.h> 

namespace SafetySystem {

    UnivacAegisBridge::UnivacAegisBridge(const std::string& endpoint_url, const std::string& node_identifier)
        : _endpoint_url(endpoint_url), _node_identifier(node_identifier) {
        curl_global_init(CURL_GLOBAL_ALL);
    }

    void UnivacAegisBridge::DispatchTelemetryAsync(uint8_t alarm_bitmask, double current_o2, double current_temp) {
        
        int severity = MapToAegisSeverity(alarm_bitmask);
        std::string event_type = MapToAegisEventType(alarm_bitmask);
        
        // Format forensic data string
        std::stringstream raw_data;
        raw_data << "O2:" << std::fixed << std::setprecision(2) << current_o2 << "%|"
                 << "TEMP:" << current_temp << "C|"
                 << "HEX:0x" << std::hex << (int)alarm_bitmask;

        // Construct Univac Aegis compliant JSON
        std::string json_payload = "{"
            "\"EventId\": \"" + GenerateUUID() + "\", "
            "\"Timestamp\": \"" + GenerateISOTimestamp() + "\", "
            "\"SourceNode\": \"" + _node_identifier + "\", "
            "\"EventType\": \"" + event_type + "\", "
            "\"SeverityLevel\": " + std::to_string(severity) + ", "
            "\"RawProtocolData\": \"" + raw_data.str() + "\""
            "}";

        // Spawn a detached thread so Core 1 is never blocked waiting for a network ACK
        std::thread worker(&UnivacAegisBridge::ExecuteHttpPost, this, json_payload);
        worker.detach();
    }

    int UnivacAegisBridge::MapToAegisSeverity(uint8_t alarm_bitmask) {
        if (alarm_bitmask & CRIT_O2_HYPOXIA) return 10;
        if (alarm_bitmask & CRIT_TEMP_THERMAL_RUN) return 9;
        if (alarm_bitmask & WARN_O2_DEVIATION) return 8;
        if (alarm_bitmask & FAULT_O2_WIRE_BREAK) return 5;
        if (alarm_bitmask & FAULT_TEMP_SPI_DISCON) return 4;
        if (alarm_bitmask & SYSTEM_WATCHDOG_JITTER) return 7;
        return 0; // STATE_NORMAL
    }

    std::string UnivacAegisBridge::MapToAegisEventType(uint8_t alarm_bitmask) {
        if (alarm_bitmask & CRIT_O2_HYPOXIA) return "HYPOXIA_EVACUATION";
        if (alarm_bitmask & CRIT_TEMP_THERMAL_RUN) return "THERMAL_RUNAWAY";
        if (alarm_bitmask & WARN_O2_DEVIATION) return "PRE_ALARM_O2_DROP";
        if (alarm_bitmask & FAULT_O2_WIRE_BREAK) return "TROUBLE_LOOP_OPEN";
        if (alarm_bitmask & FAULT_TEMP_SPI_DISCON) return "TROUBLE_SPI_DISCONNECT";
        return "NORMAL_TELEMETRY";
    }

    void UnivacAegisBridge::ExecuteHttpPost(const std::string& json_payload) {
        CURL* curl = curl_easy_init();
        if(curl) {
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            
            // The critical authentication header for the Univac Aegis Core
            headers = curl_slist_append(headers, "X-Aegis-Client: FireWatch-Node-Hutch-Env");

            curl_easy_setopt(curl, CURLOPT_URL, _endpoint_url.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());
            
            // Set aggressive timeouts (5 seconds) to kill the socket if the central server drops
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

            CURLcode res = curl_easy_perform(curl);
            if(res != CURLE_OK) {
                std::cerr << "[AEGIS BRIDGE FAULT] Network dispatch failed: " << curl_easy_strerror(res) << "\n";
            }

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
    }

    std::string UnivacAegisBridge::GenerateISOTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    }

    std::string UnivacAegisBridge::GenerateUUID() {
        // Generates a simple pseudo-UUID for event tracking
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static std::uniform_int_distribution<> dis2(8, 11);

        const char* v = "0123456789abcdef";
        std::string res(36, '-');
        for (int i = 0; i < 36; i++) {
            if (i == 8 || i == 13 || i == 18 || i == 23) continue;
            else if (i == 14) res[i] = '4';
            else if (i == 19) res[i] = v[dis2(gen)];
            else res[i] = v[dis(gen)];
        }
        return res;
    }

} // namespace SafetySystem
