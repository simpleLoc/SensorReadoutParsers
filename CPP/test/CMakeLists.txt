enable_testing()

# use boost test framework
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

add_executable(ParserTest "ParserTest.cpp")
add_test(NAME ParserTest COMMAND ParserTest)
target_link_libraries(ParserTest SensorReadoutParser ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

# deploy test files
add_custom_command(TARGET ParserTest POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/testFiles" "${CMAKE_CURRENT_BINARY_DIR}/testFiles")
