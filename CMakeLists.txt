cmake_minimum_required(VERSION 3.0.0)
project(can_library VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    can_device_database.cpp
    can_ecu.cpp
    can_message.cpp
    can_name.cpp
    can_processor.cpp
    can_utils.cpp
    local_ecu.cpp
    remote_ecu.cpp
    )

add_library(can_library can_library.cpp ${SOURCES})
target_include_directories(${PROJECT_NAME}
                            PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)