import typing

from pathlib import Path
from enum import Enum, unique

import pandas as pd
import pandera as pa

import datetime
import dateutil


@unique
class SensorEventId(Enum):
    ACCELEROMETER = 0
    GRAVITY = 1
    LINEAR_ACCELERATION = 2
    GYROSCOPE = 3
    MAGNETIC_FIELD = 4
    PRESSURE = 5
    ORIENTATION_NEW = 6
    ROTATION_MATRIX = 7
    WIFI = 8
    BLE = 9  # aka IBEACON
    RELATIVE_HUMIDITY = 10
    ORIENTATION_OLD = 11
    ROTATION_VECTOR = 12
    LIGHT = 13
    AMBIENT_TEMPERATURE = 14
    HEART_RATE = 15
    GPS = 16
    WIFIRTT = 17
    GAME_ROTATION_VECTOR = 18
    EDDYSTONE_UID = 19
    DECAWAVE_UWB = 20
    STEP_DETECTOR = 21
    HEADING_CHANGE = 22

    # Special non Sensor events
    PEDESTRIAN_ACTIVITY = 50
    GROUND_TRUTH = 99
    GROUND_TRUTH_PATH = -1
    FILE_METADATA = -2
    RECORDING_ID = -3


@unique
class PedestrianActivity(Enum):
    Walking = 0
    Standing = 1
    StairsUp = 2
    StairsDown = 3
    ElevatorUp = 4
    ElevatorDown = 5
    MessAround = 6


