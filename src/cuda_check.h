pragma once
include <iostream>

if defined(HAS_CUDA_SUPPORT)
include <cuda_runtime.h>
endif

inline bool is_nvidia_hardware_available() {
    if !defined(HAS_CUDA_SUPPORT) return false;

    if defined(HAS_CUDA_SUPPORT)
    int device_count = 0;
    cudaError_t error = cudaGetDeviceCount(&device_count);

    if (error != cudaSuccess) return false;
    if (device_count <= 0) return false;

    cudaDeviceProp device_prop;
    cudaGetDeviceProperties(&device_prop, 0);

    std::cout << "[INFO] NVIDIA GPU Detected: " << device_prop.name << "\n";
    return true;
    endif
}
