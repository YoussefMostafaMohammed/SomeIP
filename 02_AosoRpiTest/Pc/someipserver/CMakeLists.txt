cmake_minimum_required(VERSION 3.13)
project(vSomeIPHelloWorld)

# Compiler flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Find packages
find_package(vsomeip3 3.4.10 REQUIRED)
find_package(Boost 1.55 REQUIRED COMPONENTS system thread log)
find_package(Threads REQUIRED)

# Optional: Alias modern imported target to legacy name for compatibility
if (TARGET vsomeip3::vsomeip3 AND NOT TARGET vsomeip3)
    add_library(vsomeip3 ALIAS vsomeip3::vsomeip3)
endif()

# Include directories
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# === Service Header-only Interface ===
add_library(vsomeip_hello_world_service INTERFACE)
target_sources(vsomeip_hello_world_service INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/hello_world_service.hpp
)
target_include_directories(vsomeip_hello_world_service INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# === hello_world_service Executable ===
add_executable(hello_world_service
    hello_world_service_main.cpp
)
target_link_libraries(hello_world_service
    vsomeip_hello_world_service
    vsomeip3
    ${Boost_LIBRARIES}
    Threads::Threads
)
