# Environment Safety Monitor

An industrial, life-safety-critical environmental monitoring system designed for public corridors housing liquid nitrogen ($LN_2$) freezers. This system integrates real-time oxygen deficiency monitoring, cryogenic surface tracking, deterministic fault filtering, non-volatile crash telemetry logging, and a physical fail-safe bridge to building-wide automated emergency management frameworks.

---

## 🏢 System Safety Architecture

When cryogenic liquid nitrogen vaporizes, it expands by a factor of 700, immediately creating a severe asphyxiation risk by displacing breathable oxygen. This system uses a dedicated microcontroller to implement an automated safety loop that measures physical phenomena across two distinct zones:

Use code with caution.[Breathing Zone: 1.5m] ----> O2 Deficiency Monitor (4-20mA Loop)|[Floor Line: 30cm]     ----> Cryo Temperature (Type-T Probe) & Fog Tracking Array|v[Microcontroller C++ Safety Loop]|+---> [Local Alarm] (Pulsed Buzzer / Amber LED Matrix)|+---> [Edwards FireWorks Loop via NC Dry Contact Relay]
### Two-Tier Vertical Sensor Layout
*   **Floor Level (30 cm / 12 inches):** Cryogenic temperature probes (Type-T Thermocouples via MAX31856) are mounted on the legs or lower walls. Because boiling liquid nitrogen is freezing cold and forms a heavy fog, the floor level experiences the earliest drop in temperature.
*   **Breathing Zone (1.5 meters / 5 feet):** Zirconium oxide Oxygen ($O_2$) Deficiency Monitors are mounted here to measure the actual air atmosphere pedestrians breathe. They are mapped to cross-check safe air content against an **OSHA Critical Limit of 19.5%**.

### Edwards FireWorks Relay Integration
To maintain strict compliance with life-safety standards, the system interfaces with an **Edwards FireWorks Incident Management Platform** using an Addressable Monitor Module (such as an Edwards SIGA-CT1).
*   **The Fail-Safe Loop Pattern:** The system utilizes the **Normally Closed (NC)** and **Common (COM)** terminals of an active-high hardware relay loop. 
*   **The Hardware Rule:** During normal operations, the microcontroller continuously holds the GPIO output pin `HIGH` to energize the relay and maintain a complete, closed circuit. If the computer crashes, loses building power, or hits a sensor violation, its pin drops to zero, dropping power to the magnet. The circuit pops **OPEN** instantly, causing the Edwards panel to trigger an immediate building-wide Priority 1 Incident and evacuation response.

---

## 🛠️ Codebase Structure

The codebase is strictly modularized to maximize readability, maintain absolute zero dynamic memory allocation to prevent heap fragmentation, and maintain deterministic execution loops:

*   `include/alarm_latch.h`: Manages non-blocking **Time-at-Threshold** countdowns (3-second continuous grace periods for $O_2$ diffusion to filter out safe, transient dust or minor maintenance vents).
*   `include/buzzer_driver.h` & `led_driver.h`: Powers independent, non-blocking local audio beeps and color-coded status blinks to visually signal warnings before building-wide alerts lock in.
*   `include/crash_logger.h`: Leverages the on-chip non-volatile EEPROM to read hardware boot codes upon restarts, cleanly cataloging if a reboot was caused by a standard plug-in, a local brownout, or a hardware watchdog timeout.
*   `include/max31856.h`: Houses bitmasks and sequential SPI register transport drivers to command the Type-T cryogenic evaluation amplifier.
*   `include/oxygen_sensor.h`: Maps raw ADC voltages linearly to an $O_2$ percentage scale, enforcing the industrial **"Live Zero"** protocol (Voltages below 0.85V represent a current draw lower than 4mA, meaning a wire broke or sheered off).
*   `include/test_button.h`: Provides debounced, momentary-only diagnostic button checks that prioritize live safety checks over physical test procedures.
*   `src/main.cpp`: Synthesizes components inside a deterministic 100ms non-blocking safety thread loop.

---

