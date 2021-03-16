classdef SensorReadoutParser < handle
	
	properties(Access = private, Constant)
		TIMESTAMP_MULTIPLIER = 1 / (1000 * 1000 * 1000); % nanoseconds to seconds
	end
	
	properties(Access = private)
		fileName = "";
		% parsing options
		sensorTypeIdWhitelist = [];
		disableAsserts = false;
		
		% internally cached parse results
		loaded = false;
		rawInputData = [];
		timestamps = [];
		evtIds = [];
    end
	
	methods
		function self = SensorReadoutParser(fileName, disableAsserts, sensorTypeIdWhitelist)
			assert(nargin > 0, "FILENAME argument required");
			self.fileName = fileName;
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
		function [date, person, comment] = getMetadata(self)
			self.ensureLoaded();
			metadataEntryIdx = find(self.rawInputData{2} == SensorType.FILE_METADATA);
			metadataStr = self.rawInputData{3}{metadataEntryIdx};
			metadataParts = strsplit(metadataStr, ';', 'CollapseDelimiters', false);
			date = metadataParts{1};
			person = metadataParts{2};
			comment = strjoin(metadataParts(3:end), ';');
		end
		
		function timestamps = getTimestamps(self)
			self.ensureLoaded();
			timestamps = self.timestamps;
		end
		
		function dataContainer = parseSensorData(self)
			dataContainer = TimestampedRecordingContainer(self.fileName);
			activityChanges = [];
			activityChangeTimestamps = [];
			
			self.ensureLoaded();
			[timestampVariance, tsVarIdx] = max(diff(self.timestamps));
			assert(self.disableAsserts || timestampVariance < 0.3, ... %0.3 is already brutally bad - but it's necessary, unfortunately :(
				sprintf('[%s] Variance of timestamp-distances is high: [%.2f @ line %d]. Android may have stopped the App in between.\n',...
				self.fileName, timestampVariance, tsVarIdx));
			
			data = zeros(size(self.timestamps, 1), 9, 'double');
			
			for sensorId = SensorType.getBaseSensors()
				if sensorId == SensorType.PEDESTRIAN_ACTIVITY
					% Parse activity-markers manually, because there are
					% strings in the csv, and matlab doesn't like that.
					for i = find(self.evtIds == sensorId)'
						parts = strsplit(self.rawInputData{3}{i}, ';');
						activityId = uint32(str2double(parts{2}));
						activityChanges(end+1) = activityId;
						activityChangeTimestamps(end+1) = self.timestamps(i);
						data(i,1) = activityId;
					end
				else
					accessIdxs = (self.evtIds == sensorId);
					sensorDims = SensorType.getSensorEventArgumentCnt(sensorId);

					% Take value-strings from rawInputData and append to one
					rawSensorData = strjoin({self.rawInputData{3}{accessIdxs}}, newline);

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
			
			sensorIds = unique(self.evtIds);
			% filter for sensors with a fixed length of parameters
			sensorIds = sensorIds(ismember(sensorIds, SensorType.BASE_SENSOR_LIST()));
			
			% apply sensorTypeIdWhitelist
			sensorIds = sensorIds(ismember(sensorIds, self.sensorTypeIdWhitelist))';
			assert(numel(sensorIds) > 0, 'Amount of exported sensors is 0. Recording does not contain any whitelisted sensors.');
			
			for sensorId = sensorIds
				accessIdxs = (self.evtIds == sensorId);
				dimCnt = SensorType.getSensorEventArgumentCnt(sensorId);
				dataContainer.addChannel(sensorId, self.timestamps(accessIdxs), data(accessIdxs, 1:dimCnt));
			end
		end
		
		function [btAdvertisements, wifiAdvertisements, uwbMeasurements] = parseRadio(self)
			% PARSERADIO Parse radio-specific data from the recording (bluetooth & wifi advertisements)
			self.ensureLoaded();
			
			btIdxs = (self.evtIds == SensorType.IBEACON);
			rawBtData = self.rawInputData{3}(btIdxs);
			wifiIdxs = (self.evtIds == SensorType.WIFI);
			rawWifiData = self.rawInputData{3}(wifiIdxs);
			uwbIdxs = (self.evtIds == SensorType.DECAWAVE_UWB);
			rawUwbData = self.rawInputData{3}(uwbIdxs);
			
			% allocate result structures and populate timestamps
			btAdvertisements = cell(rows(rawBtData), 4);
			btAdvertisements(:,1) = num2cell(self.timestamps(btIdxs));
			wifiAdvertisements = cell(rows(rawWifiData), 4);
			wifiAdvertisements(:,1) = num2cell(self.timestamps(wifiIdxs));
			% x,y,z,quality, [id,dist,qual]
			uwbMeasurements = cell(rows(rawUwbData), 6);
			uwbMeasurements(:,1) = num2cell(self.timestamps(uwbIdxs));
			
			% parse bluetooth
			rawBtData = textscan(strjoin(rawBtData, '\n'), '%s %d %d', 'Delimiter', ';');
			btAdvertisements(:, 2) = rawBtData{1};
			btAdvertisements(:, 3:4) = num2cell([rawBtData{2:3}]);

			% parse wifi - each wifi line has multiple advertisements, thus we need
			% the loop here.
			for i = 1:length(rawWifiData)
				wifiAdvertisements(i, 2:end) = textscan(rawWifiData{i}, '%s %d %d', 'Delimiter', ';');
			end
			
			for i = 1:length(rawUwbData)
				header = sscanf(rawUwbData{i}, '%f;%f;%f;%d;%s')';
				uwbMeasurements(i, 2:5) = num2cell(header(1:4));
				distancesStr = char(header(5:end));
				distances = textscan(distancesStr, '%d %d %d', 'Delimiter', ';');
				uwbMeasurements(i, 6) = [distances{:}];
			end
		end
		
		function groundTruthPoints = parseGroundTruthPoints(self)
			% PARSEGROUNDTRUTHPOINTS Parse groundtruth events from the recording
			self.ensureLoaded();
			
			gtIdxs = (self.evtIds == SensorType.GROUND_TRUTH);
			gtData = self.rawInputData{3}(gtIdxs);
			
			groundTruthPoints = cell(sum(gtIdxs), 2);
			groundTruthPoints(:,1) = num2cell(self.timestamps(gtIdxs));
			groundTruthPoints(:,2) = cellfun(@(s) uint64(str2num(s)), gtData, 'UniformOutput', 0);
		end
	end
	
	methods(Access = private)
		function ensureLoaded(self)
			if(self.loaded == false)
				[self.rawInputData, self.timestamps, self.evtIds] = SensorReadoutParser.parseFile(self.fileName, self.disableAsserts);
				self.loaded = true;
			end
		end
	end
	
	methods(Access = private, Static)
		function [rawInputData, timestamps, evtIds] = parseFile(fileName, disableAsserts)
			% parse activity ids / labels
			fid = fopen(fileName, 'r');
			if fid == -1
				error(["Failed to open file: ", fileName]);
			end
			rawInputData = textscan(fid, '%f;%d;%s', 'Delimiter', '\n');
			fclose(fid);
			
			timestamps = rawInputData{1} * SensorReadoutParser.TIMESTAMP_MULTIPLIER;
			if ~issorted(timestamps)
				if disableAsserts
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