class SensorReadoutSchema:
    class __FastDtypeCheck:
        def __init__(self):
            # Hack: Get dtypes for Columns to improve type checking in the hot path
            self.bool = pa.Column(bool).dtype
            self.int8 = pa.Column(pa.dtypes.Int8).dtype
            self.int16 = pa.Column(pa.dtypes.Int16).dtype
            self.int = pa.Column(int).dtype
            self.float = pa.Column(float).dtype
            self.string = pa.Column(str).dtype

    def __init__(self):
        self.dtypes = self.__FastDtypeCheck()

        self.eventsChronologically = pa.DataFrameSchema({
            'eventIdx': pa.Column(int),
            'timestamp': pa.Column(int),
            'eventID': pa.Column(int),  # TODO EventID Validator
        })

        # One SensorEventId.DECAWAVE_UWB event can have up to four distance measurements
        self.uwbDistances = pa.DataFrameSchema({
            'eventIdx': pa.Column(int),
            'nodeId': pa.Column(pa.dtypes.Int16),
            'distance': pa.Column(int),
            'qualityFactor': pa.Column(pa.dtypes.Int8),
        })

        self.sensors = {
            SensorEventId.ACCELEROMETER: self._createXYZSchema(),
            SensorEventId.GRAVITY: self._createXYZSchema(),
            SensorEventId.LINEAR_ACCELERATION: self._createXYZSchema(),
            SensorEventId.GYROSCOPE: self._createXYZSchema(),
            SensorEventId.MAGNETIC_FIELD: self._createXYZSchema(),
            SensorEventId.PRESSURE: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'pressureHpa': pa.Column(float),
            }),
            SensorEventId.ORIENTATION_NEW: pa.DataFrameSchema({
               'eventIdx': pa.Column(int),
               'timestamp': pa.Column(int),
               'azimuth': pa.Column(float),
               'pitch': pa.Column(float),
               'roll': pa.Column(float),
            }),
            # TODO SensorEventId.ROTATION_MATRIX
            SensorEventId.WIFI: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'mac': pa.Column(str),
                'channelFreq': pa.Column(int),
                'rssi': pa.Column(int),
            }),
            SensorEventId.BLE: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'mac': pa.Column(str),
                'rssi': pa.Column(int),
                'txPower': pa.Column(int),
            }),
            SensorEventId.RELATIVE_HUMIDITY: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'relativeHumidity': pa.Column(float),
            }),
            SensorEventId.ORIENTATION_OLD: self._createXYZSchema(),
            SensorEventId.ROTATION_VECTOR: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'X': pa.Column(float),
                'Y': pa.Column(float),
                'Z': pa.Column(float),
                'W': pa.Column(float),
            }),
            SensorEventId.LIGHT: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'light': pa.Column(float),
            }),
            SensorEventId.AMBIENT_TEMPERATURE: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'ambientTemperature': pa.Column(float),
            }),
            SensorEventId.HEART_RATE: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'heartRate': pa.Column(float),
            }),
            SensorEventId.GPS: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'lat': pa.Column(float),
                'lon': pa.Column(float),
                'alt': pa.Column(float),
                'bearing': pa.Column(float),
            }),
            SensorEventId.WIFIRTT: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'success': pa.Column(bool),
                'mac': pa.Column(str),
                'distInMM': pa.Column(int),
                'distInStdDevMM': pa.Column(int),
                'rssi': pa.Column(int),
                'numAttempted': pa.Column(int),
                'numSuccessful': pa.Column(int),
            }),
            SensorEventId.GAME_ROTATION_VECTOR: self._createXYZSchema(),
            SensorEventId.EDDYSTONE_UID: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'mac': pa.Column(str),
                'rssi': pa.Column(int),
                'uuid': pa.Column(str),
            }),
            SensorEventId.DECAWAVE_UWB: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'X': pa.Column(float),
                'Y': pa.Column(float),
                'Z': pa.Column(float),
                'qualityFactor': pa.Column(pa.dtypes.Int8),
            }),
            SensorEventId.STEP_DETECTOR: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'stepStartTs': pa.Column(int),
                'stepEndTs': pa.Column(int),
                'probability': pa.Column(float),
            }),
            SensorEventId.HEADING_CHANGE: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'headingChangeInRad': pa.Column(float),
            }),
            SensorEventId.PEDESTRIAN_ACTIVITY: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'rawActivityName': pa.Column(str),
                'rawActivityId': pa.Column(int)
            }),
            SensorEventId.GROUND_TRUTH: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'groundTruthId': pa.Column(int),
            }),
            SensorEventId.GROUND_TRUTH_PATH: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'pathId': pa.Column(int),
                'groundTruthPointCnt': pa.Column(int),
            }),
            SensorEventId.FILE_METADATA: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'date': pa.Column(str),
                'person': pa.Column(str),
                'comment': pa.Column(str),
            }),
            SensorEventId.RECORDING_ID: pa.DataFrameSchema({
                'eventIdx': pa.Column(int),
                'timestamp': pa.Column(int),
                'recordingId': pa.Column(str),
            }),
        }

    @staticmethod
    def _createXYZSchema():
        return pa.DataFrameSchema({
            'eventIdx': pa.Column(int),
            'timestamp': pa.Column(int),
            'X': pa.Column(float),
            'Y': pa.Column(float),
            'Z': pa.Column(float),
        })


class FileMetadata:
    def __init__(self, metadata: pd.DataFrame):
        self.person = metadata.person.iloc[0]
        self.comment = metadata.comment.iloc[0]
        self.date = dateutil.parser.parse(metadata.date.iloc[0])

    def absoluteTime(self, tsInNS: int) -> datetime.datetime:
        delta = datetime.timedelta(microseconds=tsInNS / 1000)
        return self.date + delta


