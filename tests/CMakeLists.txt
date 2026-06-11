cmake_minimum_required(VERSION 3.15)
project(EnvironmentSafetyTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Fetch Catch2 testing library framework natively from sources
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com
  GIT_TAG        v3.4.0 # Stable Release
)
FetchContent_MakeAvailable(Catch2)

# Define the local unit testing binary target executable
add_executable(run_unit_tests test_alarm_latch.cpp)

# Point the testing script to your production includes directory
target_include_directories(run_unit_tests PRIVATE ../include)

# Link the Catch2 framework libraries to the test target binary
target_link_libraries(run_unit_tests PRIVATE Catch2::Catch2WithMain)
