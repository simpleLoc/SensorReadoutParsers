classdef SensorReadoutParser < handle
	
	properties(Access = private, Constant)
		TIMESTAMP_MULTIPLIER = 1 / (1000 * 1000 * 1000); % nanoseconds to seconds
	end
	
	properties(Access = private)
		csvFiles = 0;
		sensorTypeIdWhitelist = [];
		disableAsserts = false;
    end
	
	methods
		function self = SensorReadoutParser(disableAsserts, sensorTypeIdWhitelist)
			if ~exist('sensorTypeIdWhitelist', 'var')
				sensorTypeIdWhitelist = SensorType.SENSOR_START:SensorType.SENSOR_END;
			end
			if exist('disableAsserts', 'var')
				assert(islogical(disableAsserts), 'disableAsserts has to be a boolean.');
				self.disableAsserts = disableAsserts;
			end
			self.sensorTypeIdWhitelist = sensorTypeIdWhitelist;
			assert(numel(self.sensorTypeIdWhitelist) > 0, 'sensorTypeIdWhitelist is empty. No channels will be imported.');
		end
	end
	
	% Parser methods
	methods
		function dataContainer = parseSensorData(self, fileName)
			dataContainer = TimestampedRecordingContainer(fileName);
			activityChanges = [];
			activityChangeTimestamps = [];
			
			[rawInputData, timestamps, evtIds] = self.parseFile(fileName);
			[timestampVariance, tsVarIdx] = max(diff(timestamps));
			assert(self.disableAsserts || timestampVariance < 0.3, ... %0.3 is already brutally bad - but it's necessary, unfortunately :(
				sprintf('[%s] Variance of timestamp-distances is high: [%.2f @ line %d]. Android may have stopped the App in between.\n',...
				fileName, timestampVariance, tsVarIdx));
			
			data = zeros(size(timestamps, 1), 9, 'double');
			
			for sensorId = SensorType.getBaseSensors()
				if sensorId == SensorType.PEDESTRIAN_ACTIVITY
					% Parse activity-markers manually, because there are
					% strings in the csv, and matlab doesn't like that.
					for i = find(evtIds == sensorId)'
						parts = strsplit(rawInputData{3}{i}, ';');
						activityId = uint32(str2double(parts{2}));
						activityChanges(end+1) = activityId;
						activityChangeTimestamps(end+1) = timestamps(i);
						data(i,1) = activityId;
					end
				else
					accessIdxs = (evtIds == sensorId);
					sensorDims = SensorType.getSensorEventArgumentCnt(sensorId);

					% Take value-strings from rawInputData and append to one
					rawSensorData = strjoin({rawInputData{3}{accessIdxs}}, newline);

					% Parse values from appended value-string for this sensor,
					% and store in corresponding data-rows
					scanStr = strjoin(repmat({'%f'}, 1, sensorDims), ';');
					data(accessIdxs, 1:sensorDims) = sscanf(rawSensorData, scanStr, [sensorDims, Inf])';
				end
			end
			
			% bah... found recordings without activity data.. (trousers
			% recordings...)
			assert(self.disableAsserts || length(activityChanges) > 1, 'Probably defect recording without activity data!');
			assert(self.disableAsserts || ~any(diff(activityChanges) == 0), 'Two consecutive activity-tags of same type.');
			assert(self.disableAsserts || min(diff([activityChangeTimestamps(2:end), Inf])) > 0.46, 'Extremely short activity sequence detected.');
			
			dataContainer.setActivityChanges(activityChangeTimestamps, activityChanges);
			
			sensorIds = unique(evtIds);
			% filter for sensors with a fixed length of parameters
			sensorIds = sensorIds(ismember(sensorIds, SensorType.BASE_SENSOR_LIST()));
			
			% apply sensorTypeIdWhitelist
			sensorIds = sensorIds(ismember(sensorIds, self.sensorTypeIdWhitelist))';
			assert(numel(sensorIds) > 0, 'Amount of exported sensors is 0. Recording does not contain any whitelisted sensors.');
			
			for sensorId = sensorIds
				accessIdxs = (evtIds == sensorId);
				dimCnt = SensorType.getSensorEventArgumentCnt(sensorId);
				dataContainer.addChannel(sensorId, timestamps(accessIdxs), data(accessIdxs, 1:dimCnt));
			end
		end
		
		% Parse bluetooth and wifi advertisement events
		function [btAdvertisements, wifiAdvertisements] = parseRadio(self, fileName)
			[rawInputData, timestamps, evtIds] = self.parseFile(fileName);
			
			btIdxs = (evtIds == SensorType.IBEACON);
			rawBtData = rawInputData{3}(btIdxs);
			wifiIdxs = (evtIds == SensorType.WIFI);
			rawWifiData = rawInputData{3}(wifiIdxs);
			
			% allocate result structures and populate timestamps
			btAdvertisements = cell(sum(btIdxs), 4);
			btAdvertisements(:,1) = num2cell(timestamps(btIdxs));
			wifiAdvertisements = cell(sum(wifiIdxs), 4);
			wifiAdvertisements(:,1) = num2cell(timestamps(wifiIdxs));
			
			for i = 1:length(rawBtData)
				btAdvertisements(i, 2:end) = textscan(rawBtData{i}, '%s %d %d', 'Delimiter', ';');
			end
			for i = 1:length(rawWifiData)
				wifiAdvertisements(i, 2:end) = textscan(rawWifiData{i}, '%s %d %d', 'Delimiter', ';');
			end
		end
	end
	
	methods(Access = private)
		function [rawInputData, timestamps, evtIds] = parseFile(self, fileName)
			% parse activity ids / labels
			fid = fopen(fileName, 'r');
			if fid == -1
				error(["Failed to open file: ", fileName]);
			end
			rawInputData = textscan(fid, '%f;%d;%s', 'Delimiter', '\n');
			fclose(fid);
			
			timestamps = rawInputData{1} * SensorReadoutParser.TIMESTAMP_MULTIPLIER;
			if ~issorted(timestamps)
				if self.disableAsserts
					[timestamps, sortIdxs] = sort(timestamps);
					rawInputData = cellfun(@(c) c(sortIdxs), rawInputData, 'UniformOutput', false);
				else
					error(sprintf('[%s] Timestamps of file are not in correct order.\n', fileName));
				end
			end
			timestamps = timestamps - timestamps(1);
			evtIds = rawInputData{2};
		end
	end
end
