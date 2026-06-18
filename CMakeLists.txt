cmake_minimum_required(VERSION 3.13)
if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

set(CLIENT_SOURCES
    main.cpp 
    parse_cl.cpp
    parse_cl.h
)

set(GETOPT_SOURCES
  getopt.cpp
  getopt.h
)

add_executable(datachannel-client ${CLIENT_SOURCES})

set_target_properties(datachannel-client PROPERTIES
  CXX_STANDART 17
  OUTPUT_NAME client 
)

set_target_properties(datachannel-client PROPERTIES
  XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER com.github.paullouisageneau.libdatachannel.exemple.client)

find_package(Threads REQUIRED)
find_package(LibDataChannel REQUIRED)
find_package(nlohmann_json REQUIRED)
target_link_libraries(datachannel-client LibDataChannel::LibDataChannel Threads::Threads nlohmann_json::nlohmann_json)

