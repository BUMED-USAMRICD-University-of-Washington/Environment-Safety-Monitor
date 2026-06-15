cmake_minimum_required(VERSION 3.15)
project(EnvironmentSafetyMonitor VERSION 1.0.0 LANGUAGES CXX)
find_package(CURL REQUIRED)
target_link_libraries(EnvironmentMonitor PRIVATE CURL::libcurl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CheckLanguage)
check_language(CUDA)

if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
    message(STATUS ">>> NVIDIA CUDA Toolkit compiler verified. Activating GPU optimization tracks.")
    add_compile_definitions(HAS_CUDA_SUPPORT)
    file(GLOB_RECURSE CUDA_SOURCES "src/*.cu")
    list(APPEND SOURCES ${CUDA_SOURCES})
endif()

if(NOT CMAKE_CUDA_COMPILER)
    message(STATUS ">>> NVIDIA CUDA Toolkit not found. Building pure multi-core CPU executable target profile.")
    file(GLOB_RECURSE SOURCES "src/*.cpp")
endif()

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(
        -Wall 
        -Wextra 
        -Werror
        -O2
        -fno-exceptions
        -fno-rtti
    )
endif()

include_directories(include)

set(SOURCE_FILES
    src/main.cpp
    src/max31856.cpp
    src/oxygen_sensor.cpp
    src/crash_logger.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE include)

message(STATUS "Configured Safety-Critical Engine: [${PROJECT_NAME}]")
message(STATUS "Strict C++ Optimization Stack Enabled (-O2 -Werror -fno-exceptions)")

option(BUILD_TESTING "Build the automated testing framework" OFF)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
