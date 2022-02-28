clear all;
loadSensorReadoutParser('src/');

% #########################################################################
% # Select File
% #########################################################################
[cutFile, cutFilePath] = uigetfile('*.csv');
assert(ischar(cutFile), 'Did not select file in dialog.');
fileName = [cutFilePath, cutFile];

parser = SensorReadoutParser(fileName, true);
[btAdvertisements, wifiAdvertisements, ftmMeasurements, ~] = parser.parseRadio();

bleTs = [btAdvertisements{:,1}];
wifiTs = [wifiAdvertisements{:,1}];
ftmTs = [ftmMeasurements{:,1}];

figure('name', ['2.4GHz Timestamps: ', fileName]);
subplot(2, 1, 1);
hold on;
scatter(bleTs, repmat(1, length(bleTs), 1), '+');
scatter(wifiTs, repmat(2, length(wifiTs), 1), '+');
scatter(ftmTs, repmat(3, length(ftmTs), 1), '+');
legend('BLE', 'Wifi', 'FTM');
hold off;
subplot(2, 1, 2);
hold on;
hist([ftmMeasurements{:,4}]);
ftmMean = mean([ftmMeasurements{:,4}]);
plot([ftmMean, ftmMean], [0, 100]);
hold off;