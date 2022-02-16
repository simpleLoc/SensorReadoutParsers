#include <string>
#include <fstream>
#include <iostream>

// use the Boost unit-testing framework with its own main
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <sensorreadout/SensorReadoutParser.h>

using namespace SensorReadoutParser;
using namespace _internal;

// ###########
// # Helpers
// ######################
BOOST_AUTO_TEST_CASE ( tokenizerTest ) {
	{
		Tokenizer<';'> tokenizer(";");
		BOOST_CHECK_NO_THROW(tokenizer.next());
		BOOST_CHECK_NO_THROW(tokenizer.next());
		BOOST_CHECK_THROW(tokenizer.next(), std::runtime_error);
	}
	{
		Tokenizer<';'> tokenizer("a;b;ddddd;d;d;");
		std::string buffer;
		BOOST_CHECK_NO_THROW(buffer = tokenizer.next());
		BOOST_CHECK_EQUAL(buffer, "a");
		BOOST_CHECK_NO_THROW(buffer = tokenizer.next());
		BOOST_CHECK_EQUAL(buffer, "b");
		BOOST_CHECK_NO_THROW(buffer = tokenizer.remainder());
		BOOST_CHECK_EQUAL(buffer, "ddddd;d;d;");
		BOOST_CHECK_THROW(tokenizer.next(), std::runtime_error);
	}
	{ // MAC
		Tokenizer<';'> tokenizer("4C11AEEFE8BE;1.0");
		MacAddress mac = tokenizer.nextAs<MacAddress>();
		BOOST_CHECK_EQUAL(mac.toString(), "4C11AEEFE8BE");
		BOOST_CHECK_EQUAL(mac.toColonDelimitedString(), "4C:11:AE:EF:E8:BE");
		BOOST_CHECK_EQUAL(tokenizer.nextAs<float>(), 1.0);
	}
	{ // invalid MAC
		Tokenizer<';'> tokenizer("4C11AEEFE8B;1.0");
		BOOST_CHECK_THROW(tokenizer.nextAs<MacAddress>(), std::runtime_error);
	}
	{ // UUID
		Tokenizer<';'> tokenizer("c976c52c-0e9c-4dfe-9b4a-4dfe46b1a8bb;1.0");
		UUID uuid = tokenizer.nextAs<UUID>();
		BOOST_CHECK_EQUAL(uuid.toString(), "c976c52c-0e9c-4dfe-9b4a-4dfe46b1a8bb");
		BOOST_CHECK_EQUAL(tokenizer.nextAs<float>(), 1.0);
	}
	{ // invalid UUID
		Tokenizer<';'> tokenizer("c976c52c-0e9c-4dfe-9b4-4dfe46b1a8bb;1.0");
		BOOST_CHECK_THROW(tokenizer.nextAs<UUID>(), std::runtime_error);
	}
}


// ###########
// # VisitingParser & AggregatingParser test on the shipped testFiles.
// # SensorEvent parser (switch-case)
// ######################
#define PARSER_ON_TESTFILE_HEADER(filePath, fileEventCount) \
	std::fstream recordFileV(filePath); \
	BOOST_CHECK(recordFileV.is_open()); \
	std::fstream recordFileA(filePath); \
	BOOST_CHECK(recordFileA.is_open()); \
	VisitingParser visitParser(recordFileV); \
	AggregatingParser aggregatingParser(recordFileA); \
	auto aggregatedResult = aggregatingParser.parse(); \
	BOOST_CHECK_EQUAL(aggregatedResult.size(), fileEventCount); \
	auto aggregatedResultIter = aggregatedResult.begin(); \
	RawSensorEvent rawSensorEvent

#define PARSER_ON_TESTFILE_TEST_NEXT_EVENT(_SensorEventStructType, _timestamp, _eventId, _parameterStr) \
	BOOST_CHECK(visitParser.nextLine(rawSensorEvent)); \
	BOOST_CHECK_EQUAL(rawSensorEvent.timestamp, _timestamp); \
	BOOST_CHECK_EQUAL(rawSensorEvent.eventId, _eventId); \
	BOOST_CHECK_EQUAL(rawSensorEvent.parameterString, _parameterStr); \
	{ \
		SensorEvent parsedEvent = *aggregatedResultIter++; \
		BOOST_CHECK_EQUAL(parsedEvent.timestamp, _timestamp); \
		BOOST_CHECK_NO_THROW(std::get<_SensorEventStructType>(parsedEvent.data)); \
	}

