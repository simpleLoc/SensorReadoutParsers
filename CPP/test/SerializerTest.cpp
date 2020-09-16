#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>

// use the Boost unit-testing framework with its own main
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <sensorreadout/SensorReadoutParser.h>

using namespace SensorReadoutParser;
using namespace _internal;

// ###########
// # Serializer
// ######################

void parseSerializeEqualityFile(const std::string& filePath) {
	std::fstream inputFile(filePath);
	BOOST_REQUIRE(inputFile.is_open());
	std::string outputFileName = filePath + "_out";
	std::filesystem::remove(outputFileName);
	std::fstream outputFile;
	outputFile.open(outputFileName, std::ios::out);
	BOOST_REQUIRE(outputFile.is_open());

	{ // parse and serialize back
		VisitingParser parser(inputFile);
		Serializer serializer(outputFile);

		RawSensorEvent sensorEvent;
		while(parser.nextLine(sensorEvent)) {
			serializer.write(sensorEvent);
		}
	}

	// jump to the start of both files
	inputFile.clear();
	inputFile.seekg(0);
	BOOST_REQUIRE(inputFile.tellg() == 0);
	outputFile.close();
	outputFile.open(outputFileName, std::ios::in);
	BOOST_REQUIRE(outputFile.is_open());
	// compare original and generated file
	std::string inputFileBuffer;
	std::string outputFileBuffer;
	while(std::getline(inputFile, inputFileBuffer)) {
		BOOST_REQUIRE(std::getline(outputFile, outputFileBuffer));
		BOOST_CHECK_EQUAL(inputFileBuffer, outputFileBuffer);
	}
}

BOOST_AUTO_TEST_CASE ( parseSerializeEquality ) {
	parseSerializeEqualityFile("testFiles/radioData.csv");
	parseSerializeEqualityFile("testFiles/sensorData.csv");
}
