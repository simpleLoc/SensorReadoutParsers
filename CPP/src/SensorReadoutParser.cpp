#include <sensorreadout/SensorReadoutParser.h>

#include <charconv>

namespace SensorReadoutParser {

using namespace _internal;

// ###########
// # Helpers
// ######################
namespace _internal {
	#define IMPLEMENT_FROM_STRINGVIEW_NUMERIC(numberType, sscanfParameter) \
		template<> numberType fromStringView<numberType>(const std::string_view& str) { return fromStringView<numberType>(str, "%" sscanfParameter); }

	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(uint8_t, SCNu8);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(int8_t, SCNd8);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(uint16_t, SCNu16);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(int16_t, SCNd16);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(uint32_t, SCNu32);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(int32_t, SCNd32);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(uint64_t, SCNu64);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(int64_t, SCNd64);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(float, "f");
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(double, "lf");

	template<> UUID fromStringView<UUID>(const std::string_view& str) {
		return UUID::fromString(str);
	}
	template<> MacAddress fromStringView<MacAddress>(const std::string_view& str) {
		if(str.length() == MacAddress::STRING_LENGTH_SHORT) {
			return MacAddress::fromString(str);
		} else {
			return MacAddress::fromColonDelimitedString(str);
		}
	}
}


// ###########
// # BaseTypes
// ######################
static uint8_t parseHexNibble(char hex) {
	if(hex >= '0' && hex <= '9') {
		return hex - '0';
	} else if(hex >= 'A' && hex <= 'F') {
		return hex - 'A' + 10;
	} else if(hex >= 'a' || hex <= 'f') {
		return hex - 'a' + 10;
	}
	exceptUnreachable("Invalid hex encountered");
}

UUID UUID::fromString(const std::string_view& uuidStr) {
	exceptAssert(uuidStr.size() == STRING_LENGTH, "Attempted to parse invalid UUID string");
	UUID result;
	for(size_t bPtr = 0, cPtr = 0; cPtr < STRING_LENGTH;) {
		if(uuidStr[cPtr] != '-') {
			result.data[bPtr] = (parseHexNibble(uuidStr[cPtr]) << 4) | parseHexNibble(uuidStr[cPtr + 1]);
			cPtr += 2;
			bPtr += 1;
		} else {
			cPtr += 1;
		}
	}
	return result;
}

std::string UUID::toString() const {
	static const char* charMap = "0123456789abcdef";
	std::string result(STRING_LENGTH, '-');
#define PRINT_UUID_BYTE(i,o) result[(i)*2 + o] = charMap[data[(i)] >> 4]; result[(i)*2 + 1 + o] = charMap[data[(i)] & 0xF]
	for(auto i = 0; i < 4; ++i) { PRINT_UUID_BYTE(i, 0); }
	PRINT_UUID_BYTE(4, 1); PRINT_UUID_BYTE(5, 1);
	PRINT_UUID_BYTE(6, 2); PRINT_UUID_BYTE(7, 2);
	PRINT_UUID_BYTE(8, 3); PRINT_UUID_BYTE(9, 3);
	for(auto i = 0; i < 6; ++i) { PRINT_UUID_BYTE(10 + i, 4); }
	return result;
}

MacAddress MacAddress::fromString(const std::string_view& macStr) {
	exceptAssert(macStr.size() == STRING_LENGTH_SHORT, "Undelimited MAC-Address string has to have correct length");
	MacAddress res;
	exceptAssert(
				sscanf(macStr.data(), "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx", &res[0], &res[1], &res[2], &res[3], &res[4], &res[5]) == 6,
			"Parsing undelimited MAC-Address failed"
			);
	return res;
}

MacAddress MacAddress::fromColonDelimitedString(const std::string_view& delimitedMacStr) {
	exceptAssert(delimitedMacStr.size() == STRING_LENGTH_COLONDELIMITED, "Delimited MAC-Address string has to have correct length");
	MacAddress res;
	exceptAssert(
				sscanf(delimitedMacStr.data(), "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", &res[0], &res[1], &res[2], &res[3], &res[4], &res[5]) == 6,
			"Parsing colon delimited MAC-Address failed"
			);
	return res;
}

uint8_t MacAddress::operator[](size_t idx) const {
	return mac[idx];
}

uint8_t &MacAddress::operator[](size_t idx) {
	return mac[idx];
}

std::string MacAddress::toString() const {
	std::string macString;
	macString.resize(STRING_LENGTH_COLONDELIMITED);
	for(size_t i = 0; i < MAC_LENGTH; ++i) {
		sprintf(&macString[i * 3], "%02X:", mac[i]);
	}
	return macString;
}

bool MacAddress::operator==(const MacAddress &o) {
	for(size_t i = 0; i < MAC_LENGTH; ++i) {
		if(o.mac[i] != mac[i]) { return false; }
	}
	return true;
}

// ###########
// # ParsedModels
// ######################

void WifiEvent::parse(const std::string& parameterString) {
	Tokenizer<';'> tokenizer(parameterString);
	WifiAdvertisement advertisement;

	try {
		while(!tokenizer.isEOS()) {
			advertisement.mac = tokenizer.nextAs<MacAddress>();
			advertisement.channelFreq = tokenizer.nextAs<WifiFrequency>();
			advertisement.rssi = tokenizer.nextAs<Rssi>();

			advertisements.push_back(advertisement);
		}
	} catch (std::invalid_argument& e) {
		throw std::runtime_error(e.what());
	}
}

void BLEEvent::parse(const std::string& parameterString) {
	Tokenizer<';'> tokenizer(parameterString);
	mac = tokenizer.nextAs<MacAddress>();
	rssi = tokenizer.nextAs<Rssi>();
	txPower = tokenizer.nextAs<BluetoothTxPower>();
}

void GPSEvent::parse(const std::string& parameterString) {
	exceptWhen(true, "Not implemented yet.");
	//TODO: IMPLEMENT
}

void WifiRTTEvent::parse(const std::string& parameterString) {
	exceptWhen(true, "Not implemented yet.");
	//TODO: IMPLEMENT
}

void EddystoneUIDEvent::parse(const std::string& parameterString) {
	exceptAssert(parameterString.size() > 32, "EddystoneUID event does not seem to have all required fields");
	Tokenizer<';'> tokenizer(parameterString);
	mac = tokenizer.nextAs<MacAddress>();
	rssi = tokenizer.nextAs<Rssi>();
	txPower = tokenizer.nextAs<BluetoothTxPower>();
	uid = tokenizer.nextAs<UUID>();
}

void DecawaveUWBEvent::parse(const std::string& parameterString) {
	Tokenizer<';'> tokenizer(parameterString);
	// packet header (estimated position + quality)
	x = tokenizer.nextAs<float>() / 1000.0; // mm to m
	y = tokenizer.nextAs<float>() / 1000.0; // mm to m
	z = tokenizer.nextAs<float>() / 1000.0; // mm to m
	qualityFactor = tokenizer.nextAs<uint8_t>();
	// single measurements from the paired tag to each anchor in the UWB network
	try {
		while(!tokenizer.isEOS()) {
			DecawaveUWBMeasurement anchorMeasurement;
			anchorMeasurement.nodeId = tokenizer.nextAs<uint16_t>();
			anchorMeasurement.distance = tokenizer.nextAs<float>() / 1000.0; // mm to m
			anchorMeasurement.qualityFactor = tokenizer.nextAs<uint8_t>();

			anchorMeasurements.push_back(anchorMeasurement);
		}
	} catch (std::invalid_argument& e) {
		throw std::runtime_error(e.what());
	}
}

void PedestrianActivityEvent::parse(const std::string& parameterString) {
	Tokenizer<';'> tokenizer(parameterString);
	tokenizer.skipNext(); // skip activity name
	activity = static_cast<PedestrianActivity>(tokenizer.nextAs<PedestrianActivityId>());
}

void GroundTruthEvent::parse(const std::string& parameterString) {
	try {
		groundTruthId = std::stoul(parameterString);
	} catch(...) {
		exceptWhen(true, "Invalid ground truth id.");
	}
}

void GroundTruthPathEvent::parse(const std::string& parameterString) {
	try {
		Tokenizer<';'> tokenizer(parameterString);
		pathId = tokenizer.nextAs<size_t>();
		groundTruthPointCnt = tokenizer.nextAs<size_t>();
	} catch(...) {
		exceptWhen(true, "Invalid ground truth id.");
	}
}

void FileMetadataEvent::parse(const std::string& parameterString) {
	Tokenizer<';'> tokenizer(parameterString);
	date = tokenizer.next();
	person = tokenizer.next();
	comment = tokenizer.remainder();
	exceptAssert(date.length() > 0, "FileMetadata date is empty.");
}

SensorEvent SensorEvent::parse(const RawSensorEvent &rawEvent) {
#define SENSOR_EVENT_PARSE_CASE(EvtType, EvtStruct) \
	case EvtType: { \
	EvtStruct evt; \
	evt.parse(rawEvent.parameterString); \
	result.data = evt; \
	break; \
}

	SensorEvent result;
	result.timestamp = rawEvent.timestamp;
	result.eventType = static_cast<EventType>(rawEvent.eventId);
	switch(result.eventType) {
		SENSOR_EVENT_PARSE_CASE(EventType::Accelerometer, AccelerometerEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::Gravity, GravityEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::LinearAcceleration, LinearAccelerationEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::Gyroscope, GyroscopeEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::MagneticField, MagneticFieldEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::Pressure, PressureEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::Orientation, OrientationEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::RotationMatrix, RotationMatrixEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::Wifi, WifiEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::BLE, BLEEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::RelativeHumidity, RelativeHumidityEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::OrientationOld, OrientationOldEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::RotationVector, RotationVectorEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::Light, LightEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::AmbientTemperature, AmbientTemperatureEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::HeartRate, HeartRateEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::GPS, GPSEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::WifiRTT, WifiRTTEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::GameRotationVector, GameRotationVectorEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::EddystoneUID, EddystoneUIDEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::DecawaveUWB, DecawaveUWBEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::StepDetector, StepDetectorEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::HeadingChange, HeadingChangeEvent)
		// Special events
		SENSOR_EVENT_PARSE_CASE(EventType::PedestrianActivity, PedestrianActivityEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::GroundTruth, GroundTruthEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::GroundTruthPath, GroundTruthPathEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::FileMetadata, FileMetadataEvent)
		default: {
			throw std::runtime_error("Attempted to parse unknown event type.");
		}
	}
	return result;
}



// ###########
// # VisitingParser
// ######################

VisitingParser::VisitingParser(std::istream &stream) : stream(stream) {}

bool VisitingParser::nextLine(RawSensorEvent &sensorEvent) {
	if(!stream.good()) {
		if(stream.fail()) { throw std::runtime_error("An error occured while reading the SensorReadout file."); }
		return false;
	}
	std::string line;
	if(!std::getline(stream, line)) { return false; }
	std::string::size_type dIdx = line.find(';', 0);
	exceptWhen( // First section empty - no timestamp
		(dIdx == std::string::npos || dIdx < 1),
	   "SensorReadout file corrupted. Empty timestamp section.");
	exceptAssert(
		std::from_chars(line.data(), line.data() + dIdx, sensorEvent.timestamp).ec == std::errc(),
		 "Timestamp parsing error");
	std::string::size_type dIdx2 = line.find(';', dIdx + 1);
	exceptWhen( // First section empty - no event id
		(dIdx2 == std::string::npos || (dIdx2 - dIdx) < 2),
		"SensorReadout file corrupted. Empty eventId section.");
	exceptAssert(
		std::from_chars(line.data() + dIdx + 1, line.data() + dIdx2, sensorEvent.eventId).ec == std::errc(),
		"EventId parsing error");
	sensorEvent.parameterString = line.substr(dIdx2 + 1);
	return true;
}



// ###########
// # AggregatingParser
// ######################

AggregatingParser::AggregatingParser(std::istream &stream) : parser(stream) {}

AggregatingParser::AggregatedParseResult AggregatingParser::parse() {
	AggregatedParseResult result;
	RawSensorEvent rawSensorEvent;
	while(parser.nextLine(rawSensorEvent)) {
		result.push_back(SensorEvent::parse(rawSensorEvent));
	}
	return result;
}
AggregatingParser::AggregatedRawParseResult AggregatingParser::parseRaw() {
	AggregatedRawParseResult result;
	RawSensorEvent rawSensorEvent;
	while(parser.nextLine(rawSensorEvent)) {
		result.push_back(rawSensorEvent);
	}
	return result;
}



// ###########
// # Serializer
// ######################

Serializer::Serializer(std::ostream &stream) : stream(stream) {}

void Serializer::write(const RawSensorEvent &sensorEvent) {
	stream << sensorEvent.timestamp << ";";
	stream << sensorEvent.eventId << ";";
	stream << sensorEvent.parameterString << std::endl;
	exceptAssert(stream.good(), "I/O error");
}

}