class SensorReadoutData:
    def __init__(self):
        self.eventsChronologically: pd.DataFrame
        self.sensorData: typing.Dict[SensorEventId, pd.DataFrame] = {}
        self.uwbDistances: pd.DataFrame


    def hasDataForSensor(self, sensorEventId: SensorEventId) -> bool:
        return (sensorEventId in self.sensorData) and len(self.sensorData[sensorEventId]) > 0

    def file_metadata(self) -> FileMetadata:
        return FileMetadata(self.sensorData[SensorEventId.FILE_METADATA])

    def gt(self):
        return self.sensorData[SensorEventId.GROUND_TRUTH]

    def accel(self):
        return self.sensorData[SensorEventId.ACCELEROMETER]

    def gyro(self):
        return self.sensorData[SensorEventId.GYROSCOPE]

    def magnetic(self):
        return self.sensorData[SensorEventId.MAGNETIC_FIELD]

    def wifi(self):
        return self.sensorData[SensorEventId.WIFI]

    def wifi_rtt(self):
        return self.sensorData[SensorEventId.WIFIRTT]

    def ble(self):
        return self.sensorData[SensorEventId.BLE]

    def uwb(self):
        return self.sensorData[SensorEventId.DECAWAVE_UWB]


class _FileReader:
    def __init__(self, filename: Path):
        self.filename = filename
        self.lineNumber = 0

    def __enter__(self):
        self.fd = self.filename.open('rt', encoding="utf-8", newline='\n')
        return self

    def __exit__(self, type, value, traceback):
        self.fd.close()

    def readline(self) -> str:
        self.lineNumber += 1
        return self.fd.readline()

    def readEmptyLine(self):
        line = self.readline()
        assert line == '\n', f'Expected empty line at {self.lineNumber-1}'

    def lookahead(self) -> str:
        result = self.fd.read(1)
        self.fd.seek(self.fd.tell()-1)
        return result


