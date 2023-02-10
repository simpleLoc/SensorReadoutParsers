clear all;
close all;
loadSensorReadoutParser('src/');
GRID_WIDTH = 4;
STATISTIC_WINDOW_SIZE_SEC = 30;

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
recTimestamps = parser.getTimestamps();

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
[btAdvertisements, wifiAdvertisements, ftmMeasurements, uwbMeasurements] = parser.parseRadio();
btTimestamps = [btAdvertisements{:,1}]; btRssiValues = [btAdvertisements{:,3}];
wifiTimestamps = [wifiAdvertisements{:,1}];
ftmTimestamps = [ftmMeasurements{:,1}];
uwbMeasurements = [uwbMeasurements{:,1}];

fprintf('=== Recording Statistics ===\n');
recDuration = (recTimestamps(end) - recTimestamps(1));
fprintf('\tDuration: %.3fs\n', recDuration);

fprintf('=== Radio Statistics ===\n');
hasBLE = ~isempty(btTimestamps);
hasWifi = ~isempty(wifiTimestamps);
hasFTM = ~isempty(ftmTimestamps);
hasUWB = ~isempty(uwbTimestamps);
% BLUETOOTH
if(hasBLE)
	btPerSec = length(btTimestamps) / (max(btTimestamps) - min(btTimestamps));
	printf('\tBLE events: %d\n', length(btTimestamps));
	printf('\t- Timeframe: [%.2fs : %.2fs]\n', min(btTimestamps), max(btTimestamps));
	printf('\t- RSSI Distribution: [mu: %.2fdB, stddev: %.2fdB]\n', mean(btRssiValues), std(btRssiValues));
	printf('\t- Samplerate: %.2f/s\n', btPerSec);
else
	printf('\tNo BLE events\n');
end
% WIFI
if(hasWifi)
	wifiPerSec = length(wifiTimestamps) / (max(wifiTimestamps) - min(wifiTimestamps));
	printf('\tWifi events: %d\n', length(wifiTimestamps));
	printf('\t- Timeframe: [%.2fs : %.2fs]\n', min(wifiTimestamps), max(wifiTimestamps));
	printf('\t- Samplerate: %.2f/s\n', wifiPerSec);
else
	printf('\tNo Wifi events\n');
end
% FTM
if(hasFTM)
	ftmPerSec = length(ftmTimestamps) / (max(ftmTimestamps) - min(ftmTimestamps));
	validSuccessIdcs = ([ftmMeasurements{:,8}] > 0 & [ftmMeasurements{:,7}] > 0);
	ftmAvgSuccessRate = mean(double([ftmMeasurements{validSuccessIdcs,8}]) ./ double([ftmMeasurements{validSuccessIdcs,7}])) * 100;
	printf('\tFTM events: %d\n', length(ftmTimestamps));
	printf('\t- Timeframe: [%.2fs : %.2fs]\n', min(ftmTimestamps), max(ftmTimestamps));
	printf('\t- Samplerate: %.2f/s\n', ftmPerSec);
	printf('\t- Avg Success: %.2f%%\n', ftmAvgSuccessRate);
	printf('\t- Avg. distance: %.2fmm\n', mean([ftmMeasurements{:,4}]));
	printf('\t- Range INTER-Burst Stddev: %.2fmm\n', std([ftmMeasurements{:,4}]));
	printf('\t- Avg. Range INTRA-Burst Stddev: %.2fmm\n', mean([ftmMeasurements{:,5}]));
else
	printf('\tNo FTM events\n');
end
% UWB
if(hasUWB)
	uwbPerSec = length(uwbTimestamps) / (max(uwbTimestamps) - min(uwbTimestamps));
	printf('\tUWB events: %d\n', length(uwbTimestamps));
	printf('\t- Timeframe: [%.2fs : %.2fs]\n', min(uwbTimestamps), max(uwbTimestamps));
	printf('\t- Avg estimated Accuracy (0-100): %f\n', 0);
else
	printf('\tNo UWB events\n');
end


# Plots
if(hasBLE)
	BIN_DURATION = 5;
	figure('name', sprintf('BLE measurement count over time [BIN_DURATION = %f sec]', BIN_DURATION));
	timeBinStarts = [recTimestamps(1):BIN_DURATION:recTimestamps(end)-BIN_DURATION];
	timeBinEnds = timeBinStarts + BIN_DURATION;
	timeBinIdcs = (btTimestamps > timeBinStarts') & (btTimestamps < timeBinEnds');
	bleOccurencesPerTimeBin = sum(timeBinIdcs, 2);
	plot(timeBinStarts, bleOccurencesPerTimeBin);
	xlim([recTimestamps(1), recTimestamps(end)]);
	ylim([0, max(bleOccurencesPerTimeBin)]);

	figure('name', 'BLE RSSI histogram');
	hist(btRssiValues);
end

if(hasFTM)
	figure('name', 'Measured FTM Distance distribution');
	span = (max([ftmMeasurements{:,4}]) - min([ftmMeasurements{:,4}])) / 1000;
	hist([ftmMeasurements{:,4}], ceil(span));

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

	figure('name', sprintf('FTM behavior over time (%.2fs windows)', STATISTIC_WINDOW_SIZE_SEC));
	windowCnt = recDuration / STATISTIC_WINDOW_SIZE_SEC;
	windowStarts = [1:windowCnt] * STATISTIC_WINDOW_SIZE_SEC;
	fotRangeStdDev = [];
	fotMeasPerSec = [];
	for wIdx = 1:windowCnt
		wStart = (wIdx - 1) * STATISTIC_WINDOW_SIZE_SEC;
		wEnd = wStart + STATISTIC_WINDOW_SIZE_SEC;
		windowMeasIdcs = (ftmTimestamps >= wStart & ftmTimestamps <= wEnd);
		windowMeasurements = ftmMeasurements(windowMeasIdcs, :);
		% calculate statistics
		%% stddev in window
		fotRangeStdDev(end + 1) = std([windowMeasurements{:, 4}]);
		%% samplerate in window
		fotMeasPerSec(end + 1) = rows(windowMeasurements) / STATISTIC_WINDOW_SIZE_SEC;
	end
	subplot(2, 1, 1);
	plot(windowStarts, fotRangeStdDev); title('Range Standard Deviation');
	subplot(2, 1, 2);
	plot(windowStarts, fotMeasPerSec); title('Measurements /s');
end