## 💻 Compilation and Local Execution

### Prerequisites
*   CMake 3.15 or higher
*   A C++17 compliant compiler toolset (`gcc`, `clang`, or `MSVC`)

### Compilation Commands

To execute a local sandbox optimization build testing the repository modules, execute the following commands inside your terminal workspace:

### Navigate to your local project root directory space
cd Environment-Safety-Monitor

### Create a clean, temporary build sandboxing directory
mkdir build && cd build

### Generate native platform build makefiles with strict compilation profiles
cmake ..

### Run the project compiler to generate the target runtime binary executable
cmake --build .

---

## 🐍 Interactive Python CLI Companion Tool

The repository provides a companion utility inside `tools/doc_generator.py` powered by **Typer** and **Rich**. This command-line interface tool allows facilities engineers to verify threshold compliance, output automated Markdown audit documentation ledgers, or interface via a physical USB Serial connection to pull live diagnostic hardware registries straight onto a laptop screen.

### Python Environment Installation

Prior to executing CLI tasks, configure required dependency packages inside your tracking workspace:

## Install required modules natively via your package environment manager
pip install -r requirements.txt

### Complete CLI Tool Command List

To discover, audit, or calibrate field hardware units deployed along facility corridors, utilize the following precise command sequences:

## 1. View the main interactive global application help guide menu
python tools/doc_generator.py --help

## 2. View specific usage documentation parameters for the threshold verification engine
python tools/doc_generator.py verify-thresholds --help

## 3. View arguments and configuration rules for compliance document generation
python tools/doc_generator.py generate-readme --help

## 4. View connection rules for streaming serial registry data streams over USB links
python tools/doc_generator.py read-telemetry --help

## 5. Extract live thresholds directly from C++ files and output an audit matrix table
python tools/doc_generator.py verify-thresholds --main-file src/main.cpp --o2-file include/oxygen_sensor.h

## 6. Extract code variables and print a threshold table using standard default file maps
python tools/doc_generator.py verify-thresholds

## 7. Auto-compile a fresh, standardized markdown compliance report document file
python tools/doc_generator.py generate-readme --output-path README_COMPLIANCE.md

## 8. Generate an audit report markdown ledger using default root destination file configurations
python tools/doc_generator.py generate-readme

## 9. Establish a physical serial connection to stream live telemetry from a specific hardware port
python tools/doc_generator.py read-telemetry --port /dev/ttyUSB0 --baudrate 9600 --timeout 2.0

## 10. Open a telemetry stream connection link utilizing standard platform default variables
python tools/doc_generator.py read-telemetry

## 📈 Compliance Matrix Rules For Facility Auditors

| Target Parameter | System Configuration Value | Compliance Evaluation Criteria |
| :--- | :--- | :--- |
| **Oxygen Alarm Threshold** | `19.5%` | Calibrated to the minimum safe breathing boundary dictated by OSHA regulations. |
| **Cryo Trip Floor Limit** | `0.0°C` | Deployed along floor lines to intercept liquid flash dumps instantly bypassing delays. |
| **Time-at-Threshold Grace** | `3000ms (3 Seconds)` | Continuous confirmation gate protecting the facility against transient dust spike alerts. |
| **Microcontroller Safety Rate** | `100ms (10Hz Sweep)` | Guaranteed deterministic observation window ensuring rapid threat isolation loop sweeps. |
| **Hardware Fail-Safe Method** | `Normally Closed (NC)` | Circuit loops drop mechanically if software freezes, wires snap, or power completely cuts out. |
| **Watchdog Reboot Window** | `2000ms
## 11. Compile your system repository with unit testing modes fully activated
mkdir build && cd build
cmake -DBUILD_TESTING=ON ..
cmake --build .

## 12. Run the compiled automated test suite to confirm algorithms match safety laws
./tests/run_unit_tests

## 13. Pray for those who lost their lives to nitrogen leaks.


https://open.spotify.com/track/133L5cGDGW4puIjxY5GPTk?si=6ae4e31939aa4a06

```