class SensorReadoutParser:
    def __init__(self, filename: Path):
        self.inputFilename = Path(filename)
        self.schema = SensorReadoutSchema()
        self._eventIdLookup = {e.value: e for e in list(SensorEventId)}  # Avoid Enum.__call__ because dict lookup is fast

    def is_sensor_supported(self, eventId: SensorEventId):
        return eventId in self.schema.sensors

    def parse_radio(self) -> SensorReadoutData | typing.Dict[str, SensorReadoutData]:
        return self.parse([SensorEventId.WIFI, SensorEventId.WIFIRTT, SensorEventId.BLE, SensorEventId.DECAWAVE_UWB])

    def parse(self, activeSensors: typing.List[SensorEventId] = None) -> SensorReadoutData | typing.Dict[str, SensorReadoutData]:
        assert self.inputFilename.exists(), f'Input file "{str(self.inputFilename)}" not found'
        assert self.inputFilename.is_file(), f'Input path "{str(self.inputFilename)}" is not a file'

        # setup lookup for supported sensors or sensors requested by the user
        if activeSensors is None:
            sensorLookup = set(self.schema.sensors.keys())  # Use all sensors we know
        else:
            # Special non sensor based events we always want to read
            sensorLookup = {SensorEventId.FILE_METADATA, SensorEventId.GROUND_TRUTH_PATH, SensorEventId.GROUND_TRUTH, SensorEventId.PEDESTRIAN_ACTIVITY}
            for s in activeSensors:
                if self.is_sensor_supported(s):
                    sensorLookup.add(s)

        sensorLookup = {e.value for e in sensorLookup}  # convert Enum to int; As lookup is faster for int

        # only use sensorLookup from here
        del activeSensors

        result = {}

        with _FileReader(self.inputFilename) as file:
            while True:
                fpName = ''
                lookahead = file.lookahead()

                if lookahead == '':
                    break  # EOF
                elif lookahead == '[':
                    lineStr = file.readline().strip()
                    assert lineStr == '[fingerprint:point]', f'Unknown fingerprint header "{lineStr}" at line {file.lineNumber}.'

                    lineStr = file.readline().strip()
                    assert lineStr.startswith('name='), f'Unexpected text after fingerprint header at line {file.lineNumber}.'

                    fpName = lineStr[len('name='):]

                    assert fpName not in result, f'Duplicated fingerprint name {fpName}'

                    #print(fpName)

                    # skip empty line
                    file.readEmptyLine()


                data = self.__process_data(file, sensorLookup)
                result[fpName] = data

        if len(result) == 1 and '' in result:
            return result['']  # single SensorReadout file
        else:
            return result  # named fingerprint file


    def __process_data(self, file: _FileReader, sensorLookup: typing.Set[int]) -> SensorReadoutData:
        orderedEvents = []
        uwbDistances = []

        rawEvents = {}
        for supportedSensor in self.schema.sensors.keys():
            rawEvents[supportedSensor] = []

        eventIndex = -1

        # read file
        while lineStr := file.readline():
            eventIndex += 1

            lineStr = lineStr.strip()

            if lineStr == '':
                break

            # Split line
            parts = lineStr.split(';')
            if len(parts) < 2:
                raise Exception(f'Invalid syntax at line {file.lineNumber}!')

            timestamp = int(parts[0])
            eventIdInt = int(parts[1])

            # Check if sensor is known by the parser and if it is activated by the user
            if eventIdInt not in sensorLookup:
                continue

            eventId = self._eventIdLookup[eventIdInt]  # SensorEventId(int(parts[1])) is much slower than dict lookup

            # Process sensor specific data
            (sensorData, uwbDists) = self.__process_line(eventIndex, eventId, parts)

            eventData = [eventIndex, timestamp]
            eventData.extend(sensorData)

            rawEvents[eventId].append(eventData)
            orderedEvents.append([eventIndex, timestamp, eventId])

            if uwbDists is not None:
                uwbDistances.extend(uwbDists)

        # Construct DataFrames
        result = SensorReadoutData()
        result.eventsChronologically = pd.DataFrame(orderedEvents, columns=self.schema.eventsChronologically.columns.keys())
        result.uwbDistances = pd.DataFrame(uwbDistances, columns=self.schema.uwbDistances.columns.keys())
        for eventId, dataList in rawEvents.items():
            result.sensorData[eventId] = pd.DataFrame(dataList, columns=self.schema.sensors[eventId].columns.keys())

        # TODO implement validation before index; also types are not valid e.g. int8/int16
        # needs coerce_dtype() but is super slow (don't forget coerce=True)
        # eventIdx column will be dropped and the validation will be invalid because of the missing column

        # Set eventIdx as Index for alle DataFrames
        result.eventsChronologically.set_index('eventIdx', inplace=True)
        result.uwbDistances.set_index('eventIdx', inplace=True)
        for eventId in result.sensorData.keys():
            result.sensorData[eventId].set_index('eventIdx', inplace=True)

        return result


    def __process_line(self, event_index: int, eventId: SensorEventId, parts: typing.List[str]):
        schema = self.schema.sensors[eventId]

        sensordata = []
        uwbDists = None

        for colIndex, col in enumerate(schema.columns.values()):
            # skip eventIdx and timestamp column
            if colIndex <= 1:
                continue

            part = parts[colIndex]
            col_dtype = col.dtype

            # Using pa.dtypes.is_subdtype() (e.g. pa.dtypes.is_int() etc.) is the right way to check the dtype
            # However, this is the hot path and is_subdtype() slows down the parser significantly.
            if col_dtype == self.schema.dtypes.float:
                sensordata.append(float(part))
            elif col_dtype == self.schema.dtypes.string:
                sensordata.append(str(part))
            elif col_dtype == self.schema.dtypes.int or col_dtype == self.schema.dtypes.int8:
                sensordata.append(int(part))
            elif col_dtype == self.schema.dtypes.bool:
                sensordata.append(bool(part))
            else:
                raise Exception(f'Invalid column type "{col_dtype}" for "{eventId}"')

        # Special handling UWB
        if eventId == SensorEventId.DECAWAVE_UWB:
            # process variable length part of UWB
            uwbDists = []
            i = len(schema.columns)
            while i < len(parts):
                nodeId = int(parts[i + 0])
                distInMM = int(parts[i + 1])
                quality = int(parts[i + 2])
                uwbDists.append([event_index, nodeId, distInMM, quality])

                i += 3

        return sensordata, uwbDists


