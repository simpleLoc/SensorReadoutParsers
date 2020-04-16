% Container class, taking in one recording of a dataset, consisting of multiple channels
% (sensors) of timestamped data. The container further contains timestamped
% class-changes, encoding which timed region contains which activity.
classdef TimestampedRecordingContainer < handle
	
	properties(Access = private)
		% Datastructure containing multiple timestamped channels of data
		% { <sensorId>, <timestamps>, <data, ...> }
		% { <sensorId>, <timestamps>, <data, ...> }
		%
		% data = [ N x M ] where M is the amount of dimensions, and N is
		% the amount of samples
		channels = {};
		% Datastructure encoding time-regions of activies
		% Each entry marks the beginning of an activity region.
		% { <timestamps>, <activityIds> }
		activityChanges = {};
		
		% metadata
		name = 0;
		
		channelIdxMap = containers.Map('KeyType', 'uint64', 'ValueType', 'uint64');
	end
	
	methods
		function self = TimestampedRecordingContainer(name)
			self.name = name;
		end
		
		function name = getName(self)
			name = self.name;
		end
		
		function addChannel(self, sensorId, timestamps, data)
			assert(~isempty(timestamps) && ~isempty(data), 'Timestamps or data is empty.');
			assert(size(timestamps, 1) == size(data, 1), 'Timestamps has to have as many entries as data');
			assert(issorted(timestamps), 'Timestamps have to be ascendingly sorted.');
			
			self.channels{end+1, 1} = sensorId;
			self.channels{end, 2} = timestamps;
			self.channels{end, 3} = data;
			self.channelIdxMap(sensorId) = size(self.channels, 1);
		end
		
		function setActivityChanges(self, timestamps, activityIds)
			self.activityChanges{1} = timestamps(:)';
			self.activityChanges{2} = uint32(activityIds(:)');
		end
		
		function result = hasSensorId(self, sensorId)
			result = self.channelIdxMap.isKey(sensorId);
		end
		
		% This is the finalizer method. This resamples the contained
		% timestamped recording into a uniform representation containing all
		% sensor-channels and returns it in a new RecordingContainer.
		function dataContainer = resampleIntoDataContainer(self, sampleInterval)
			dataContainer = RecordingContainer(sampleInterval, self.activityChanges);
			
			% resample channels and add to new RecordingContainer
			sampleCnt = Inf;
			startTs = self.getStartTimestamp();
			for ch = 1:size(self.channels, 1)
				[resampledSensorStream, resampledTimestamps] = timedResample(self.channels{ch, 3}, self.channels{ch, 2}, startTs, sampleInterval);
				sampleCnt = min(sampleCnt, size(resampledSensorStream, 1));
				dataContainer.addSensorStream(self.channels{ch, 1}, resampledSensorStream');
			end
			dataContainer.setDataTimestamps(resampledTimestamps(1:sampleCnt));
		end
	end
	
	methods(Access = private)
		function startTimestamp = getStartTimestamp(self)
			startTimestamps = zeros(size(self.channels, 1), 1);
			for c = 1:size(self.channels, 1)
				startTimestamps(c) = self.channels{c,2}(1); % get first timestamp for channel
			end
			startTimestamp = max(startTimestamps);
			assert(max(abs(startTimestamps - startTimestamp)) < 2.0, 'High variance in start timestamps detected. Slow sensor startup causing huge throw-away!');
		end
	end
end

