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
		sorted = false;
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
			assert(self.disableAsserts || numel(sensorIds) > 0, 'Amount of exported sensors is 0. Recording does not contain any whitelisted sensors.');

			for sensorId = sensorIds
				accessIdxs = (self.evtIds == sensorId);
				dimCnt = SensorType.getSensorEventArgumentCnt(sensorId);
				dataContainer.addChannel(sensorId, self.timestamps(accessIdxs), data(accessIdxs, 1:dimCnt));
			end
		end

		function [stepEvts] = parseEventSensors(self)
			% PARSEEVENTSENSORS Parse event-based sensors such as the StepDetector
			self.ensureLoaded();

			stepIdxs = (self.evtIds == SensorType.STEP_DETECTOR);
			rawStepData = self.rawInputData{3}(stepIdxs);

			% allocate result structures and populate timestamps
			stepEvts = cell(rows(rawStepData), 2);
			stepEvts(:,1) = num2cell(self.timestamps(stepIdxs));

			% parse StepDetector
			rawStepData = textscan(strjoin(rawStepData, '\n'), '%f');
			stepEvts(:, 2) = num2cell(rawStepData{1});
		end

		function [btAdvertisements, wifiAdvertisements, ftmMeasurements, uwbMeasurements] = parseRadio(self)
			% PARSERADIO Parse radio-specific data from the recording (bluetooth & wifi advertisements)
			self.ensureLoaded();

			btIdxs = (self.evtIds == SensorType.IBEACON);
			rawBtData = self.rawInputData{3}(btIdxs);
			wifiIdxs = (self.evtIds == SensorType.WIFI);
			rawWifiData = self.rawInputData{3}(wifiIdxs);
			ftmIdxs = (self.evtIds == SensorType.WIFIRTT);
			rawFtmData = self.rawInputData{3}(ftmIdxs);
			uwbIdxs = (self.evtIds == SensorType.DECAWAVE_UWB);
			rawUwbData = self.rawInputData{3}(uwbIdxs);

			% allocate result structures and populate timestamps
			btAdvertisements = cell(rows(rawBtData), 4);
			btAdvertisements(:,1) = num2cell(self.timestamps(btIdxs));
			wifiAdvertisements = cell(rows(rawWifiData), 4);
			wifiAdvertisements(:,1) = num2cell(self.timestamps(wifiIdxs));
			% success, mac, distanceMM, stdDevDistance, rssi, numAttemptedMeas, numSuccessfulMeas
			ftmMeasurements = cell(rows(rawFtmData), 8);
			ftmMeasurements(:, 1) = num2cell(self.timestamps(ftmIdxs));
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

			% parse ftm
			rawFtmData = textscan(strjoin(rawFtmData, '\n'), '%u %s %f %f %d %d %d', 'Delimiter', ';');
			ftmMeasurements(:, 2) = num2cell([rawFtmData{1}]);
			ftmMeasurements(:, 3) = rawFtmData{2};
			ftmMeasurements(:, 4:end) = num2cell([rawFtmData{3:end}]);

			% parse uwb
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

		function [errorCnt, warnCnt] = validateRecording(self)
			errorCnt = 0;
			warnCnt = 0;
			self.ensureLoaded(false);
			uniqueSensorIds = unique(self.evtIds);
			sortedTimestamps = sort(self.timestamps);
			unsortedTimestampDistances = diff(self.timestamps);
			sortedTimestampDistances = diff(sortedTimestamps);

			% Validate timestamp variance
			[timestampVariance, tsVarIdx] = max(sortedTimestampDistances);
			if(timestampVariance > 0.3)
				errorCnt = errorCnt + 1;
				printf('ERR: Variance of timestamp-distances is high: [%.2f @ line %d]. Android may have stopped the App in between.\n', timestampVariance, tsVarIdx);
			end

			% Validate monotonic ordering of timestamps
			tsErrors = (unsortedTimestampDistances < 0);
			for(errEvtIdx = find(tsErrors))
				errorCnt = errorCnt + 1;
				printf('ERR(l: %d) Timestamp not monotonically increased from previous event.\n', errEvtIdx);
			end
			if(any(tsErrors))
				plot(sortedTimestampDistances);
				hold on;
				plot(unsortedTimestampDistances);
				legend('Sorted', 'Unsorted');
				hold off;
			end

			% Validate Amount of parameters for each event
			baseSensorAccess = SensorType.isBaseSensor(self.evtIds);
			baseSensorIdcs = [1:rows(self.evtIds)];
            baseSensorIdcs = baseSensorIdcs(baseSensorAccess);
			actualArgCnts = strfind(self.rawInputData{3}(baseSensorAccess), ';');
			actualArgCnts = cellfun(@(s) length(s), actualArgCnts)';
			shouldArgCnts = SensorType.getSensorEventArgumentCnt(self.evtIds(baseSensorAccess));
			argCntErrors = (actualArgCnts == shouldArgCnts);
			for argCntErrIdx = find(argCntErrors)
				errorCnt = errorCnt + 1;
				evtIdx = baseSensorIdcs(argCntErrIdx);
				printf('ERR(l: %d): Wrong argument-count for eventType %d. Should be: %d, but was: %d\n', evtIdx, self.evtIds(evtIdx), shouldArgCnts(evtIdx), actualArgCnts(evtIdx));
			end

			if(sum(argCntErrors) > 0)
				printf('#############################################################\n');
				printf('Not running content related checks because of previous errors\n');
				return;
			end
			%##############################
			% Content related errors
			%##############################

			try
				timestampedSensorData = self.parseSensorData();

				% validate GRAVITY
				if(ismember(SensorType.GRAVITY, uniqueSensorIds))
					[gravTs, gravData] = timestampedSensorData.getChannel(SensorType.GRAVITY);
					if(abs(mean(gravData(:)) - 9.8) <= 0.3)
						warnCnt = warnCnt + 1;
						printf('WARN: GRAVITY mean value seems suspicious\n');
					end
				end
			catch e
				warnCnt = warnCnt + 1;
				printf('WARN: Recording does not contain any base sensors\n');
			end

			[btAdvertisements, wifiAdvertisements, ftmMeasurements, uwbMeasurements] = self.parseRadio();

			% RSSI checks
			btRssiErrorIdcs = find([btAdvertisements{:,3}] > -30 | [btAdvertisements{:,3}] < -110);
			wifiRssiErrorIdcs = find([wifiAdvertisements{:,4}] > -20 | [wifiAdvertisements{:,4}] < -100);
			ftmRssiErrorIdcs = find([ftmMeasurements{:,6}] > -20 | [ftmMeasurements{:,6}] < -100);
			btIdcs = find(self.evtIds == SensorType.IBEACON);
			wifiIdcs = find(self.evtIds == SensorType.WIFI);
			ftmIdcs = find(self.evtIds == SensorType.WIFIRTT);
			for(errEvtIdx = btRssiErrorIdcs)
				warnCnt = warnCnt + 1;
				printf('WARN(l: %d): BLE Measurement has RSSI value (%f dB) outside expected range [-30,-100].\n', btIdcs(errEvtIdx), btAdvertisements{errEvtIdx, 3});
			end
			for(errEvtIdx = wifiRssiErrorIdcs)
				warnCnt = warnCnt + 1;
				printf('WARN(l: %d): Wifi Measurement has RSSI value (%f dB) outside expected range [-20,-100].\n', wifiIdcs(errEvtIdx), wifiAdvertisements{errEvtIdx, 4});
			end
			for(errEvtIdx = ftmRssiErrorIdcs)
				warnCnt = warnCnt + 1;
				printf('WARN(l: %d): FTM Measurement has RSSI value (%f dB) outside expected range [-20,-100].\n', ftmIdcs(errEvtIdx), ftmMeasurements{errEvtIdx, 6});
			end

			% last event timestamp check
			lastTimestamp = sortedTimestamps(end);
			if(rows(btAdvertisements) > 0 && abs(btAdvertisements{end,1} - lastTimestamp) > 20)
				warnCnt = warnCnt + 1;
				printf('WARN: Last Timestamp of BLE events is older than 20 secs. Has recording stopped?\n');
			end
			if(rows(wifiAdvertisements) > 0 && abs(wifiAdvertisements{end,1} - lastTimestamp) > 20)
				warnCnt = warnCnt + 1;
				printf('WARN: Last Timestamp of Wifi events is older than 20 secs. Has recording stopped?\n');
			end
			if(rows(ftmMeasurements) > 0 && abs(ftmMeasurements{end,1} - lastTimestamp) > 20)
				warnCnt = warnCnt + 1;
				printf('WARN: Last Timestamp of FTM events is older than 20 secs. Has recording stopped?\n');
			end
			if(rows(uwbMeasurements) > 0 && abs(uwbMeasurements{end,1} - lastTimestamp) > 20)
				warnCnt = warnCnt + 1;
				printf('WARN: Last Timestamp of UWB events is older than 20 secs. Has recording stopped?\n');
			end

			printf('#########################\nRadio Statistics:\n---------------------\n');
			printf('\tBLE: %d\n\tWifi: %d\n\tFTM: %d\n\tUWB: %d\n', rows(btAdvertisements), rows(wifiAdvertisements), rows(ftmMeasurements), rows(uwbMeasurements));
		end
	end

	methods(Access = private)
		function ensureLoaded(self, ensureSorted)
			if ~exist('ensureSorted', 'var')
				ensureSorted = true;
			end
			if(self.loaded == false)
				self.rawInputData = SensorReadoutParser.parseFile(self.fileName);
				self.syncFastAccessArrays();
				self.loaded = true;
			end
			if self.sorted == false && ensureSorted
				assert(self.disableAsserts || issorted(self.timestamps), 'Timestamps of recording not ordered');
				[self.timestamps, sortIdxs] = sort(self.timestamps);
				self.rawInputData = cellfun(@(c) c(sortIdxs), self.rawInputData, 'UniformOutput', false);
				self.syncFastAccessArrays();
			end
		end
		function syncFastAccessArrays(self)
			self.timestamps = self.rawInputData{1} * SensorReadoutParser.TIMESTAMP_MULTIPLIER;
			self.timestamps = self.timestamps - min(self.timestamps);
			self.evtIds = self.rawInputData{2};
		end
	end

	methods(Access = private, Static)
		function rawInputData = parseFile(fileName)
			% parse activity ids / labels
			fid = fopen(fileName, 'r');
			if fid == -1
				error(["Failed to open file: ", fileName]);
			end
			rawInputData = textscan(fid, '%f;%d;%s', 'Delimiter', '\n');
			fclose(fid);
		end
	end
end
