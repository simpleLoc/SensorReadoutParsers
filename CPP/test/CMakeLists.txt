enable_testing()

# use boost test framework
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

file(GLOB_RECURSE TEST_FILES "testFiles/*.csv" "testFiles/*.dat")
source_group("testFiles" FILES ${TEST_FILES})

add_executable(ParserTest "ParserTest.cpp" ${TEST_FILES})
add_test(NAME ParserTest COMMAND ParserTest)
target_link_libraries(ParserTest SensorReadoutParser ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_executable(FingerprintParserTest "FingerprintParserTest.cpp")
add_test(NAME FingerprintParserTest COMMAND FingerprintParserTest)
target_link_libraries(FingerprintParserTest SensorReadoutParser ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_executable(SerializerTest "SerializerTest.cpp")
add_test(NAME SerializerTest COMMAND SerializerTest)
target_link_libraries(SerializerTest SensorReadoutParser ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

# deploy test files
add_custom_command(TARGET ParserTest POST_BUILD COMMAND
	${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/testFiles" "${CMAKE_CURRENT_BINARY_DIR}/testFiles")
