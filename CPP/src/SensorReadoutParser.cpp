#include <sensorreadout/SensorReadoutParser.h>

#include <charconv>
#include <iomanip>

namespace SensorReadoutParser {

using namespace _internal;

// ###########
// # Helpers
// ######################
namespace _internal {
	#define IMPLEMENT_FROM_STRINGVIEW_NUMERIC(NumberType) \
		template<> NumberType fromStringView(const std::string_view& str) { \
			NumberType result; \
			auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result); \
			exceptAssert(ec == std::errc(), "Failed to parse token to value"); \
			return result; \
		}

	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(uint8_t);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(int8_t);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(uint16_t);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(int16_t);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(uint32_t);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(int32_t);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(uint64_t);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(int64_t);
#ifndef ANDROID
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(float);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(double);
#else // NDK26 will have from_chars, everything before... is missing from_chars<float> and from_chars<double>
	template<> float fromStringView(const std::string_view& strView) {
		float result; size_t endIdx = 0;
		std::string str{strView};
		result = std::stof(str, &endIdx);
		exceptAssert(endIdx == strView.size(), "Failed to parse token to float");
		return result;
	}
	template<> double fromStringView(const std::string_view& strView) {
		double result; size_t endIdx = 0;
		std::string str{strView};
		result = std::stod(str, &endIdx);
		exceptAssert(endIdx == strView.size(), "Failed to parse token to float");
		return result;
	}
#endif

	template<> bool fromStringView<bool>(const std::string_view& str) {
		uint8_t data = fromStringView<uint8_t>(str);
		return (data != 0);
	}
	template<> UUID fromStringView<UUID>(const std::string_view& str) {
		return UUID::fromString(str);
	}
	template<> HexString fromStringView<HexString>(const std::string_view& str) {
		exceptAssert(str.size() % 2 == 0, "Invalid HexString!");
		HexString result;
		for (unsigned int i = 0; i < str.length(); i += 2) {
			std::string byteString = std::string(str.substr(i, 2));
			char byte = std::strtol(byteString.c_str(), NULL, 16);
			result.data.push_back(byte);
		}
		return result;
	}
	template<> MacAddress fromStringView<MacAddress>(const std::string_view& str) {
		if(str.length() == MacAddress::STRING_LENGTH_SHORT) {
			return MacAddress::fromString(str);
		} else {
			return MacAddress::fromColonDelimitedString(str);
		}
	}

