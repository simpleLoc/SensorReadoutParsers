clear all;
close all;
loadSensorReadoutParser('src/');

% #########################################################################
% # Select File
% #########################################################################
[cutFile, cutFilePath] = uigetfile('*.csv');
assert(ischar(cutFile), 'Did not select file in dialog.');
fileName = [cutFilePath, cutFile];
printf('Generating Recording Statistics: %s\n', fileName);

% #########################################################################
% # Parse & Generate Statistics
% #########################################################################
parser = SensorReadoutParser(fileName, false);

% #########################################################################
% # Generate Base Statistics
% #########################################################################
sensorSelector = SensorSelector()...
	.useSensor(SensorType.ACCELEROMETER)...
	.useSensor(SensorType.GYROSCOPE)...
	.useSensor(SensorType.MAGNETIC_FIELD);
timestampedSensorData = parser.parseSensorData();

[accTimestamps, accData] = timestampedSensorData.getChannel(SensorType.ACCELEROMETER);
[gyroTimestamps, gyroData] = timestampedSensorData.getChannel(SensorType.GYROSCOPE);
[magnTimestamps, magnData] = timestampedSensorData.getChannel(SensorType.MAGNETIC_FIELD);

accHz = 1/mean(diff(accTimestamps));
gyroHz = 1/mean(diff(gyroTimestamps));
magnHz = 1/mean(diff(magnTimestamps));

fprintf('=== IMU Statistics ===\n');
fprintf('\tAccelerometer: %f Hz\n', accHz);
fprintf('\tGyroscope: %f Hz\n', gyroHz);
fprintf('\tMagnetic Field Sensor: %f Hz\n', magnHz);

% #########################################################################
% # Generate Radio Statistics
% #########################################################################
[btAdvertisements, wifiAdvertisements, ftmMeasurements, ~] = parser.parseRadio();
btTimestamps = [btAdvertisements{:,1}];
wifiTimestamps = [wifiAdvertisements{:,1}];
ftmTimestamps = [ftmMeasurements{:,1}];

fprintf('=== Radio Statistics ===\n');
btPerSec = length(btTimestamps) / (max(btTimestamps) - min(btTimestamps));
printf('\tBLE events: %f/s\n', btPerSec);
wifiPerSec = length(wifiTimestamps) / (max(wifiTimestamps) - min(wifiTimestamps));
printf('\tWifi events: %f/s\n', wifiPerSec);
ftmPerSec = length(ftmTimestamps) / (max(ftmTimestamps) - min(ftmTimestamps));
printf('\tFTM events: %f/s\n', ftmPerSec);