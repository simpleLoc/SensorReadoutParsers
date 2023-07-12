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
		BOOST_CHECK_EQUAL(fp053->get().parameters.size(), 1);
		BOOST_CHECK_EQUAL(fp053->get().parameters.at("name"), "FP 053");
		BOOST_CHECK_EQUAL(fp053->get().evts.size(), 14);
	}
	{
		auto fp053 = fps.getFirstByName("FP 1337");
		BOOST_CHECK_EQUAL(fp053.has_value(), true);
		BOOST_CHECK_EQUAL(fp053->get().name, "FP 1337");
		BOOST_CHECK(fp053->get().fpType == FingerprintType::Point);
		BOOST_CHECK_EQUAL(fp053->get().parameters.size(), 2);
		BOOST_CHECK_EQUAL(fp053->get().parameters.at("name"), "FP 1337");
		BOOST_CHECK_EQUAL(fp053->get().parameters.at("paramWithEqInName"), "key=value");
		BOOST_CHECK_EQUAL(fp053->get().evts.size(), 13);
	}
	{
		auto fp053 = fps.getFirstByName("FP 099");
		BOOST_CHECK_EQUAL(fp053.has_value(), true);
		BOOST_CHECK_EQUAL(fp053->get().name, "FP 099");
		BOOST_CHECK(fp053->get().fpType == FingerprintType::Point);
		BOOST_CHECK_EQUAL(fp053->get().parameters.size(), 2);
		BOOST_CHECK_EQUAL(fp053->get().parameters.at("name"), "FP 099");
		BOOST_CHECK_EQUAL(fp053->get().parameters.at("additionalParam1"), "testvalue");
		BOOST_CHECK_EQUAL(fp053->get().evts.size(), 12);
	}
}