BOOST_AUTO_TEST_CASE ( parsersOnTestFiles ) {
	{
		PARSER_ON_TESTFILE_HEADER("testFiles/radioData.csv", 13);
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(FileMetadataEvent,		0, -2, "Wed Apr 08 17:26:38 GMT+02:00 2020;Markus;Synthetic Radio Advertisements");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(BLEEvent,				500000000, 9, "95F6D34CDE0A;-88;-2147483648");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(BLEEvent,				500000001, 9, "78B63370F02E;-50;-2147483648");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(BLEEvent,				500000002, 9, "DEADBEEF1337;-94;-2147483648");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GroundTruthEvent,		600000000, 99, "0");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GPSEvent,				800000000, 16, "49.77766206;9.96340305;302.0;23.799999237060547");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(WifiEvent,				1000000000, 8, "189dced9412c;2400;-50;4b95e95bd201;2350;-45;470da82627b0;2200;-84;a5d5e23f91c3;2325;-91");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(BLEEvent,				1500000000, 9, "DEADBEEF1337;-56;-2147483648");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GroundTruthEvent,		1600000000, 99, "1");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(WifiEvent,				2000000000, 8, "a5d5e23f91c3;2325;-50;4b95e95bd201;2350;-45;189dced9412c;2400;-63;470da82627b0;2200;-79");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(EddystoneUIDEvent,		3000000000, 19, "4C11AEEFE8BE;-42;1337;73696d70-6c65-4c6f-6330-4c11aeefe8bc");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(DecawaveUWBEvent,		4000000000, 20, "67307;37195;566;0;33303;4891;100;3243;8488;100;54943;12103;100;22708;2323;100");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(WifiRTTEvent,			5000000000, 17, "1;d0c637bc778a;6514;603;-55;8;7");
	}
	{
		PARSER_ON_TESTFILE_HEADER("testFiles/sensorData.csv", 16699);
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(FileMetadataEvent,		0, -2, "Wed Apr 08 17:26:38 GMT+02:00 2020;;");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(PedestrianActivityEvent,	0, 50, "STANDING;1");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(AccelerometerEvent,		21221425, 0, "1.4317327;5.0996494;7.9870567");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GyroscopeEvent,			21221425, 3, "-0.25673655;-0.16725162;-0.3504827");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(AccelerometerEvent,		26245425, 0, "1.3551182;5.2480903;7.9822683");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GyroscopeEvent,			26245425, 3, "-0.24608359;-0.1725781;-0.37178862");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GravityEvent,			26245425, 1, "0.14096293;4.5850363;8.667274");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(LinearAccelerationEvent,	26245425, 2, "1.2114661;0.6607997;-0.6847417");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(MagneticFieldEvent,		26245425, 4, "-2.5;-42.125;-24.625");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(OrientationEvent,		26245425, 6, "-3.0661385;-0.5751048;-0.16816276");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(RotationMatrixEvent,		26245425, 7, "-0.9762262;-0.06325613;0.20731898;0.0;0.16509719;-0.83674777;0.5221075;0.0;0.14044718");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(OrientationOldEvent,		26245425, 11, "273.54688;-32.9375;8.03125");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(HeadingChangeEvent,		26245425, 22, "1.5707963");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(PressureEvent,			27285425, 5, "978.63293");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(AccelerometerEvent,		31253425, 0, "1.1971009;5.3438582;7.9966335");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GyroscopeEvent,			31253425, 3, "-0.2418224;-0.16299044;-0.419727");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(RotationVectorEvent,		31253425, 12, "0.07892086;0.28449354;0.9541612;0.048524432");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(AccelerometerEvent,		36357425, 0, "1.0726024;5.3821654;8.087613");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GyroscopeEvent,			36357425, 3, "-0.23436533;-0.13209683;-0.45807767");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(RotationVectorEvent,		36357425, 12, "0.068086796;0.28519547;0.9553209;0.036988433");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GravityEvent,			36357425, 1, "1.0690438;5.393104;8.120183");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(LinearAccelerationEvent,	36357425, 2, "0.0;-0.009576807;-0.02873042");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(AccelerometerEvent,		41381425, 0, "0.94810385;5.377377;8.078036");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GyroscopeEvent,			41381425, 3, "-0.22264706;-0.08628905;-0.47512242");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(RotationVectorEvent,		41381425, 12, "0.058320872;0.28644672;0.95593125;0.026642658");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GameRotationVectorEvent,	41381425, 18, "0.23966186;-0.16739403;-0.40433973");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(StepDetectorEvent,		44737425, 21, "3737425;44737425;1.0");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(AccelerometerEvent,		46421425, 0, "0.8571242;5.3438582;8.169016");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GyroscopeEvent,			46421425, 3, "-0.2119941;-0.03728539;-0.47725302");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(RotationVectorEvent,		46421425, 12, "0.05035554;0.2834559;0.9574572;0.017853327");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GameRotationVectorEvent,	46421425, 18, "0.23938718;-0.16696677;-0.40549943");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GravityEvent,			46421425, 1, "0.8463761;5.3410287;8.180638");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(LinearAccelerationEvent,	46421425, 2, "0.009576807;0.0;-0.009576807");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(MagneticFieldEvent,		46421425, 4, "-4.125;-41.25;-24.125");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(OrientationEvent,		46421425, 6, "3.0671673;-0.5767902;-0.104541294");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(RotationMatrixEvent,		46421425, 7, "-0.99601877;0.0623271;0.06373406;0.0;-0.017201945;-0.835897;0.5486168;0.0;0.0874688");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(OrientationOldEvent,		46421425, 11, "225.57812;-33.0625;4.953125");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(AccelerometerEvent,		51493425, 0, "0.7182605;5.281609;8.197746");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GyroscopeEvent,			51493425, 3, "-0.19814523;0.012783563;-0.47086126");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(RotationVectorEvent,		51493425, 12, "-0.028473768;-0.28235725;-0.9583728;0.031006806");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GameRotationVectorEvent,	51493425, 18, "0.23914304;-0.16644795;-0.4065981");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(AccelerometerEvent,		56533425, 0, "0.5650316;5.2385135;8.307879");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GyroscopeEvent,			56533425, 3, "-0.18110047;0.053264845;-0.45807767");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(RotationVectorEvent,		56533425, 12, "-0.016571552;-0.27790156;-0.9594714;0.043488875");
		PARSER_ON_TESTFILE_TEST_NEXT_EVENT(GameRotationVectorEvent,	56533425, 18, "0.23902097;-0.1658681;-0.40763572");
	}
}



