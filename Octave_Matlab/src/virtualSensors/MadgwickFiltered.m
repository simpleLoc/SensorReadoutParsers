classdef MadgwickFiltered < VirtualSensorBase

	properties(Access = private)
		inputSensorId = -1;
		beta = -1;
	end

	methods
		function self = MadgwickFiltered(inputSensorId, beta)
			self.inputSensorId = inputSensorId;
			self.beta = beta;
			channelCnt = SensorType.getSensorEventArgumentCnt(inputSensorId);
			self@VirtualSensorBase(channelCnt);
		end

		function featureStream = calculate(self, dataContainer)
			sampleFreq = 1.0 / dataContainer.getSampleInterval();
			accelSensor = dataContainer.getSensorStream(SensorType.ACCELEROMETER);
			gyroSensor = dataContainer.getSensorStream(SensorType.GYROSCOPE);
			featureStream = dataContainer.getSensorStream(self.inputSensorId);

			quats = zeros(4, columns(accelSensor));

			madgwick = Madgwick(sampleFreq, self.beta);
			for i = 1:columns(accelSensor)
				quats(:, i) = madgwick.push(accelSensor(:,i), gyroSensor(:,i));
			end

			quaternions = quaternion(quats(1,:), quats(2,:), quats(3,:), quats(4,:));
			featureStream = rotatepoint(quaternions, featureStream')';
		end
	end

end

