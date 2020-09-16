#pragma once

#include <cinttypes>
#include <string>
#include <string_view>
#include <cstring>
#include <sstream>
#include <istream>
#include <vector>
#include <variant>

namespace SensorReadoutParser {

// ###########
// # Helpers
// ######################

namespace _internal {
	#define exceptAssert(cond, exceptionStr) if(!(cond)) { throw std::runtime_error(exceptionStr); }
	#define exceptWhen(cond, exceptionStr) if((cond)) { throw std::runtime_error(exceptionStr); }

	template<typename TValue>
	TValue fromStringView(const std::string_view& str, const char* format) {
		TValue result;
		exceptAssert(sscanf(str.data(), format, &result) == 1, "Failed to parse token to value");
		return result;
	}
	template<typename TValue> TValue fromStringView(const std::string_view& str) { return TValue::unimplemented_function(); }

	template<const char SEPERATOR>
	class Tokenizer {
	private:
		std::string_view str;
		size_t ptr = 0;

	private:
		template<typename TValue>
		TValue nextAs(const char* format) {
			std::string_view token = next();
			TValue result;
			exceptAssert(sscanf(token.data(), format, &result) == 1, "Failed to parse token to value");
			return result;
		}

	public:
		Tokenizer(const std::string& str) : str(str) {}

		std::string_view next() {
			std::string_view result;
			exceptAssert(!isEOS(), "Unexpected EOS");
			auto nextSepPtr = str.find(SEPERATOR, ptr);
			if(nextSepPtr == std::string::npos) { // reached EOS, no further tokens
				nextSepPtr = str.length();
			}
			result = str.substr(ptr, (nextSepPtr - ptr));
			ptr = nextSepPtr + 1;
			return result;
		}

		template<typename TValue> TValue nextAs() {
			return fromStringView<TValue>(next());
		}

		void skipNext() {
			exceptAssert(ptr <= str.length(), "Unexpected EOS");
			auto nextSepPtr = str.find(SEPERATOR, ptr);
			if(nextSepPtr == std::string::npos) { // reached EOS, no further tokens
				nextSepPtr = str.length();
			}
			ptr = nextSepPtr + 1;
		}

		std::string_view remainder() {
			std::string_view result = str.substr(ptr);
			ptr = str.length() + 1;
			return result;
		}

		bool isEOS() const {
			return ptr > str.length();
		}
	};

}

// ###########
// # BaseTypes
// ######################

using Timestamp = uint64_t;
using EventId = int32_t;
using PedestrianActivityId = uint32_t;
///
/// \brief Primitive type used to represent the signal strength (RSSI) of an advertisement.
///
using Rssi = int32_t;
///
/// \brief Primitive type used to represent the tx power a bluetooth advertisement was sent with.
///
using BluetoothTxPower = int32_t;
///
/// \brief Primitive type used to represent the frequency channel a wifi advertisement was sent on.
///
using WifiFrequency = uint32_t;

struct RawSensorEvent {
	Timestamp timestamp = 0;
	EventId eventId = 0;
	std::string parameterString;
};

struct MacAddress {
public: // Associated Types & Constants
	static constexpr size_t MAC_LENGTH = 6;
	static constexpr size_t STRING_LENGTH_SHORT = 12;
	static constexpr size_t STRING_LENGTH_COLONDELIMITED = 17;

private:
	uint8_t mac[MAC_LENGTH] = {0};

public:
	static MacAddress fromString(const std::string_view& macStr);
	static MacAddress fromColonDelimitedString(const std::string_view& delimitedMacStr);

	uint8_t operator[](size_t idx) const;
	uint8_t& operator[](size_t idx);

	std::string toString() const;