	std::string ParameterAssembler::str() const { return stream.str(); }
	// override for uint8_t because retarded c++ default stream outputs this
	// as character instead of as number
	template<> void ParameterAssembler::push<uint8_t>(uint8_t value) {
		push<uint32_t>(value);
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

std::ostream& operator<<(std::ostream& output, const HexString& self) {
	output << std::hex << std::uppercase << std::setfill( '0' );
	for( uint8_t b : self.data ) {
		output << std::setw( 2 ) << b;
	}
	return output;
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

uint8_t& MacAddress::operator[](size_t idx) {
	return mac[idx];
}

std::string MacAddress::toString() const {
	std::string macString;
	macString.resize(STRING_LENGTH_SHORT);
	for(size_t i = 0; i < MAC_LENGTH; ++i) {
		sprintf(&macString[i * 2], "%02X", mac[i]);
	}
	return macString;
}
std::string MacAddress::toColonDelimitedString() const {
	std::string macString;
	macString.resize(STRING_LENGTH_COLONDELIMITED);
	for(size_t i = 0; i < MAC_LENGTH; ++i) {
		sprintf(&macString[i * 3], "%02X:", mac[i]);
	}
	return macString;
}

bool MacAddress::operator==(const MacAddress& o) {
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
			// this event sometimes had a trailing `;`, so we handle this by
			// peeking and aborting early on.
			if(tokenizer.peekNext().value() == "") { break; }

			advertisement.mac = tokenizer.nextAs<MacAddress>();
			advertisement.channelFreq = tokenizer.nextAs<WifiFrequency>();
			advertisement.rssi = tokenizer.nextAs<Rssi>();

			advertisements.push_back(advertisement);
		}
	} catch (std::invalid_argument& e) {
		throw std::runtime_error(e.what());
	}
}
void WifiEvent::serializeInto(_internal::ParameterAssembler& stream) const {
	for(const auto& adv : advertisements) {
		stream.push(adv.mac.toString());
		stream.push(adv.channelFreq);
		stream.push(adv.rssi);
	}
}

void BLEEvent::parse(const std::string& parameterString) {
	Tokenizer<';'> tokenizer(parameterString);
	mac = tokenizer.nextAs<MacAddress>();
	rssi = tokenizer.nextAs<Rssi>();
	txPower = tokenizer.nextAs<BluetoothTxPower>();
	if(tokenizer.peekNext()) { // only available on newer files
		rawData = tokenizer.nextAs<HexString>().data;
	}
}
void BLEEvent::serializeInto(_internal::ParameterAssembler& stream) const {
	stream.push(mac.toString());
	stream.push(rssi);
	stream.push(txPower);
	if(rawData.size() > 0) {
		stream.push(HexString { rawData } );
	}
}

void WifiRTTEvent::parse(const std::string& parameterString) {
	Tokenizer<';'> tokenizer(parameterString);
	success = tokenizer.nextAs<bool>();
	mac = tokenizer.nextAs<MacAddress>();
	distanceMM = tokenizer.nextAs<int64_t>();
	distanceStdDevMM = tokenizer.nextAs<int64_t>();
	rssi = tokenizer.nextAs<Rssi>();
	numAttempted = tokenizer.nextAs<size_t>();
	numSuccessfull = tokenizer.nextAs<size_t>();
}
void WifiRTTEvent::serializeInto(_internal::ParameterAssembler& stream) const {
	stream.push(success);
	stream.push(mac.toString());
	stream.push(distanceMM);
	stream.push(distanceStdDevMM);
	stream.push(rssi);
	stream.push(numAttempted);
	stream.push(numSuccessfull);
}

void EddystoneUIDEvent::parse(const std::string& parameterString) {
	exceptAssert(parameterString.size() > 32, "EddystoneUID event does not seem to have all required fields");
	Tokenizer<';'> tokenizer(parameterString);
	mac = tokenizer.nextAs<MacAddress>();
	rssi = tokenizer.nextAs<Rssi>();
	txPower = tokenizer.nextAs<BluetoothTxPower>();
	uid = tokenizer.nextAs<UUID>();
}
void EddystoneUIDEvent::serializeInto(_internal::ParameterAssembler& stream) const {
	stream.push(mac.toString());
	stream.push(rssi);
	stream.push(txPower);
	stream.push(uid.toString());
}

void StepDetectorEvent::parse(const std::string& parameterString) {
	Tokenizer<';'> tokenizer(parameterString);
	stepStartTs = tokenizer.nextAs<Timestamp>();
	stepEndTs = tokenizer.nextAs<Timestamp>();
	probability = tokenizer.nextAs<float>();
}
void StepDetectorEvent::serializeInto(_internal::ParameterAssembler& stream) const {
	stream.push(stepStartTs);
	stream.push(stepEndTs);
	stream.push(probability);
}

void FutureShapeSensFloorEvent::parse(const std::string& parameterString) {
	Tokenizer<';'> tokenizer(parameterString);
	roomId = tokenizer.nextAs<uint16_t>();
	x = tokenizer.nextAs<uint8_t>();
	y = tokenizer.nextAs<uint8_t>();
	for(size_t i = 0; i < fieldCapacities.size(); ++i) {
		fieldCapacities[i] = tokenizer.nextAs<uint8_t>();
	}
}
void FutureShapeSensFloorEvent::serializeInto(_internal::ParameterAssembler& stream) const {
	stream.push(roomId);
	stream.push((int)x);
	stream.push((int)y);
	for(size_t i = 0; i < this->fieldCapacities.size(); ++i) {
		stream.push((int)fieldCapacities[i]);
	}
}

void MicrophoneMetadataEvent::parse(const std::string& parameterString) {
	Tokenizer<';'> tokenizer(parameterString);
	channelCnt = tokenizer.nextAs<size_t>();
	sampleRateHz = tokenizer.nextAs<size_t>();
	sampleFormat = tokenizer.next();
}
void MicrophoneMetadataEvent::serializeInto(_internal::ParameterAssembler& stream) const {
	stream.push(channelCnt);
	stream.push(sampleRateHz);
	stream.push(sampleFormat);
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
void DecawaveUWBEvent::serializeInto(_internal::ParameterAssembler& stream) const {
	stream.push(x * 1000.0); // m to mm
	stream.push(y * 1000.0); // m to mm
	stream.push(z * 1000.0); // m to mm
	stream.push(qualityFactor);
	for(const auto& anchorMeasurement : anchorMeasurements) {
		stream.push(anchorMeasurement.nodeId);
		stream.push(anchorMeasurement.distance * 1000.0); // m to mm
		stream.push(anchorMeasurement.qualityFactor);
	}
}

void PedestrianActivityEvent::parse(const std::string& parameterString) {
	Tokenizer<';'> tokenizer(parameterString);
	rawActivityName = tokenizer.next();
	rawActivityId = tokenizer.nextAs<PedestrianActivityId>();
	activity = static_cast<PedestrianActivity>(rawActivityId);
}
void PedestrianActivityEvent::serializeInto(_internal::ParameterAssembler& stream) const {
	stream.push(rawActivityName);
	stream.push(rawActivityId);
}

void FileMetadataEvent::parse(const std::string& parameterString) {
	Tokenizer<';'> tokenizer(parameterString);
	date = tokenizer.next();
	person = tokenizer.next();
	comment = tokenizer.remainder();
	exceptAssert(date.length() > 0, "FileMetadata date is empty.");
}
void FileMetadataEvent::serializeInto(_internal::ParameterAssembler& stream) const {
	stream.push(date);
	stream.push(person);
	stream.push(comment);
}

void RecordingIdEvent::parse(const std::string& parameterString) {
	recordingId = UUID::fromString(parameterString);
}
void RecordingIdEvent::serializeInto(_internal::ParameterAssembler& stream) const {
	stream.push(recordingId.toString());
}

SensorEvent SensorEvent::parse(const RawSensorEvent& rawEvent) {
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
		SENSOR_EVENT_PARSE_CASE(EventType::FutureShapeSensFloor, FutureShapeSensFloorEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::MicrophoneMetadata, MicrophoneMetadataEvent)
		// Special events
		SENSOR_EVENT_PARSE_CASE(EventType::PedestrianActivity, PedestrianActivityEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::GroundTruth, GroundTruthEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::GroundTruthPath, GroundTruthPathEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::FileMetadata, FileMetadataEvent)
		SENSOR_EVENT_PARSE_CASE(EventType::RecordingId, RecordingIdEvent)
		default: {
			throw std::runtime_error("Attempted to parse unknown event type.");
		}
	}
	return result;
}

void SensorEvent::serializeInto(RawSensorEvent& rawEvent) const {
	#define SENSOR_EVENT_SERIALIZE_INTO_CASE(EvtType, EvtStruct) \
		case EvtType: { \
			std::get<EvtStruct>(data).serializeInto(parameterStream); \
			break; \
		}

	rawEvent.eventId = static_cast<EventId>(eventType);
	rawEvent.timestamp = timestamp;
	_internal::ParameterAssembler parameterStream;
	switch(eventType) {
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::Accelerometer, AccelerometerEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::Gravity, GravityEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::LinearAcceleration, LinearAccelerationEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::Gyroscope, GyroscopeEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::MagneticField, MagneticFieldEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::Pressure, PressureEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::Orientation, OrientationEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::RotationMatrix, RotationMatrixEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::Wifi, WifiEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::BLE, BLEEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::RelativeHumidity, RelativeHumidityEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::OrientationOld, OrientationOldEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::RotationVector, RotationVectorEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::Light, LightEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::AmbientTemperature, AmbientTemperatureEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::HeartRate, HeartRateEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::GPS, GPSEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::WifiRTT, WifiRTTEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::GameRotationVector, GameRotationVectorEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::EddystoneUID, EddystoneUIDEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::DecawaveUWB, DecawaveUWBEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::StepDetector, StepDetectorEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::HeadingChange, HeadingChangeEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::FutureShapeSensFloor, FutureShapeSensFloorEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::MicrophoneMetadata, MicrophoneMetadataEvent)
		// Special events
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::PedestrianActivity, PedestrianActivityEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::GroundTruth, GroundTruthEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::GroundTruthPath, GroundTruthPathEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::FileMetadata, FileMetadataEvent)
		SENSOR_EVENT_SERIALIZE_INTO_CASE(EventType::RecordingId, RecordingIdEvent)
	}
	rawEvent.parameterString = parameterStream.str();
}



