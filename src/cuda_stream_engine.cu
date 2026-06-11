include "cuda_check.h"
include "telemetry_types.h"
include <iostream>

if defined(HAS_CUDA_SUPPORT)
include <cuda_runtime.h>

define LIVE_ZERO_VOLTAGE 0.85f
define MAX_SENSOR_VOLTAGE 4.00f
define O2_SPAN 100.0f
define VOLTAGE_SPAN_INV 0.3174603f 

__global__ void process_usb_stream_kernel(
    const float* __restrict__ d_voltages, 
    const float* __restrict__ d_temps, 
    SafetySystem::AlarmTelemetryPacket* __restrict__ d_outputs, 
    int num_outlets, 
    uint32_t current_time_ms
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= num_outlets) return;

    float v = d_voltages[idx];
    float t = d_temps[idx];

    SafetySystem::AlarmTelemetryPacket pkt;
    pkt.timestamp_ms = current_time_ms;
    pkt.loop_counter = idx;
    pkt.alarm_state = SafetySystem::STATE_NORMAL;

    if (v < LIVE_ZERO_VOLTAGE) {
        pkt.alarm_state |= SafetySystem::FAULT_O2_WIRE_BREAK;
        pkt.oxygen_level = 0.0f;
    }

    if (v >= LIVE_ZERO_VOLTAGE) {
        float clamped_v = v;
        if (clamped_v > MAX_SENSOR_VOLTAGE) clamped_v = MAX_SENSOR_VOLTAGE;
        
        pkt.oxygen_level = (clamped_v - LIVE_ZERO_VOLTAGE) * O2_SPAN * VOLTAGE_SPAN_INV;
    }

    pkt.floor_temp = t;

    if (pkt.oxygen_level <= 19.5f) pkt.alarm_state |= SafetySystem::WARN_O2_DEVIATION;
    if (pkt.oxygen_level <= 18.0f) pkt.alarm_state |= SafetySystem::CRIT_O2_HYPOXIA;
    if (pkt.floor_temp <= 0.0f) pkt.alarm_state |= SafetySystem::CRIT_CRYO_FAILURE;

    d_outputs[idx] = pkt;
}

void execute_dual_stream_pipeline(float* h_voltages, float* h_temps, SafetySystem::AlarmTelemetryPacket* h_outputs, int total_outlets) {
    if (total_outlets <= 0) return;

    float *d_voltages, *d_temps;
    SafetySystem::AlarmTelemetryPacket *d_outputs;

    cudaMalloc(&d_voltages, total_outlets * sizeof(float));
    cudaMalloc(&d_temps, total_outlets * sizeof(float));
    cudaMalloc(&d_outputs, total_outlets * sizeof(SafetySystem::AlarmTelemetryPacket));

    cudaStream_t stream0, stream1;
    cudaStreamCreate(&stream0);
    cudaStreamCreate(&stream1);

    int half_batch = total_outlets >> 1; 

    cudaMemcpyAsync(d_voltages, h_voltages, half_batch * sizeof(float), cudaMemcpyHostToDevice, stream0);
    cudaMemcpyAsync(d_temps, h_temps, half_batch * sizeof(float), cudaMemcpyHostToDevice, stream0);
    
    int threadsPerBlock = 256;
    int blocksPerGrid = (half_batch + threadsPerBlock - 1) / threadsPerBlock;
    
    process_usb_stream_kernel<<<blocksPerGrid, threadsPerBlock, 0, stream0>>>(d_voltages, d_temps, d_outputs, half_batch, 0);
    
    cudaMemcpyAsync(h_outputs, d_outputs, half_batch * sizeof(SafetySystem::AlarmTelemetryPacket), cudaMemcpyDeviceToHost, stream0);

    cudaStreamSynchronize(stream0);
    cudaStreamSynchronize(stream1);

    cudaFree(d_voltages);
    cudaFree(d_temps);
    cudaFree(d_outputs);
    cudaStreamDestroy(stream0);
    cudaStreamDestroy(stream1);
}
endif
