classdef Magnitude < VirtualSensorBase

	properties(Access = private)
		inputSensorId = -1;
	end

	methods
		function self = Magnitude(inputSensorId)
			self.inputSensorId = inputSensorId;
			self@VirtualSensorBase(1);
		end

		function featureStream = calculate(self, dataContainer)
			sampleFreq = 1.0 / dataContainer.getSampleInterval();
			featureStream = dataContainer.getSensorStream(self.inputSensorId);

			featureStream = vecnorm(featureStream);
		end
	end

end
