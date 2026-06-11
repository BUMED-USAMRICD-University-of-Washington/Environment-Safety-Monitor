#pragma once
#include <iostream>

#if defined(HAS_CUDA_SUPPORT)
    #include <cuda_runtime.h>
#endif

inline bool is_nvidia_hardware_available() {
#if defined(HAS_CUDA_SUPPORT)
    int device_count = 0;
    cudaError_t error = cudaGetDeviceCount(&device_count);
    
    if (error == cudaSuccess && device_count > 0) {
        // Fetch and print the model name for deployment engineering logs
        cudaDeviceProp device_prop;
        cudaGetDeviceProperties(&device_prop, 0);
        std::cout << "[INFO] NVIDIA GPU Detected: " << device_prop.name << "\n";
        std::cout << "       Compute Capability: " << device_prop.major << "." << device_prop.minor << "\n";
        return true;
    }
    std::cout << "[WARN] NVIDIA hardware driver present but no active device found. Falling back to CPU.\n";
    return false;
#else
    std::cout << "[INFO] CUDA support not compiled into this binary build. Using multi-core CPU engine.\n";
    return false;
#endif
}
