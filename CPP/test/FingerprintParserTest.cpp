#include <string>
#include <fstream>
#include <iostream>

// use the Boost unit-testing framework with its own main
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <sensorreadout/FingerprintParser.h>

using namespace SensorReadoutParser;

BOOST_AUTO_TEST_CASE ( fingerprintParserTest ) {
	std::ifstream fpFile("testFiles/fingerprints.dat");
	BOOST_CHECK(fpFile.is_open());
	FingerprintParser parser(fpFile);
	Fingerprints fps = parser.parse();

	{
		auto fp053 = fps.getFirstByName("FP 053");
		BOOST_CHECK_EQUAL(fp053.has_value(), true);
		BOOST_CHECK_EQUAL(fp053->get().name, "FP 053");
		BOOST_CHECK(fp053->get().fpType == FingerprintType::Point);
		BOOST_CHECK_EQUAL(fp053->get().parameters.size(), 4);
		BOOST_CHECK_EQUAL(fp053->get().parameters.at("name"), "FP 053");
		BOOST_CHECK_EQUAL(fp053->get().getFloorIdx(), 0);
		BOOST_CHECK_EQUAL(fp053->get().getFloorName(), "floor");
		auto pos = fp053->get().getPosition();
		BOOST_CHECK_CLOSE(pos[0], 12.6, 0.01);
		BOOST_CHECK_CLOSE(pos[1], 0.6, 0.01);
		BOOST_CHECK_CLOSE(pos[2], 1.3, 0.01);
		BOOST_CHECK_EQUAL(fp053->get().evts.size(), 14);
	}
	{
		auto fp1337 = fps.getFirstByName("FP 1337");
		BOOST_CHECK_EQUAL(fp1337.has_value(), true);
		BOOST_CHECK_EQUAL(fp1337->get().name, "FP 1337");
		BOOST_CHECK(fp1337->get().fpType == FingerprintType::Point);
		BOOST_CHECK_EQUAL(fp1337->get().parameters.size(), 5);
		BOOST_CHECK_EQUAL(fp1337->get().parameters.at("name"), "FP 1337");
		BOOST_CHECK_EQUAL(fp1337->get().parameters.at("paramWithEqInName"), "key=value");
		BOOST_CHECK_EQUAL(fp1337->get().getFloorIdx(), 1);
		BOOST_CHECK_EQUAL(fp1337->get().getFloorName(), "floorTest");
		auto pos = fp1337->get().getPosition();
		BOOST_CHECK_CLOSE(pos[0], 13.37, 0.01);
		BOOST_CHECK_CLOSE(pos[1], 4.2, 0.01);
		BOOST_CHECK_CLOSE(pos[2], 2.4, 0.01);
		BOOST_CHECK_EQUAL(fp1337->get().evts.size(), 13);
	}
	{
		auto fp099 = fps.getFirstByName("FP 099");
		BOOST_CHECK_EQUAL(fp099.has_value(), true);
		BOOST_CHECK_EQUAL(fp099->get().name, "FP 099");
		BOOST_CHECK(fp099->get().fpType == FingerprintType::Point);
		BOOST_CHECK_EQUAL(fp099->get().parameters.size(), 5);
		BOOST_CHECK_EQUAL(fp099->get().parameters.at("name"), "FP 099");
		BOOST_CHECK_EQUAL(fp099->get().parameters.at("additionalParam1"), "testvalue");
		BOOST_CHECK_EQUAL(fp099->get().getFloorIdx(), 2);
		BOOST_CHECK_EQUAL(fp099->get().getFloorName(), "floor1");
		auto pos = fp099->get().getPosition();
		BOOST_CHECK_CLOSE(pos[0], 13.37, 0.01);
		BOOST_CHECK_CLOSE(pos[1], 4.2, 0.01);
		BOOST_CHECK_CLOSE(pos[2], 2.4, 0.01);
		BOOST_CHECK_EQUAL(fp099->get().evts.size(), 12);
	}
	{
		auto fp02_07 = fps.getFirstByName("FP 02;FP 07");
		BOOST_CHECK_EQUAL(fp02_07.has_value(), true);
		BOOST_CHECK_EQUAL(fp02_07->get().name, "FP 02;FP 07");
		BOOST_CHECK(fp02_07->get().fpType == FingerprintType::Path);
		BOOST_CHECK_EQUAL(fp02_07->get().parameters.size(), 9);
		BOOST_CHECK_EQUAL(fp02_07->get().getFloorIdx(), 1337);
		BOOST_CHECK_EQUAL(fp02_07->get().getFloorName(), "floor42");
		{
			auto positions = fp02_07->get().getPositions();
			BOOST_CHECK_CLOSE(positions[0][0], 11.4, 0.01);
			BOOST_CHECK_CLOSE(positions[0][1], 3.6, 0.01);
			BOOST_CHECK_CLOSE(positions[0][2], 1.3, 0.01);
			BOOST_CHECK_CLOSE(positions[1][0], 8.8, 0.01);
			BOOST_CHECK_CLOSE(positions[1][1], 4.0, 0.01);
			BOOST_CHECK_CLOSE(positions[1][2], 1.3, 0.01);
		}
		{
			auto points = fp02_07->get().getPointNames();
			BOOST_CHECK_EQUAL(points[0], "FP 02");
			BOOST_CHECK_EQUAL(points[1], "FP 07");
		}
	}
}
