clear all;
close all;
loadSensorReadoutParser('src/');

% #########################################################################
% # Config
% #########################################################################
SAMPLE_INTERVAL = (1/50); % 50Hz
SCREEN_SIZE = [1920, 1080];

sensorSelector = SensorSelector()...
	.useSensor(SensorType.ACCELEROMETER)...
	.useSensor(SensorType.GYROSCOPE);

% #########################################################################
% # Select File
% #########################################################################
[cutFile, cutFilePath] = uigetfile('*.csv');
assert(ischar(cutFile), 'Did not select file in dialog.');
fileName = [cutFilePath, cutFile];

% #########################################################################
% # Parse & Resample
% #########################################################################
parser = SensorReadoutParser(false);
timestampedSensorData = parser.parseSensorData(fileName);
recording = timestampedSensorData.resampleIntoDataContainer(SAMPLE_INTERVAL);

% #########################################################################
% # Render
% #########################################################################
[recordingTimestamps, recordingDataMatrix] = recording.getSensorData(sensorSelector);
activityChanges = recording.getActivityChanges();
activityChangeCnt = length(activityChanges{1});

activityColors = ActionType.actions2Color(activityChanges{2});

figureHandle = figure('name', 'Recording');
dimensionCnt = size(recordingDataMatrix, 1);
for d = 1:dimensionCnt
	subplot(dimensionCnt, 1, d);
	plot(recordingTimestamps, recordingDataMatrix(d,:));
	axis tight;
	if d == 1
		minY = min(recordingDataMatrix(d,:));
		maxY = max(recordingDataMatrix(d,:));
		hold on;
		for a = 1:activityChangeCnt
			plot(repmat(activityChanges{1}(a),2,1), [minY,maxY], 'Color', activityColors(a,:), 'LineWidth', 2);
		end
		hold off;
	end
end

% apply configured screen-size
set(figureHandle, 'Position', [0, 0, SCREEN_SIZE]);