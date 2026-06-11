#include <vector>
#include <cstring>
#include "serialization_protocol.h"
#include "crc32.h"

class NetworkSerializerModule {
public:
    NetworkSerializerModule() : sequence_counter_(0) {}

    void serialize_and_pipe_to_fireworks(const SafetySystem::AlarmTelemetryPacket* packets, int total_outlets) {
        using namespace SafetySystem;

        // 1. Calculate buffer segment boundaries
        size_t header_size  = sizeof(EdwardsBulkFrameHeader);
        size_t payload_size = total_outlets * sizeof(AlarmTelemetryPacket);
        size_t tail_size    = sizeof(EdwardsBulkFrameTail);
        size_t total_frame_size = header_size + payload_size + tail_size;

        std::vector<uint8_t> transmission_buffer(total_frame_size);

        // 2. Formulate Header
        EdwardsBulkFrameHeader header;
        header.start_byte = FRAME_START_BYTE;
        header.total_records = static_cast<uint16_t>(total_outlets);
        header.message_sequence = sequence_counter_++;

        // 3. Sequential Copy Step A: Move header and payload into the contiguous byte array
        std::memcpy(transmission_buffer.data(), &header, header_size);
        std::memcpy(transmission_buffer.data() + header_size, packets, payload_size);

        // 4. Calculate Checksum across the combined Header + Payload data array segments
        size_t bytes_to_checksum = header_size + payload_size;
        uint32_t computed_crc = CRC32Engine::calculate(transmission_buffer.data(), bytes_to_checksum);

        // 5. Formulate and Append Tail
        EdwardsBulkFrameTail tail;
        tail.checksum = computed_crc;
        tail.end_byte = FRAME_END_BYTE;

        // 6. Sequential Copy Step B: Append the trailing tail structure to the end
        std::memcpy(transmission_buffer.data() + bytes_to_checksum, &tail, tail_size);

        // 7. Dispatch verified data block downstream to the network or serial ports
        write_to_fireworks_integration_channel(transmission_buffer.data(), total_frame_size);
    }

private:
    uint32_t sequence_counter_;
    void write_to_fireworks_integration_channel(const uint8_t* data, size_t size) {
        // System hardware output hook...
    }
};
