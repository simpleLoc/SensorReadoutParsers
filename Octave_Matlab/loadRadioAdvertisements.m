clear all;
close all;
loadSensorReadoutParser('src/');

% #########################################################################
% # Select File
% #########################################################################
[cutFile, cutFilePath] = uigetfile('*.csv');
assert(ischar(cutFile), 'Did not select file in dialog.');
fileName = [cutFilePath, cutFile];

% #########################################################################
% # Parse & Resample
% #########################################################################
parser = SensorReadoutParser(fileName, false);
[btAdvertisements, wifiAdvertisements, uwbMeasurements] = parser.parseRadio();
groundTruthPoints = parser.parseGroundTruthPoints();

% #########################################################################
% # Iterate Bluetooth advertisements
% #########################################################################
for adIdx = 1:size(btAdvertisements, 1)
	timestamp = btAdvertisements{adIdx, 1};
	macAddress = btAdvertisements{adIdx, 2};
	rssis = btAdvertisements{adIdx, 3};
	txPower = btAdvertisements{adIdx, 4};

	fprintf('\tAdv: %s (rssi: %d) (txPwr: %d)\n', macAddress, rssis, txPower);
end

% #########################################################################
% # Iterate Wifi advertisements
% #########################################################################
for adIdx = 1:size(wifiAdvertisements, 1)
	timestamp = wifiAdvertisements{adIdx, 1};
	macAddresses = wifiAdvertisements{adIdx, 2};
	frequencies = wifiAdvertisements{adIdx, 3};
	rssis = wifiAdvertisements{adIdx, 4};
	
	fprintf('\n\n##### Advertisement (@ %f sec)\n', timestamp);
	for s = 1:length(macAddresses)
		fprintf('\tAdv: %s (freq: %d) (rssi: %d)\n', macAddresses{s}, frequencies(s), rssis(s));
	end
end

% #########################################################################
% # Iterate Ground Truth Marker Events
% #########################################################################
printGroundTruthPoints = flip(groundTruthPoints, 2)';
fprintf('- Reached Groundtruth Point %d (@ %f sec)\n', printGroundTruthPoints{:});
