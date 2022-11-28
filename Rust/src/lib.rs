use std::{io::{self, BufRead, Read, BufReader, Write}, str::FromStr, os::linux::raw};
use num_traits::{ToPrimitive, AsPrimitive, FromPrimitive};
use thiserror::Error;

#[derive(Error, Debug)]
pub enum SensorReadoutParserError {
    #[error("IoError during parsing")]
    IoError(#[from] io::Error),
	#[error("An error in the file's format has occured and could not be parsed: {0}")]
	FileFormatError(String),
}

use num_derive::{FromPrimitive, ToPrimitive};

pub type Timestamp = u64;
pub type EventId = i64;

#[repr(i64)]
#[derive(FromPrimitive, ToPrimitive, Debug, Clone)]
pub enum EventType {
	// Sensor events
	Accelerometer = 0,
	Gravity = 1,
	LinearAcceleration = 2,
	Gyroscope = 3,
	MagneticField = 4,
	Pressure = 5,
	Orientation = 6,
	RotationMatrix = 7,
	Wifi = 8,
	BLE = 9,
	RelativeHumidity = 10,
	OrientationOld = 11,
	RotationVector = 12,
	Light = 13,
	AmbientTemperature = 14,
	HeartRate = 15,
	GPS = 16,
	WifiRTT = 17,
	GameRotationVector = 18,
	EddystoneUID = 19,
	DecawaveUWB = 20,
	StepDetector = 21,
	HeadingChange = 22,
	FutureShapeSensFloor = 23,
	// Special events
	PedestrianActivity = 50,
	GroundTruth = 99,
	GroundTruthPath = -1,
	FileMetadata = -2,
	RecordingId = -3
}

#[derive(Debug, Clone)]
pub struct RawSensorEvent {
    pub timestamp: Timestamp,
	pub event_type: EventType,
	pub parameter_str: String
}

#[derive(Clone)]
pub enum SensorEventData {
	FileMetadata {
		date: chrono::DateTime<chrono::Utc>,
		person: String,
		comment: String
	}
}
impl SensorEventData {
	pub fn parse(rawevt: &RawSensorEvent) -> Result<SensorEventData, SensorReadoutParserError> {
		Ok(match rawevt.event_type {
			EventType::FileMetadata => {
				let parts: Vec<_> = rawevt.parameter_str.splitn(3, ';').collect();
				if parts.len() != 3 { return Err(SensorReadoutParserError::FileFormatError("Invalid number of parameters".to_owned())); }
				SensorEventData::FileMetadata {
					date: parts[0].parse::<chrono::DateTime<chrono::Utc>>()
						.map_err(|_| SensorReadoutParserError::FileFormatError("Failed to parse file timestamp".to_owned()))?,
					person: parts[1].to_owned(),
					comment: parts[2].to_owned(),
				}
			},
			_ => unreachable!()
		})
	}
	pub fn serialize<W: Write>(&self, out: &mut W) -> io::Result<()> {
		match self {
			SensorEventData::FileMetadata { date, person, comment } => {
				out.write_fmt(format_args!("{};{};{}", date.to_rfc3339(), person, comment))?;
			},
		}
		Ok(())
	}
	pub fn serialize_to_string(&self) -> io::Result<String> {
		let mut result = Vec::new();
		self.serialize(&mut result)?;
		Ok(String::from_utf8(result).unwrap())
	}
}

pub struct SensorEvent {
	pub timestamp: Timestamp,
	pub data: SensorEventData
}
impl SensorEvent {
	pub fn parse(rawevt: &RawSensorEvent) -> Result<SensorEvent, SensorReadoutParserError> {
		Ok(SensorEvent {
			timestamp: rawevt.timestamp,
			data: SensorEventData::parse(rawevt)?
		})
	}
}

struct VisitingParser<R: Read> {
    input: BufReader<R>,
	line_buffer: String
}
impl<R: Read> VisitingParser<R> {
    pub fn new(input: R) -> Self {
        Self {
			input: BufReader::new(input),
			line_buffer: String::new()
		}
    }
    
    pub fn next(&mut self) -> Result<Option<RawSensorEvent>, SensorReadoutParserError> {
		self.line_buffer.clear();
		let cnt = self.input.read_line(&mut self.line_buffer)?;
		if cnt == 0 { return Ok(None); }
        let parts: Vec<_> = self.line_buffer.splitn(3, ';').collect();
		if parts.len() != 3 {
			return Err(SensorReadoutParserError::FileFormatError("General line format <ts>;<evtId>;<params;...> not satisfied".to_owned()));
		}
		Ok(Some(RawSensorEvent {
			timestamp: parts[0].parse::<Timestamp>()
				.map_err(|_| SensorReadoutParserError::FileFormatError("Timestamp could not be parsed".to_owned()))?,
			event_type: {
				let event_id = parts[1].parse::<i64>()
					.map_err(|_| SensorReadoutParserError::FileFormatError("EventId could not be parsed".to_owned()))?;
				FromPrimitive::from_i64(event_id)
					.ok_or(SensorReadoutParserError::FileFormatError(format!("Encountered unknown EventType: {}", event_id)))?
			},
			parameter_str: parts[2].trim_end().to_owned()
		}))
    }
}

pub struct AggregatingParser {}
impl AggregatingParser {
    pub fn parse<R: Read>(input: R) -> Result<Vec<RawSensorEvent>, SensorReadoutParserError> {
		let mut events = Vec::new();
		let mut parser = VisitingParser::new(input);
		loop {
			match parser.next() {
				Ok(Some(evt)) => events.push(evt),
				Ok(None) => return Ok(events),
				Err(e) => return Err(e)
			}
		}
    }
}

pub struct Serializer {}
impl Serializer {
	pub fn serialize<W: Write>(output: &mut W, evts: &Vec<RawSensorEvent>) -> std::io::Result<()> {
		for evt in evts {
			let evtid = ToPrimitive::to_i64(&evt.event_type).unwrap();
			output.write_fmt(format_args!("{};{};{}\n", evt.timestamp, evtid, evt.parameter_str))?;
		}
		Ok(())
	}
}


// event_type: {
// 	let event_id = parts[1].parse::<i64>()
// 		.map_err(|_| SensorReadoutParserError::FileFormatError("EventId could not be parsed".to_owned()))?;
// 	FromPrimitive::from_i64(event_id)
// 		.ok_or(SensorReadoutParserError::FileFormatError("Encountered unknown EventType".to_owned()))?
// },



#[cfg(test)]
mod tests {
	//TODO
}