// ###########
// # ModelParsing
// ######################
#define FLOAT_COMPARE(a, b) (std::abs(a - b) < 0.00000001)

template<const size_t EXCPECTED_ARG_CNT, const size_t PROVIDED_ARG_CNT>
static void testNumericSensorEventParserSingle(const std::string& parameterString, std::array<float, PROVIDED_ARG_CNT> parameters) {
	struct TestEvent : public NumericSensorEventBase<EXCPECTED_ARG_CNT> {
		float p[EXCPECTED_ARG_CNT] = {0};
	};
	TestEvent evt;

	if constexpr(EXCPECTED_ARG_CNT != PROVIDED_ARG_CNT) {
		// there is a discrepancy between the amount of expected parameters
		// and the amount of provided parameters (in the parameterString).
		// Thus, the parser should throw an exception.
		BOOST_CHECK_THROW(evt.parse(parameterString), std::runtime_error);
	} else {
		BOOST_CHECK_NO_THROW(evt.parse(parameterString));
		for(size_t i = 0; i < EXCPECTED_ARG_CNT; ++i) {
			BOOST_CHECK(FLOAT_COMPARE(evt.p[i], parameters[i]));
		}
	}
}

template<const size_t ARG_CNT>
static void testNumericSensorEventParser() {
	testNumericSensorEventParserSingle<ARG_CNT, 3>("0.062249243;4.6926355;8.781932", {0.062249243,4.6926355,8.781932});
	testNumericSensorEventParserSingle<ARG_CNT, 3>("-0.009587673;-0.06178722;-0.07989727", {-0.009587673,-0.06178722,-0.07989727});
	testNumericSensorEventParserSingle<ARG_CNT, 4>("0.20810571;0.110080265;0.49839777;0.8343455", {0.20810571,0.110080265,0.49839777,0.8343455});
	testNumericSensorEventParserSingle<ARG_CNT, 3>("0.095919676;0.21472824;0.90331733", {0.095919676,0.21472824,0.90331733});
	testNumericSensorEventParserSingle<ARG_CNT, 3>("0.038307227;4.620809;8.825027", {0.038307227,4.620809,8.825027});
	testNumericSensorEventParserSingle<ARG_CNT, 3>("-0.0010652969;-0.064983115;-0.07350549", {-0.0010652969,-0.064983115,-0.07350549});
	testNumericSensorEventParserSingle<ARG_CNT, 4>("0.20816675;0.11001923;0.49821466;0.8344371", {0.20816675,0.11001923,0.49821466,0.8344371});
	testNumericSensorEventParserSingle<ARG_CNT, 3>("0.09604175;0.21472824;0.9032258", {0.09604175,0.21472824,0.9032258});
	testNumericSensorEventParserSingle<ARG_CNT, 3>("0.23314251;4.482681;8.718752", {0.23314251,4.482681,8.718752});
	testNumericSensorEventParserSingle<ARG_CNT, 3>("-0.19153613;0.1340753;0.10534488", {-0.19153613,0.1340753,0.10534488});
	testNumericSensorEventParserSingle<ARG_CNT, 3>("17.875;-11.375;-41.625", {17.875,-11.375,-41.625});
	testNumericSensorEventParserSingle<ARG_CNT, 3>("-1.0997756;-0.4823472;-0.0043407218", {-1.0997756,-0.4823472,-0.0043407218});
	testNumericSensorEventParserSingle<ARG_CNT, 9>("0.45558608;-0.7894381;0.41137442;0.0;0.89018357;0.40202188;-0.21436384;0.0;0.0038454705", {0.45558608,-0.7894381,0.41137442,0.0,0.89018357,0.40202188,-0.21436384,0.0,0.0038454705});
	testNumericSensorEventParserSingle<ARG_CNT, 3>("299.21875;-27.609375;0.171875", {299.21875,-27.609375,0.171875});
	testNumericSensorEventParserSingle<ARG_CNT, 3>("0.08140286;4.6160207;8.834604", {0.08140286,4.6160207,8.834604});
	testNumericSensorEventParserSingle<ARG_CNT, 3>("0.0085223755;-0.06604841;-0.07350549", {0.0085223755,-0.06604841,-0.07350549});
	testNumericSensorEventParserSingle<ARG_CNT, 4>("0.2082583;0.109988704;0.49803156;0.8345286", {0.2082583,0.109988704,0.49803156,0.8345286});
	testNumericSensorEventParserSingle<ARG_CNT, 3>("0.096163824;0.21475875;0.9031342", {0.096163824,0.21475875,0.9031342});
	testNumericSensorEventParserSingle<ARG_CNT, 9>("0.46359527;-0.7861248;0.4087633;0.0;0.885839;0.4212176;-0.19458985;0.0;-0.019206338", {0.46359527,-0.7861248,0.4087633,0.0,0.885839,0.4212176,-0.19458985,0.0,-0.019206338});
	testNumericSensorEventParserSingle<ARG_CNT, 3>("298.95312;-26.875;-1.09375", {298.95312,-26.875,-1.09375});
	testNumericSensorEventParserSingle<ARG_CNT, 1>("77.0", {77.0});
	testNumericSensorEventParserSingle<ARG_CNT, 1>("0.0001352", {0.0001352});
	testNumericSensorEventParserSingle<ARG_CNT, 2>("0.000135235;77.0", {0.000135235,77.0});
	testNumericSensorEventParserSingle<ARG_CNT, 3>("-0.21068975;4.525041;8.935161", {-0.21068975,4.525041,8.935161});
}