	bool operator==(const MacAddress& o);
};

struct WifiAdvertisement {
	MacAddress mac;
	Rssi rssi;
	WifiFrequency channelFreq;
};


// ###########
// # EventTypes
// ######################


static constexpr EventId EVENTID_ACCELEROMETER = 0;
static constexpr EventId EVENTID_GRAVITY = 1;
static constexpr EventId EVENTID_LINEAR_ACCELERATION = 2;
static constexpr EventId EVENTID_GYROSCOPE = 3;
static constexpr EventId EVENTID_MAGNETIC_FIELD = 4;
static constexpr EventId EVENTID_PRESSURE = 5;
static constexpr EventId EVENTID_ORIENTATION_NEW = 6;
static constexpr EventId EVENTID_ROTATION_MATRIX = 7;
static constexpr EventId EVENTID_WIFI = 8;
static constexpr EventId EVENTID_IBEACON = 9;
static constexpr EventId EVENTID_RELATIVE_HUMIDITY = 10;
static constexpr EventId EVENTID_ORIENTATION_OLD = 11;
static constexpr EventId EVENTID_ROTATION_VECTOR = 12;
static constexpr EventId EVENTID_LIGHT = 13;
static constexpr EventId EVENTID_AMBIENT_TEMPERATURE = 14;
static constexpr EventId EVENTID_HEART_RATE = 15;
static constexpr EventId EVENTID_GPS = 16;
static constexpr EventId EVENTID_WIFIRTT = 17;
static constexpr EventId EVENTID_GAME_ROTATION_VECTOR = 18;
// ------
static constexpr EventId EVENTID_PEDESTRIAN_ACTIVITY = 50;
static constexpr EventId EVENTID_GROUND_TRUTH = 99;
static constexpr EventId EVENTID_GROUND_TRUTH_PATH = -1;
static constexpr EventId EVENTID_FILE_METADATA = -2;

enum class EventType : EventId {
	// Sensor events
	Accelerometer = EVENTID_ACCELEROMETER,
	Gravity = EVENTID_GRAVITY,
	LinearAcceleration = EVENTID_LINEAR_ACCELERATION,
	Gyroscope = EVENTID_GYROSCOPE,
	MagneticField = EVENTID_MAGNETIC_FIELD,
	Pressure = EVENTID_PRESSURE,
	Orientation = EVENTID_ORIENTATION_NEW,
	RotationMatrix = EVENTID_ROTATION_MATRIX,
	Wifi = EVENTID_WIFI,
	BLE = EVENTID_IBEACON,
	RelativeHumidity = EVENTID_RELATIVE_HUMIDITY,
	OrientationOld = EVENTID_ORIENTATION_OLD,
	RotationVector = EVENTID_ROTATION_VECTOR,
	Light = EVENTID_LIGHT,
	AmbientTemperature = EVENTID_AMBIENT_TEMPERATURE,
	HeartRate = EVENTID_HEART_RATE,
	GPS = EVENTID_GPS,
	WifiRTT = EVENTID_WIFIRTT,
	GameRotationVector = EVENTID_GAME_ROTATION_VECTOR,
	// Special events
	PedestrianActivity = EVENTID_PEDESTRIAN_ACTIVITY,
	GroundTruth = EVENTID_GROUND_TRUTH,
	GroundTruthPath = EVENTID_GROUND_TRUTH_PATH,
	FileMetadata = EVENTID_FILE_METADATA
};

enum class PedestrianActivity : PedestrianActivityId {
	Walking = 0,
	Standing = 1,
	StairsUp = 2,
	StairsDown = 3,
	ElevatorUp = 4,
	ElevatorDown = 5,
	MessAround = 6
};




// ###########
// # ParsedModels (bases)
// ######################

template<const size_t ARG_CNT = 1>
struct NumericSensorEventBase {
	using NumericValue = float;

	//static_assert (sizeof(Self) == (sizeof(NumericValue) * ARG_CNT), "Struct size and argument count do not match.");

	template<const size_t ARGIDX>
	NumericValue& getValue() {
		NumericValue* valuePtr = reinterpret_cast<NumericValue*>(this);
		return valuePtr[ARGIDX];
	}
	template<const size_t ARGIDX>
	const NumericValue& getValue() const {
		const NumericValue* valuePtr = reinterpret_cast<const NumericValue*>(this);
		return valuePtr[ARGIDX];
	}

