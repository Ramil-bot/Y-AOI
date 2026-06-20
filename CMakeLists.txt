cmake_minimum_required(VERSION 3.13)

project(Messanger)

if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

file(GLOB_RECURSE CLIENT_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

set(GETOPT_SOURCES
  getopt.cpp
  getopt.h
)


set(LibDataChannel_STATIC_LINKING ON CACHE BOOL "Use static libdatachannel")
add_executable(Messanger ${CLIENT_SOURCES})

set_target_properties(Messanger PROPERTIES
  CXX_STANDARD 17
)

find_package(Threads REQUIRED)
find_package(LibDataChannel REQUIRED)
find_package(nlohmann_json REQUIRED)
target_link_libraries(Messanger LibDataChannel::LibDataChannel Threads::Threads nlohmann_json::nlohmann_json)