BOOST_AUTO_TEST_CASE ( numericSensorEventParser ) {
	testNumericSensorEventParser<1>();
	testNumericSensorEventParser<2>();
	testNumericSensorEventParser<3>();
	testNumericSensorEventParser<9>();
}


void testFileMetadataEventParserSingle(const std::string& parameterString, const std::string& date, const std::string& person, const std::string& comment) {
	FileMetadataEvent fmEvt;
	BOOST_CHECK_NO_THROW(fmEvt.parse(parameterString));
	BOOST_CHECK_EQUAL(fmEvt.date, date);
	BOOST_CHECK_EQUAL(fmEvt.person, person);
	BOOST_CHECK_EQUAL(fmEvt.comment, comment);
}

BOOST_AUTO_TEST_CASE ( FileMetadataEventParserTest ) {
	FileMetadataEvent fmEvt;
	BOOST_CHECK_THROW(fmEvt.parse(""), std::runtime_error);
	BOOST_CHECK_THROW(fmEvt.parse(";"), std::runtime_error);
	BOOST_CHECK_THROW(fmEvt.parse(";;"), std::runtime_error);

	testFileMetadataEventParserSingle("date;;", "date", "", "");
	testFileMetadataEventParserSingle("date;person;", "date", "person", "");
	testFileMetadataEventParserSingle("date;person;comment", "date", "person", "comment");
	testFileMetadataEventParserSingle("date;person;comment;with;semicolons", "date", "person", "comment;with;semicolons");
}

