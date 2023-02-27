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

void parseRawSerializeEqualityFile(const std::string& filePath) {
	std::fstream inputFile(filePath);
	BOOST_REQUIRE(inputFile.is_open());
	std::string outputFileName = filePath + "_outRaw";
	std::filesystem::remove(outputFileName);
	std::fstream outputFile;
	outputFile.open(outputFileName, std::ios::out);
	BOOST_REQUIRE(outputFile.is_open());

	{ // parse raw and serialize back
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

void parseSerializeEqualityFile(const std::string& filePath) {
	// This function parses the file, parses the raw events into typed events
	// serializes the typed events out into the "[...]_out" file.

	// This file is then parsed, the events are parsed into typed events
	// and the output of <typed event>.serializeInto() is compared to the raw
	// event parsed from the generated _out file.

	// This is so complicated, because float formatting makes direct comparisons
	// with a file provided by another application basically impossible.

	std::fstream inputFile(filePath);
	BOOST_REQUIRE(inputFile.is_open());
	std::string outputFileName = filePath + "_out";
	std::filesystem::remove(outputFileName);

	{ // parse and serialize back
		AggregatingParser parser(inputFile);
		auto inputResult = parser.parse();
		// first serializer run
		std::ofstream outputFile(outputFileName);
		BOOST_REQUIRE(outputFile.is_open());
		Serializer serializer(outputFile);
		std::for_each(inputResult.begin(), inputResult.end(), [&](const auto& evt){ serializer.write(evt); });
	}
	{ // parse generated output file, and test based on that
		std::ifstream outputFile(outputFileName);
		BOOST_REQUIRE(outputFile.is_open());
		AggregatingParser parser(outputFile);
		auto inputResult = parser.parseRaw();

		RawSensorEvent evtRawOut;
		for(const auto& evtRawIn : inputResult) {
			SensorEvent sensorEvent = SensorEvent::parse(evtRawIn);
			sensorEvent.serializeInto(evtRawOut);
			BOOST_CHECK_EQUAL(evtRawIn.eventId, evtRawOut.eventId);
			BOOST_CHECK_EQUAL(evtRawIn.timestamp, evtRawOut.timestamp);
			BOOST_CHECK_EQUAL(evtRawIn.parameterString, evtRawOut.parameterString);
		}
	}
}


BOOST_AUTO_TEST_CASE ( parseSerializeEquality ) {
	parseRawSerializeEqualityFile("testFiles/radioData.csv");
	parseRawSerializeEqualityFile("testFiles/sensorData.csv");
	parseRawSerializeEqualityFile("testFiles/customActivity.csv");

	parseSerializeEqualityFile("testFiles/radioData.csv");
	parseSerializeEqualityFile("testFiles/sensorData.csv");
	parseSerializeEqualityFile("testFiles/customActivity.csv");
}
