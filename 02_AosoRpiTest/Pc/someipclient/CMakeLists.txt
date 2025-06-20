# Copyright (C) 2015-2017 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.13)
project(vSomeIPHelloWorldClient)

# Compiler flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Find packages
find_package(vsomeip3 3.4.10 REQUIRED)
find_package(Boost 1.55 REQUIRED COMPONENTS system thread log)
find_package(Threads REQUIRED)

# Optional alias for compatibility
if (TARGET vsomeip3::vsomeip3 AND NOT TARGET vsomeip3)
    add_library(vsomeip3 ALIAS vsomeip3::vsomeip3)
endif()

# Include directories
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# === Client Header-only Interface ===
add_library(vsomeip_hello_world_client INTERFACE)
target_sources(vsomeip_hello_world_client INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/hello_world_client.hpp"
)
target_include_directories(vsomeip_hello_world_client INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# === hello_world_client Executable ===
add_executable(hello_world_client
    hello_world_client_main.cpp
)
target_link_libraries(hello_world_client
    vsomeip_hello_world_client
    vsomeip3
    ${Boost_LIBRARIES}
    Threads::Threads
)
