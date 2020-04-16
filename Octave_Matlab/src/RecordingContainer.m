classdef RecordingContainer < handle
	
	properties(Access = private)
		data = {};
		sampleInterval = 0;
		dataTimestamps = 0;
		sensorIdxMap = 0;
		
		% metadata
		datasetId = 0;
		name = 0;
		
		% Datastructure encoding time-regions of activies
		% Each entry marks the beginning of an activity region.
		% { <timestamps>, <activityIds> }
		activityChanges = {};
	end
	
	methods
		function self = RecordingContainer(sampleInterval, activityChanges)
			self.sampleInterval = sampleInterval;
			self.sensorIdxMap = containers.Map('KeyType', 'uint64', 'ValueType', 'uint64');
			self.activityChanges = activityChanges;
		end
		
		% GETTERS
		function sampleInterval = getSampleInterval(self)
			sampleInterval = self.sampleInterval;
		end
		function sampleFrequency = getSampleFrequency(self)
			sampleFrequency = round(1.0 / self.sampleInterval);
		end
		function activities = getContainedActivities(self)
			activities = unique([self.activityChanges{:, 2}]);
		end
		function activityChanges = getActivityChanges(self)
			activityChanges = self.activityChanges;
		end
		function datasetId = getDatasetId(self)
			datasetId = self.datasetId;
		end
		function name = getName(self)
			name = self.name;
		end
		
		% SETTERS
		function setDataTimestamps(self, dataTimestamps)
			self.dataTimestamps = dataTimestamps;
			% Take amount of samples in dataTimestamps to shorten the
			% contained sensor-data streams, to all have the same length.
			for s = 1:length(self.data)
				self.data{s} = self.data{s}(:, 1:length(dataTimestamps));
			end
		end
		function dataTimestamps = getDataTimestamps(self)
			dataTimestamps = self.dataTimestamps;
		end
		
		function sensorStream = getSensorStream(self, sensorTypeId)
			sensorStream = self.data{self.sensorIdxMap(uint64(sensorTypeId))};
		end
		
		function optSensorStream = tryGetSensorStream(self, sensorTypeId)
			optSensorStream = [];
			if self.hasSensorStream(sensorTypeId)
				optSensorStream = self.getSensorStream(sensorTypeId);
			end
		end
		
		function setMetadata(self, datasetId, name)
			self.datasetId = datasetId;
			self.name = name;
		end
		
		% Checks all elements in sensorTypeId for presence in the container
		% Returns a logic array presenting the result of each sensorTypeId.
		function result = hasSensorStream(self, sensorTypeId)
			result = self.sensorIdxMap.isKey(num2cell(sensorTypeId));
		end
		
		% Checks all elements in sensorTypeId for presence in the container
		% Returns true if all are contained, otherwise false.
		function result = hasAllSensorStreams(self, sensorTypeId)
			result = sum(self.hasSensorStream(sensorTypeId)) == numel(sensorTypeId);
		end
		
		function addSensorStream(self, sensorTypeId, sensorStream)
			assert(~self.sensorIdxMap.isKey(sensorTypeId), 'Sensor already exported in container.');
			self.data{end+1} = sensorStream;
			self.sensorIdxMap(uint64(sensorTypeId)) = length(self.data);
		end
	end
	
	
	% Data extraction methods
	methods
		function [recordingTimestamps, recordingDataMatrix] = getSensorData(self, sensorSelector)
			recordingTimestamps = self.getDataTimestamps();
			recordingDataMatrix = zeros(sensorSelector.getChannelCnt(), length(recordingTimestamps));
			
			% copy base sensors into recordingDataMatrix
			matrixRowOffset = 1;
			sensors = sensorSelector.getSensors();
			sensorChannelCnts = sensorSelector.getSensorChannelCnts();
			for s = 1:length(sensors)
				sensorId = sensors(s);
				sensorChannelCnt = sensorChannelCnts(s);
				
				recordingDataMatrix(matrixRowOffset:matrixRowOffset+sensorChannelCnt-1, :) = self.getSensorStream(sensorId);
				matrixRowOffset = matrixRowOffset + sensorChannelCnt;
			end
		end
	end
	
	
	% DEBUGGING METHODS
	methods(Access = public, Static)
		function plotCompareSensorStreams(sensorStream1, sensorStream2)
			dimensionCnt = size(sensorStream1, 1);
			assert(dimensionCnt == size(sensorStream2, 1), 'SensorStreams have to have same dimensionality');
			
			figure('name', 'Compare SensorStreams');
			for i = 1:dimensionCnt
				subplot(dimensionCnt,1,i);
				plot(sensorStream1(i,:));
				hold on;
				plot(sensorStream2(i,:));
				legend('SensorStream1', 'SensorStream2');
				hold off;
			end
		end
	end
	
	
		
	% LOAD / STORE
	methods
		% SERIALIZE / DESERIALIZE
		function s = saveobj(self)
			s.data = self.data;
			s.sampleInterval = self.sampleInterval;
			s.dataTimestamps = self.dataTimestamps;
			s.sensorIdxMap = {self.sensorIdxMap.keys(), self.sensorIdxMap.values()};
			s.activityChanges = self.activityChanges;
			s.datasetId = self.datasetId;
			s.name = self.name;
		end
	end
	
	methods(Static)
		function self = loadobj(s)
			self = RecordingContainer(s.sampleInterval, s.activityChanges);
			self.data = s.data;
			self.dataTimestamps = s.dataTimestamps;
			self.sensorIdxMap = containers.Map(s.sensorIdxMap{1}, s.sensorIdxMap{2});
			self.datasetId = s.datasetId;
			self.name = s.name;
		end
	end
end

