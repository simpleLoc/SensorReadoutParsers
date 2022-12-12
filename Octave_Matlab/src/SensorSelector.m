classdef SensorSelector < handle

	properties(Access = protected)
		sensors = [];
		sensorsChannelCnt = [];
		channelCnt = 0;
		virtualSensors = {};
	end

	methods
		function result = containsSensor(self, sensorTypeId)
			result = sum(ismember(self.sensors, sensorTypeId)) > 0;
		end

		function self = useSensor(self, sensorTypeId)
			self.addSensor(sensorTypeId, SensorType.getSensorEventArgumentCnt(sensorTypeId));
		end

		function self = useVirtualSensor(self, virtualSensor)
			assert(isa(virtualSensor, 'VirtualSensorBase'), 'Virtual sensors have to be a subclass of VirtualSensorBase');
			% add to sensor-List and count sensorChannels
			self.virtualSensors(end+1) = virtualSensor;
			self.channelCnt += virtualSensor.getChannelCnt();
		end

		function self = useVirtualSensors(self, virtualSensors)
			for vS = 1:length(virtualSensors)
				self.useVirtualSensor(virtualSensors{vS});
			end
		end

		function channelCnt = getTotalChannelCnt(self)
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
		function virtualSensors = getVirtualSensors(self)
			virtualSensors = self.virtualSensors;
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

