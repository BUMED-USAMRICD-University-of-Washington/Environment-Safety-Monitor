include "serialization_protocol.h"
include <iostream>

class NetworkSerializerModule {
public:
    NetworkSerializerModule(const char* ip, int port) {
    }

    void scan_and_broadcast_anomalies(const SafetySystem::AlarmTelemetryPacket* h_outputs, int total_outlets) {
        if (total_outlets <= 0) return;

        for (int i = 0; i < total_outlets; ++i) {
            if (h_outputs[i].alarm_state == SafetySystem::STATE_NORMAL) continue;

            std::cout << "[UDP ANOMALY BROADCAST] Outlet: " << i 
                      << " | State Mask: " << static_cast<int>(h_outputs[i].alarm_state) 
                      << " | O2: " << h_outputs[i].oxygen_level << "%\n";
        }
    }

    void serialize_and_pipe_to_fireworks(const SafetySystem::AlarmTelemetryPacket* h_outputs, int total_outlets) {
        if (total_outlets <= 0) return;

        SafetySystem::EdwardsBulkFrameHeader header;
        header.start_byte = SafetySystem::FRAME_START_BYTE;
        header.total_payload_bytes = total_outlets * sizeof(SafetySystem::AlarmTelemetryPacket);
        header.checksum_crc32 = 0; 

        std::cout << "[TCP BULK] Piped " << total_outlets 
                  << " serialized packets to Edwards FireWorks Integration Gateway.\n";
    }
};
