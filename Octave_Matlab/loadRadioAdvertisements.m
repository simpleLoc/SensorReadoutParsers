clear all;
close all;
loadSensorReadoutParser("src/");

% #########################################################################
% # Select File
% #########################################################################
[cutFile, cutFilePath] = uigetfile();
assert(cutFile ~= 0, 'Did not select file in dialog.');
fileName = [cutFilePath, cutFile];

% #########################################################################
% # Parse & Resample
% #########################################################################
parser = SensorReadoutParser(false);
[btAdvertisements, wifiAdvertisements] = parser.parseRadio(fileName);

% #########################################################################
% # Iterate Bluetooth advertisements
% #########################################################################
for adIdx = 1:rows(btAdvertisements)
	timestamp = btAdvertisements{adIdx, 1};
	macAddresses = btAdvertisements{adIdx, 2};
	rssis = btAdvertisements{adIdx, 3};
	txPower = btAdvertisements{adIdx, 4};

	printf("\n\n##### Advertisement (@ %f sec)\n", timestamp);
	for s = 1:length(macAddresses)
		printf("\tAdv: %s (rssi: %d) (txPwr: %d)\n", macAddresses{s}, rssis(s), txPower(s));
	end
end

% #########################################################################
% # Iterate Wifi advertisements
% #########################################################################
for adIdx = 1:rows(wifiAdvertisements)
	timestamp = wifiAdvertisements{adIdx, 1};
	macAddresses = wifiAdvertisements{adIdx, 2};
	frequencies = wifiAdvertisements{adIdx, 3};
	rssis = wifiAdvertisements{adIdx, 4};
	
	printf("\n\n##### Advertisement (@ %f sec)\n", timestamp);
	for s = 1:length(macAddresses)
		printf("\tAdv: %s (freq: %d) (rssi: %d)\n", macAddresses{s}, frequencies(s), rssis(s));
	end
end