BOOST_AUTO_TEST_CASE ( PedestrianActivityEventParserTest ) {
	PedestrianActivityEvent paEvt;
	BOOST_CHECK_THROW(paEvt.parse(""), std::runtime_error);

	BOOST_CHECK_NO_THROW(paEvt.parse(";1"));
	BOOST_CHECK(paEvt.activity == PedestrianActivity::Standing);
	BOOST_CHECK_NO_THROW(paEvt.parse(";5"));
	BOOST_CHECK(paEvt.activity == PedestrianActivity::ElevatorDown);
	BOOST_CHECK_NO_THROW(paEvt.parse(";6"));
	BOOST_CHECK(paEvt.activity == PedestrianActivity::MessAround);
}

BOOST_AUTO_TEST_CASE ( GroundTruthEventParserTest ) {
	GroundTruthEvent gtEvt;
	BOOST_CHECK_THROW(gtEvt.parse(""), std::runtime_error);

	BOOST_CHECK_NO_THROW(gtEvt.parse("0"));
	BOOST_CHECK_EQUAL(gtEvt.groundTruthId, 0);
	BOOST_CHECK_NO_THROW(gtEvt.parse("5"));
	BOOST_CHECK_EQUAL(gtEvt.groundTruthId, 5);
}

BOOST_AUTO_TEST_CASE ( GroundTruthPathEventParserTest ) {
	GroundTruthPathEvent gtpEvt;
	BOOST_CHECK_THROW(gtpEvt.parse(""), std::runtime_error);
	BOOST_CHECK_THROW(gtpEvt.parse(";"), std::runtime_error);
	BOOST_CHECK_THROW(gtpEvt.parse("0;"), std::runtime_error);

	BOOST_CHECK_NO_THROW(gtpEvt.parse("0;0"));
	BOOST_CHECK_EQUAL(gtpEvt.pathId, 0);
	BOOST_CHECK_EQUAL(gtpEvt.groundTruthPointCnt, 0);
	BOOST_CHECK_NO_THROW(gtpEvt.parse("1337;42"));
	BOOST_CHECK_EQUAL(gtpEvt.pathId, 1337);
	BOOST_CHECK_EQUAL(gtpEvt.groundTruthPointCnt, 42);
}


BOOST_AUTO_TEST_CASE ( CustomActivityType ) {
	std::fstream customActivityFile("testFiles/customActivity.csv");
	SensorReadoutParser::AggregatingParser parser(customActivityFile);
	auto events = parser.parse();
	{
		auto actEvt = std::get<SensorReadoutParser::PedestrianActivityEvent>(events[7].data);
		BOOST_CHECK_EQUAL(actEvt.rawActivityId, 7);
		BOOST_CHECK_EQUAL(actEvt.rawActivityName, "CUSTOM_1");
	}
	{
		auto actEvt = std::get<SensorReadoutParser::PedestrianActivityEvent>(events[17].data);
		BOOST_CHECK_EQUAL(actEvt.rawActivityId, 10);
		BOOST_CHECK_EQUAL(actEvt.rawActivityName, "CUSTOM_5");
	}
}
