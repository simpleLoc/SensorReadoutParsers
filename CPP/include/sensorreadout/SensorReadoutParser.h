#pragma once

#include <array>
#include <cinttypes>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <cstring>
#include <sstream>
#include <istream>
#include <vector>
#include <variant>

#include "Tokenizer.h"

namespace SensorReadoutParser {

	// ###########
	// # BaseTypes
	// ######################
	using Timestamp = uint64_t;
	using EventId = int32_t;
	using PedestrianActivityId = uint32_t;

	///
	/// \brief MarkerStruct representing a dynamic-sized string in hex encoding that
	/// should be decoded to a raw byte array.
	///
	struct HexString {
		std::vector<uint8_t> data;
	};

	struct UUID {
		static constexpr size_t UUID_LENGTH = 16;
		static constexpr size_t STRING_LENGTH = 16*2 + 4;

		std::array<uint8_t, UUID_LENGTH> data;

		static UUID fromString(const std::string_view& uuidStr);
		std::string toString() const;
	};

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
		std::string toColonDelimitedString() const;

		bool operator==(const MacAddress& o);
	};

	struct WifiAdvertisement {
		MacAddress mac;
		Rssi rssi;
		WifiFrequency channelFreq;
	};

	struct DecawaveUWBMeasurement {
		uint16_t nodeId;
		float distance;
		uint8_t qualityFactor;
	};



	// ###########
	// # Helpers
	// ######################

	template<> UUID fromStringView(const std::string_view&);
	template<> HexString fromStringView(const std::string_view&);
	template<> MacAddress fromStringView(const std::string_view&);

	namespace _internal {

		class ParameterAssembler {
		private:
			std::ostringstream stream;

		public:
			ParameterAssembler() {
				stream.precision(15);
			}
			template<typename TValue> void push(TValue value) {
				if(stream.tellp() > 0) { stream << ';'; }
				stream << value;
			}

			std::string str() const;
		};

	}


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
	static constexpr EventId EVENTID_EDDYSTONE_UID = 19;
	static constexpr EventId EVENTID_DECAWAVE_UWB = 20;
	static constexpr EventId EVENTID_STEP_DETECTOR = 21;
	static constexpr EventId EVENTID_HEADING_CHANGE = 22;
	static constexpr EventId EVENTID_FUTURESHAPE_SENSFLOOR = 23;
	static constexpr EventId EVENTID_MICROPHONE_METADATA = 24;
	// ------
	static constexpr EventId EVENTID_PEDESTRIAN_ACTIVITY = 50;
	static constexpr EventId EVENTID_GROUND_TRUTH = 99;
	static constexpr EventId EVENTID_GROUND_TRUTH_PATH = -1;
	static constexpr EventId EVENTID_FILE_METADATA = -2;
	static constexpr EventId EVENTID_RECORDING_ID = -3;

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
		EddystoneUID = EVENTID_EDDYSTONE_UID,
		DecawaveUWB = EVENTID_DECAWAVE_UWB,
		StepDetector = EVENTID_STEP_DETECTOR,
		HeadingChange = EVENTID_HEADING_CHANGE,
		FutureShapeSensFloor = EVENTID_FUTURESHAPE_SENSFLOOR,
		MicrophoneMetadata = EVENTID_MICROPHONE_METADATA,
		// Special events
		PedestrianActivity = EVENTID_PEDESTRIAN_ACTIVITY,
		GroundTruth = EVENTID_GROUND_TRUTH,
		GroundTruthPath = EVENTID_GROUND_TRUTH_PATH,
		FileMetadata = EVENTID_FILE_METADATA,
		RecordingId = EVENTID_RECORDING_ID
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
	template<const size_t ARG_CNT = 1, typename TNumericValue = float>
	struct NumericSensorEventBase {
		using NumericValue = TNumericValue;

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
			Tokenizer<';'> tokenizer(parameterString);
			for(size_t i = 0; i < ARG_CNT; ++i) {
				resultPtr[i] = tokenizer.nextAs<NumericValue>();
			}
		}

		void serializeInto(_internal::ParameterAssembler& stream) const {
			const NumericValue* resultPtr = reinterpret_cast<const NumericValue*>(this);
			for(size_t i = 0; i < ARG_CNT; ++i) {
				stream.push(resultPtr[i]);
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
		float pressureHpa;
	};
	struct OrientationEvent : public NumericSensorEventBase<3> {
		float azimuth;
		float pitch;
		float roll;
	};
	struct RotationMatrixEvent : public NumericSensorEventBase<9> {
		float matrix[9];
	};
	struct WifiEvent {
		std::vector<WifiAdvertisement> advertisements;

		void parse(const std::string& parameterString);
		void serializeInto(_internal::ParameterAssembler& stream) const;
	};
	struct BLEEvent {
		MacAddress mac;
		Rssi rssi;
		BluetoothTxPower txPower;
		/** The entire advertisement packet as raw bytes */
		std::vector<uint8_t> rawData;

		void parse(const std::string& parameterString);
		void serializeInto(_internal::ParameterAssembler& stream) const;
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
	struct GPSEvent : public NumericSensorEventBase<4> {
		float lat;
		float lon;
		float alt;
		float bearing;
	};
	struct WifiRTTEvent {
		bool success;
		MacAddress mac;
		/* distance in millimeters */
		int64_t distanceMM;
		/* stddev of the distance in millimeters */
		int64_t distanceStdDevMM;
		Rssi rssi;
		size_t numAttempted;
		size_t numSuccessfull;

		void parse(const std::string& parameterString);
		void serializeInto(_internal::ParameterAssembler& stream) const;
	};
	struct GameRotationVectorEvent : public XYZSensorEventBase {};
	struct EddystoneUIDEvent {
		MacAddress mac;
		Rssi rssi;
		BluetoothTxPower txPower;
		UUID uid;

		void parse(const std::string& parameterString);
		void serializeInto(_internal::ParameterAssembler& stream) const;
	};
	struct DecawaveUWBEvent {
		float x;
		float y;
		float z;
		uint8_t qualityFactor;
		std::vector<DecawaveUWBMeasurement> anchorMeasurements;

		void parse(const std::string& parameterString);
		void serializeInto(_internal::ParameterAssembler& stream) const;
	};
	struct StepDetectorEvent {
		Timestamp stepStartTs;
		Timestamp stepEndTs;
		float probability;

		void parse(const std::string& parameterString);
		void serializeInto(_internal::ParameterAssembler& stream) const;
	};
	struct HeadingChangeEvent : public NumericSensorEventBase<1, double> {
		double headingChangeRad;
	};
	struct FutureShapeSensFloorEvent {
		uint16_t roomId;
		uint8_t x;
		uint8_t y;
		std::array<uint8_t, 8> fieldCapacities;

		void parse(const std::string& parameterString);
		void serializeInto(_internal::ParameterAssembler& stream) const;
	};
	struct MicrophoneMetadataEvent {
		size_t channelCnt;
		size_t sampleRateHz;
		std::string sampleFormat;

		void parse(const std::string& parameterString);
		void serializeInto(_internal::ParameterAssembler& stream) const;
	};


	// #### Special Events ####
	struct PedestrianActivityEvent {
		PedestrianActivity activity;
		PedestrianActivityId rawActivityId;
		std::string rawActivityName;

		static PedestrianActivityEvent from(PedestrianActivity activity)  {
			PedestrianActivityEvent result;
			result.activity = activity;
			result.rawActivityId = static_cast<PedestrianActivityId>(activity);
			switch(activity) {
				case PedestrianActivity::Walking: result.rawActivityName = "WALKING"; break;
				case PedestrianActivity::Standing: result.rawActivityName = "STANDING"; break;
				case PedestrianActivity::StairsUp: result.rawActivityName = "STAIRS_UP"; break;
				case PedestrianActivity::StairsDown: result.rawActivityName = "STAIRS_DOWN"; break;
				case PedestrianActivity::ElevatorUp: result.rawActivityName = "ELEVATOR_UP"; break;
				case PedestrianActivity::ElevatorDown: result.rawActivityName = "ELEVATOR_DOWN"; break;
				case PedestrianActivity::MessAround: result.rawActivityName = "MESS_AROUND"; break;
			}
			return result;
		}

		void parse(const std::string& parameterString);
		void serializeInto(_internal::ParameterAssembler& stream) const;
	};
	struct GroundTruthEvent : public NumericSensorEventBase<1, size_t> {
		size_t groundTruthId;
	};
	struct GroundTruthPathEvent : public NumericSensorEventBase<2, size_t> {
		size_t pathId;
		size_t groundTruthPointCnt;
	};
	struct FileMetadataEvent {
		std::string date;
		std::string person;
		std::string comment;

		void parse(const std::string& parameterString);
		void serializeInto(_internal::ParameterAssembler& stream) const;
	};
	struct RecordingIdEvent {
		UUID recordingId;

		void parse(const std::string& prameterString);
		void serializeInto(_internal::ParameterAssembler& stream) const;
	};


	using EventData = std::variant<AccelerometerEvent, GravityEvent, LinearAccelerationEvent, GyroscopeEvent, MagneticFieldEvent, PressureEvent,
	OrientationEvent, RotationMatrixEvent, WifiEvent, BLEEvent, RelativeHumidityEvent, OrientationOldEvent, RotationVectorEvent, LightEvent,
	AmbientTemperatureEvent, HeartRateEvent, GPSEvent, WifiRTTEvent, GameRotationVectorEvent, EddystoneUIDEvent, DecawaveUWBEvent, StepDetectorEvent,
	HeadingChangeEvent, FutureShapeSensFloorEvent, MicrophoneMetadataEvent,
	PedestrianActivityEvent, GroundTruthEvent, GroundTruthPathEvent, FileMetadataEvent, RecordingIdEvent>;

	struct SensorEvent {

		Timestamp timestamp;
		EventType eventType;
		EventData data;

		static SensorEvent parse(const RawSensorEvent& rawEvent);
		void serializeInto(RawSensorEvent& rawEvent) const;
	};

	enum class FileVersion {
		/** This file format uses millisecond timestamps */
		V0,
		/** This file format uses nanosecond timestamps */
		V1
	};


	// ###########
	// # VisitingParser
	// ######################

	class VisitingParser {

	private: // Parser state
		std::istream& stream;
		FileVersion fileVersion;

	public: // API-Surface
		VisitingParser(std::istream& stream, FileVersion fileVersion = FileVersion::V1);

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
		AggregatingParser(std::istream& stream, FileVersion fileVersion = FileVersion::V1);

		AggregatedParseResult parse();
		AggregatedRawParseResult parseRaw();
	};



	// ###########
	// # Serializer
	// ######################
	class Serializer {

	private: // Serializer state
		std::ostream& stream;
		FileVersion fileVersion;

	public:
		Serializer(std::ostream& stream, FileVersion fileVersion = FileVersion::V1);
		~Serializer();

		void write(const RawSensorEvent& sensorEvent);
		void write(const SensorEvent& sensorEvent);

		void flush();
	};
}
