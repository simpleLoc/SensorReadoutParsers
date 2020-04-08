classdef SensorSelector < handle

	properties(Access = protected)
		sensors = [];
		sensorsChannelCnt = [];
		channelCnt = 0;
	end
	
	methods
		function result = containsSensor(self, sensorTypeId)
			result = sum(ismember(self.sensors, sensorTypeId)) > 0;
		end
		
		function self = useSensor(self, sensorTypeId)
			self.addSensor(sensorTypeId, SensorType.getSensorEventArgumentCnt(sensorTypeId));
		end
		
		function channelCnt = getChannelCnt(self)
			channelCnt = self.channelCnt;
		end
		
		function sensorCnt = getSensorCnt(self)
			sensorCnt = length(self.sensors);
		end
		
		function sensors = getSensors(self)
			sensors = self.sensors;
		end
		function sensorsChannelCnt = getSensorChannelCnts(self)
			sensorsChannelCnt = self.sensorsChannelCnt;
		end
	end
	
	methods(Access = private)
		function addSensor(self, sensorTypeId, channelCnt)
			if ~self.containsSensor(sensorTypeId)
				self.sensors(end+1) = sensorTypeId;
				self.sensorsChannelCnt(end+1) = channelCnt;
			end
		end
	end
end

