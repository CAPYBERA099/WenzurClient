cmake_minimum_required(VERSION 3.15)
project(WenzGuard VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-only
if(NOT WIN32)
    message(FATAL_ERROR "WenzGuard работает только на Windows")
endif()

add_executable(WenzGuard
    src/main.cpp
    src/monitor.cpp
    src/browser_paths.cpp
    src/alert.cpp
    src/logger.cpp
    src/process_scanner.cpp
)

target_include_directories(WenzGuard PRIVATE include)

# Windows API libraries
target_link_libraries(WenzGuard PRIVATE
    user32
    shell32
    advapi32
    psapi
    shlwapi
)

# Unicode
target_compile_definitions(WenzGuard PRIVATE UNICODE _UNICODE)

# Оптимизация для Release
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(WenzGuard PRIVATE /O2)
endif()