	void parse(const std::string& parameterString) {
		NumericValue* resultPtr = reinterpret_cast<NumericValue*>(this);
		std::stringstream stream(parameterString);
		char delimiter = 0;
		for(size_t i = 0; i < ARG_CNT; ++i) {
			exceptAssert(stream.good(), "Unexpectedly reached EOS.");
			stream >> resultPtr[i];
			if(i != (ARG_CNT - 1)) {
				stream.read(&delimiter, 1);
				exceptAssert(delimiter == ';', "Found unexpected character != ;");
			} else {
				exceptAssert(stream.read(&delimiter, 1).fail(), "Too much input.");
			}
		}
	}
};
struct XYZSensorEventBase : public NumericSensorEventBase<3> {
	float x;
	float y;
	float z;
};

// ###########
// # ParsedModels
// ######################

struct AccelerometerEvent : public XYZSensorEventBase {};
struct GravityEvent : public XYZSensorEventBase {};
struct LinearAccelerationEvent : public XYZSensorEventBase {};
struct GyroscopeEvent : public XYZSensorEventBase {};
struct MagneticFieldEvent : public XYZSensorEventBase {};
struct PressureEvent : public NumericSensorEventBase<1> {
	float pressure;
};
struct OrientationEvent : public XYZSensorEventBase {};
struct RotationMatrixEvent : public NumericSensorEventBase<9> {
	float matrix[9];
};
struct WifiEvent {
	std::vector<WifiAdvertisement> advertisements;

	void parse(const std::string& parameterString);
};
struct BLEEvent {
	MacAddress mac;
	Rssi rssi;
	BluetoothTxPower txPower;

	void parse(const std::string& parameterString);
};
struct RelativeHumidityEvent : public NumericSensorEventBase<1> {
	float relativeHumidity;
};
struct OrientationOldEvent : public XYZSensorEventBase {};
struct RotationVectorEvent : public NumericSensorEventBase<4> {
	float x;
	float y;
	float z;
	float w;
};
struct LightEvent : public NumericSensorEventBase<1> {
	float light;
};
struct AmbientTemperatureEvent : public NumericSensorEventBase<1> {
	float ambientTemperature;
};
struct HeartRateEvent : public NumericSensorEventBase<1> {
	float heartRate;
};
struct GPSEvent {
	void parse(const std::string& parameterString);
};
struct WifiRTTEvent {
	void parse(const std::string& parameterString);
};
struct GameRotationVectorEvent : public XYZSensorEventBase {};


// #### Special Events ####
struct PedestrianActivityEvent {
	PedestrianActivity activity;

	void parse(const std::string& parameterString);
};
struct GroundTruthEvent {
	size_t groundTruthId;

	void parse(const std::string& parameterString);
};
struct GroundTruthPathEvent {
	size_t pathId;
	size_t groundTruthPointCnt;

	void parse(const std::string& parameterString);
};
struct FileMetadataEvent {
	std::string date;
	std::string person;
	std::string comment;

	void parse(const std::string& parameterString);
};


using EventData = std::variant<AccelerometerEvent, GravityEvent, LinearAccelerationEvent, GyroscopeEvent, MagneticFieldEvent, PressureEvent,
OrientationEvent, RotationMatrixEvent, WifiEvent, BLEEvent, RelativeHumidityEvent, OrientationOldEvent, RotationVectorEvent,
LightEvent, AmbientTemperatureEvent, HeartRateEvent, GPSEvent, WifiRTTEvent, GameRotationVectorEvent,
PedestrianActivityEvent, GroundTruthEvent, GroundTruthPathEvent, FileMetadataEvent>;

struct SensorEvent {

	Timestamp timestamp;
	EventType eventType;
	EventData data;

	static SensorEvent parse(const RawSensorEvent& rawEvent);
};


// ###########
// # VisitingParser
// ######################

class VisitingParser {

private: // Parser state
	std::istream& stream;

public: // API-Surface
	VisitingParser(std::istream& stream);

	bool nextLine(RawSensorEvent& sensorEvent);
};


// ###########
// # AggregatingParser
// ######################

class AggregatingParser {

public:
	using AggregatedParseResult = std::vector<SensorEvent>;
	using AggregatedRawParseResult = std::vector<RawSensorEvent>;

private:
	VisitingParser parser;

public:
	AggregatingParser(std::istream& stream);

	AggregatedParseResult parse();
	AggregatedRawParseResult parseRaw();
};



// ###########
// # Serializer
// ######################
class Serializer {

private: // Serializer state
	std::ostream& stream;

public:
	Serializer(std::ostream& stream);

	void write(const RawSensorEvent& sensorEvent);

};

}
