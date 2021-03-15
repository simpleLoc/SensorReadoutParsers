clear all;
close all;
loadSensorReadoutParser('src/');

% #########################################################################
% # Config
% #########################################################################
TIMESTEP = 0.2;
RUNSPEED = 1;
DISABLE_Z = true;
UWB_NODES = {...
	'1', [56.4; 38.6; 0]; ...
	'4', [67.6; 38.0; 0]; ...
	'5', [54.8; 44.2; 0]; ...
	'6', [56.4; 32.2; 0]; ...
	'7', [67.6; 32.2; 0]; ...
	'9', [62.5; 44.2; 0]; ...
	'10', [68.0; 44.2; 0] ...
};

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

assert(length(uwbMeasurements) > 0, 'Radio-Signal visualization only works when there are usb measurements!');
fprintf('BT-Advertisements: %d\nUWB-Advertisements: %d\n', rows(btAdvertisements), rows(uwbMeasurements));

visMeasurements = zeros(rows(btAdvertisements), 5);
visMeasurements(:, 1) = [btAdvertisements{:,1}];
uwbTimestamps = [uwbMeasurements{:,1}];
for btIdx = 1:rows(btAdvertisements)
	measurementTs = btAdvertisements{btIdx, 1};
	rssi = btAdvertisements{btIdx, 3};
	[~, uwbIdx] = min(abs(uwbTimestamps - measurementTs));
	visMeasurements(btIdx,2:end) = [[uwbMeasurements{uwbIdx,2:4}], rssi];
end


% initialize plot animation
currentTime = 0;
uwbNodePos = [UWB_NODES{:,2}]' * 1000;
figure();
sp = plot3([uwbNodePos(1,1)], [uwbNodePos(1,2)], [uwbNodePos(1,3)], 'Marker', 'x');
grid on;
xlim([ min([visMeasurements(:, 2); uwbNodePos(:,1)]), max([visMeasurements(:, 2); uwbNodePos(:,1)]) ]);
ylim([ min([visMeasurements(:, 3); uwbNodePos(:,2)]), max([visMeasurements(:, 3); uwbNodePos(:,2)]) ]);
zlim([ min([visMeasurements(:, 4); uwbNodePos(:,3)]), max([visMeasurements(:, 4); uwbNodePos(:,3)]) ]);
hold on;
plot3(uwbNodePos(:,1), uwbNodePos(:,2), uwbNodePos(:,3), 'o', 'MarkerFaceColor', [1,0,0]);
text(uwbNodePos(:,1), uwbNodePos(:,2), uwbNodePos(:,3), UWB_NODES(:,1), 'VerticalAlignment', 'bottom', 'HorizontalAlignment', 'right');
hold off;

pause;

while currentTime < timestamps(end)
	measIdcs = (visMeasurements(:, 1) <= currentTime);
	set(sp, 'xdata', visMeasurements(measIdcs, 2));
	set(sp, 'ydata', visMeasurements(measIdcs, 3));
	if(DISABLE_Z)
		set(sp, 'zdata', repmat(min(visMeasurements(:, 4)), sum(measIdcs), 1));
	else
		set(sp, 'zdata', visMeasurements(measIdcs, 4));
	end
	drawnow;
	pause(TIMESTEP);
	currentTime += TIMESTEP * RUNSPEED;
end