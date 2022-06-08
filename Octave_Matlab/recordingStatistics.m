clear all;
close all;
loadSensorReadoutParser('src/');
GRID_WIDTH = 4;

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
parser = SensorReadoutParser(fileName, true);

% #########################################################################
% # Generate Base Statistics
% #########################################################################
sensorSelector = SensorSelector()...
	.useSensor(SensorType.ACCELEROMETER)...
	.useSensor(SensorType.GYROSCOPE)...
	.useSensor(SensorType.MAGNETIC_FIELD);
timestampedSensorData = parser.parseSensorData();

fprintf('=== IMU Statistics ===\n');
if timestampedSensorData.hasSensorId(SensorType.ACCELEROMETER)
	[accTimestamps, accData] = timestampedSensorData.getChannel(SensorType.ACCELEROMETER);
	accHz = 1/mean(diff(accTimestamps));
	fprintf('\tAccelerometer: %f Hz\n', accHz);
end
if timestampedSensorData.hasSensorId(SensorType.GYROSCOPE)
	[gyroTimestamps, gyroData] = timestampedSensorData.getChannel(SensorType.GYROSCOPE);
	gyroHz = 1/mean(diff(gyroTimestamps));
	fprintf('\tGyroscope: %f Hz\n', gyroHz);
end
if timestampedSensorData.hasSensorId(SensorType.MAGNETIC_FIELD)
	[magnTimestamps, magnData] = timestampedSensorData.getChannel(SensorType.MAGNETIC_FIELD);
	magnHz = 1/mean(diff(magnTimestamps));
	fprintf('\tMagnetic Field Sensor: %f Hz\n', magnHz);
end


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

fprintf('=== FTM Statistics ===\n');
printf('\tAvg. distance: %.2f mm\n', mean([ftmMeasurements{:,4}]));
printf('\tStddev. distance: %.2f mm\n', std([ftmMeasurements{:,4}]));
figure('name', 'Measured FTM Distance distribution');
hist([ftmMeasurements{:,4}]);

figure('name', 'Measured FTM Distance distributions by device');
hold on;
rangingMacs = unique(ftmMeasurements(:,3));
minFtmDist = min([ftmMeasurements{:,4}]);
maxFtmDist = max([ftmMeasurements{:,4}]);

gridHeight = ceil(rows(rangingMacs) / GRID_WIDTH);
for(rangingId = [1:length(rangingMacs)])
	rangingMac = rangingMacs{rangingId};
	macIdcs = strcmp(rangingMac, ftmMeasurements(:,3));
	subplot(gridHeight, GRID_WIDTH, rangingId);
	hist([ftmMeasurements{macIdcs,4}]);
	xlim([minFtmDist, maxFtmDist]);
	title(rangingMac);
end
hold off;

figure('name', 'FTM measurement count per device');
hold on;
ftmMeasurementCnts = [];
for(rangingId = [1:length(rangingMacs)])
	rangingMac = rangingMacs{rangingId};
	macIdcs = strcmp(rangingMac, ftmMeasurements(:,3));
	measurementCnt = sum(macIdcs);
	ftmMeasurementCnts = [ftmMeasurementCnts, measurementCnt];
end
barh([1:length(rangingMacs)], ftmMeasurementCnts);
for(rangingId = [1:length(rangingMacs)])
	text(0.05 * max(ftmMeasurementCnts), rangingId, rangingMacs{rangingId});
end
hold off;