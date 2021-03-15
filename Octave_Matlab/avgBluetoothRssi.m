clear all;
close all;
loadSensorReadoutParser('src/');

% #########################################################################
% # Config
% #########################################################################
MAC_FILTER="AABBCCDDEEFF"

% #########################################################################
% # Select File
% #########################################################################
[cutFile, cutFilePath] = uigetfile('*.csv');
assert(ischar(cutFile), 'Did not select file in dialog.');
fileName = [cutFilePath, cutFile];

% #########################################################################
% # Parse
% #########################################################################
parser = SensorReadoutParser(fileName, false);
[btAdvertisements, wifiAdvertisements, uwbMeasurements] = parser.parseRadio();
groundTruthPoints = parser.parseGroundTruthPoints();
timestamps = parser.getTimestamps();

btAdvMacs = [btAdvertisements{:, 2}];
accessIdxs = strcmp(btAdvMacs, MAC_FILTER);
mean([btAdvertisements{accessIdxs, 3}])
plot([btAdvertisements{accessIdxs, 3}])