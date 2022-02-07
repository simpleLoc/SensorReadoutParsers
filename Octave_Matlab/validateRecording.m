clear all;
close all;
loadSensorReadoutParser('src/');

% #########################################################################
% # Select File
% #########################################################################
[cutFile, cutFilePath] = uigetfile('*.csv');
assert(ischar(cutFile), 'Did not select file in dialog.');
fileName = [cutFilePath, cutFile];
printf('Validating Recording: %s\n', fileName);

% #########################################################################
% # Parse & Validate
% #########################################################################
parser = SensorReadoutParser(fileName, true);
parser.validateRecording();