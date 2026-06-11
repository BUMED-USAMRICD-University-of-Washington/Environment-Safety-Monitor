cmake_minimum_required(VERSION 3.15)
project(EnvironmentSafetyMonitor VERSION 1.0.0 LANGUAGES CXX)

# --- COMPILER CONFIGURATION & LIFE-SAFETY EXTRA ENFORCEMENTS ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Prevent compiler-specific language hacks

# Force standard embedded cross-compilation warning profiles
if(MSVC)
    add_compile_options(/W4 /WX) # Treat warnings as errors on Windows build tests
else()
    add_compile_options(
        -Wall 
        -Wextra 
        -Werror              # Stop build instantly if any warning is tripped
        -O2                  # Strict optimization to keep loop executions deterministic
        -fno-exceptions      # Disable overhead exceptions for memory-safe embedded runs
        -fno-rtti            # Disable runtime type info overhead
    )
endif()

# --- DEFINE DIRECTORY PATTERNS ---
# Explicitly announce header lookup spaces
include_directories(include)

# Gather all compiled safety-critical modules from your workspace folders
set(SOURCE_FILES
    src/main.cpp
    src/max31856.cpp
    src/oxygen_sensor.cpp
    src/crash_logger.cpp
)

# --- BUILD TARGET ---
# Compile the unified binary
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Label target definitions cleanly for modular file tracking
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Provide terminal user info upon compilation generation
message(STATUS "Configured Safety-Critical Engine: [${PROJECT_NAME}]")
message(STATUS "Strict C++ Optimization Stack Enabled (-O2 -Werror -fno-exceptions)")
# Add this code snippet to the bottom of your root CMakeLists.txt file

# Include test engine compilation targets if running test sandboxes locally
option(BUILD_TESTING "Build the automated testing framework" OFF)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
