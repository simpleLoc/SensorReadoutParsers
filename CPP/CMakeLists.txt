cmake_minimum_required(VERSION 3.14)

project(SensorReadoutParser LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# PROJECT SETTINGS
option(WITH_TESTS "Whether to build tests" OFF)

# LIBRARY
file(GLOB PARSER_SOURCES "src/*.cpp")
file(GLOB_RECURSE PARSER_HEADERS "include/*.h")
add_library(SensorReadoutParser SHARED ${PARSER_SOURCES} ${PARSER_HEADERS})
target_include_directories(SensorReadoutParser PUBLIC "include/")
target_compile_definitions(SensorReadoutParser PRIVATE SENSORREADOUTPARSER_LIBRARY)

# UNIT TESTS
if(WITH_TESTS)
    add_subdirectory(test)
endif()
