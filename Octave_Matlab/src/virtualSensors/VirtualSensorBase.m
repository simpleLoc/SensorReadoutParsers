classdef VirtualSensorBase < handle

	properties(Access = protected)
		channelCnt = -1;
	end

	methods
		function self = VirtualSensorBase(channelCnt, requiredSensors)
			self.channelCnt = channelCnt;
		end

		function channelCnt = getChannelCnt(self)
			channelCnt = self.channelCnt;
		end
	end

	methods(Abstract)
		% featureStream = calculate(self, dataContainer)
    end

end