// ###########
// # VisitingParser
// ######################

VisitingParser::VisitingParser(std::istream& stream, FileVersion fileVersion) : stream(stream), fileVersion(fileVersion) {}

bool VisitingParser::nextLine(RawSensorEvent& sensorEvent) {
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
	if(fileVersion == FileVersion::V0) { // transform timestamp from ms to ns
		sensorEvent.timestamp *= 1000000;
	}
	std::string::size_type dIdx2 = line.find(';', dIdx + 1);
	exceptWhen( // Second section empty - no event id
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

AggregatingParser::AggregatingParser(std::istream& stream, FileVersion fileVersion) : parser(stream, fileVersion) {}

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

Serializer::Serializer(std::ostream& stream, FileVersion fileVersion) : stream(stream), fileVersion(fileVersion) {}

Serializer::~Serializer() {
	flush();
}

void Serializer::write(const RawSensorEvent& sensorEvent) {
	auto timestamp = (fileVersion == FileVersion::V0) ? (sensorEvent.timestamp / 1000000) : sensorEvent.timestamp;
	stream << timestamp << ';';
	stream << sensorEvent.eventId << ';';
	stream << sensorEvent.parameterString << '\n';
	exceptAssert(stream.good(), "I/O error");
}

void Serializer::write(const SensorEvent& sensorEvent) {
	RawSensorEvent rawEvt;
	sensorEvent.serializeInto(rawEvt);
	write(rawEvt);
}

void Serializer::flush() {
	exceptAssert(stream.good(), "I/O error");
	stream.flush();
}

}
