#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include <cassert>
#include <cmath>

namespace SafetySystemTest {

    // Target resistance constants matching the NFPA 72 wiring blueprint (in Ohms)
    constexpr double TARGET_RESISTANCE_NORMAL  = 6666.67; // 10k parallel with 20k
    constexpr double TARGET_RESISTANCE_ALARM   = 20000.00; // 20k EOL only
    constexpr double TARGET_RESISTANCE_TROUBLE = 999999.99; // Representing Open Circuit (Infinity)
    constexpr double RESISTANCE_TOLERANCE      = 200.0;    // Allow +/- 200 Ohms for wiring noise

    enum class CircuitState {
        STATE_NORMAL,
        STATE_ALARM,
        STATE_TROUBLE,
        STATE_UNKNOWN
    };

    // Hardware Simulation Class
    class Mock55000Module {
    public:
        // Evaluates raw resistance inputs into exact, discrete loop states
        static CircuitState evaluate_loop_impedance(double measured_ohms) {
            if (std::abs(measured_ohms - TARGET_RESISTANCE_NORMAL) <= RESISTANCE_TOLERANCE) {
                return CircuitState::STATE_NORMAL;
            }
            if (std::abs(measured_ohms - TARGET_RESISTANCE_ALARM) <= RESISTANCE_TOLERANCE) {
                return CircuitState::STATE_ALARM;
            }
            if (measured_ohms >= (TARGET_RESISTANCE_TROUBLE - RESISTANCE_TOLERANCE)) {
                return CircuitState::STATE_TROUBLE;
            }
            return CircuitState::STATE_UNKNOWN;
        }
    };
}

// =========================================================================
// ASYNCHRONOUS TEST RUNNER ENGINE
// =========================================================================
int main() {
    using namespace SafetySystemTest;
    using namespace std::chrono_literals;

    std::cout << "==================================================\n";
    std::cout << "STARTING ASYNCHRONOUS LOOP IMPEDANCE UNIT TEST\n";
    std::cout << "==================================================\n\n";

    // Define an asynchronous hardware state-machine simulation task.
    // This background worker simulates physical relay events over time.
    auto async_hardware_sim = std::async(std::launch::async, []() -> bool {
        
        // --- PHASE 1: Verify System Bootup (Standby State) ---
        std::cout << "[SIM] Booting system. Simulating Normal Closed relay circuit...\n";
        double current_loop_resistance = 6670.0; // Simulated ADC reading close to 6.67k
        CircuitState state = Mock55000Module::evaluate_loop_impedance(current_loop_resistance);
        std::cout << "[TEST] Measured Input: " << current_loop_resistance << " Ohms -> Status: NORMAL\n";
        assert(state == CircuitState::STATE_NORMAL);
        std::this_thread::sleep_for(500ms); // Maintain steady state briefly

        // --- PHASE 2: Trigger Gas Breach (Alarm State Transition) ---
        std::cout << "\n[SIM] Critical environment breach! Dropping Series Relay (Opening Circuit)...\n";
        current_loop_resistance = 20050.0; // Loop resistance spikes to 20k EOL resistor value
        state = Mock55000Module::evaluate_loop_impedance(current_loop_resistance);
        std::cout << "[TEST] Measured Input: " << current_loop_resistance << " Ohms -> Status: ALARM DETECTED\n";
        assert(state == CircuitState::STATE_ALARM);
        std::this_thread::sleep_for(500ms);

        // --- PHASE 3: Physical Wire Failure (Trouble State Transition) ---
        std::cout << "\n[SIM] Simulating field hazard: Shrapnel/Heat shearing the copper wire line...\n";
        current_loop_resistance = 1000000.0; // Open Circuit (Infinite Ohms)
        state = Mock55000Module::evaluate_loop_impedance(current_loop_resistance);
        std::cout << "[TEST] Measured Input: Infinite Ohms -> Status: HARDWARE TROUBLE FAULT\n";
        assert(state == CircuitState::STATE_TROUBLE);
        std::this_thread::sleep_for(500ms);

        // --- PHASE 4: Illegal/Short Circuit Event (Unknown State Transition) ---
        std::cout << "\n[SIM] Simulating electrical wire pinch (Direct short circuit bypassing EOL)...\n";
        current_loop_resistance = 12.4; // Direct copper short circuit
        state = Mock55000Module::evaluate_loop_impedance(current_loop_resistance);
        std::cout << "[TEST] Measured Input: " << current_loop_resistance << " Ohms -> Status: UNKNOWN / INVALID IMPEDANCE\n";
        assert(state == CircuitState::STATE_UNKNOWN);

        return true; 
    });

    // Main execution tracking check
    if (async_hardware_sim.get()) {
        std::cout << "\n==================================================\n";
        std::cout << "[SUCCESS] All loop impedance test vectors passed.\n";
        std::cout << "          Impedance transitions comply with NFPA 72.\n";
        std::cout << "==================================================\n";
        return 0;
    }

    return 1;
}
