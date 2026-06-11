""" NJIT Mathematical Acceleration Engine """
import time
import numpy as np
from numba import njit

LIVE_ZERO_VOLTAGE = 0.85
MAX_SENSOR_VOLTAGE = 4.00
VOLTAGE_SPAN = 3.15
O2_SPAN = 100.0

@njit
def verify_sensor_math_fast(raw_voltages):
    """ High-Speed Simulation Engine resolving operations in microseconds """
    num_samples = len(raw_voltages)
    scaled_o2 = np.zeros(num_samples, dtype=np.float32)
    fault_flags = np.zeros(num_samples, dtype=np.bool_)
    
    for i in range(num_samples):
        v = raw_voltages[i]
        
        if v < LIVE_ZERO_VOLTAGE:
            fault_flags[i] = True
            scaled_o2[i] = 0.0
            continue
            
        clamped_v = v
        if clamped_v > MAX_SENSOR_VOLTAGE:
            clamped_v = MAX_SENSOR_VOLTAGE
            
        scaled_o2[i] = ((clamped_v - LIVE_ZERO_VOLTAGE) * O2_SPAN) / VOLTAGE_SPAN
        
    return scaled_o2, fault_flags

def export_cpp_lut():
    """ Exports static array for zero-latency microcontroller lookup """
    test_voltages = np.linspace(0.85, 4.00, 256, dtype=np.float32)
    o2_values, faults = verify_sensor_math_fast(test_voltages)
    
    print("constexpr float O2_LOOKUP_TABLE[256] = {")
    for val in o2_values:
        print(f"    {val:.2f}f,")
    print("};")

if __name__ == "__main__":
    export_cpp_lut()
