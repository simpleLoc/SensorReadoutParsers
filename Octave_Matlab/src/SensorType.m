classdef SensorType
	
	properties (Access = private, Constant)
		% start with FILE_METADATA(=-2) => right shift by +3
		argCntLookup = [-1, ...	% FILE_METADATA
						2, ...	% GROUND_TRUTH_PATH
						3, ...	% ACCELEROMETER
						3, ...	% GRAVITY
						3, ...	% LINEAR_ACCELERATION
						3, ...	% GYROSCOPE
						3, ...	% MAGNETIC_FIELD
						1, ...	% PRESSURE
						3, ...	% ORIENTATION_NEW
						9, ...	% ROTATION_MATRIX
						-1, ... % WIFI
						-1, ...	% IBEACON
						-1, ...	% RELATIVE_HUMIDITY
						3, ...	% ORIENTATION_OLD
						4, ...	% ROTATION_VECTOR
						1, ...	% LIGHT
						1, ...	% AMBIENT_TEMPERATURE
						1, ...	% HEART_RATE
						-1, ...	% GPS
						-1, ...	% WIFIRTT
						3,	...	% GAME_ROTATION_VECTOR
						-1, ... % EDDISTONE_UUID
						-1, ... % DECAWAVE_UWB
						zeros(1, 29), ...
						1, ...	% PEDESTRIAN_ACTIVITY
						zeros(1, 48), ...
						1];		% GROUND_TRUTH
	end
	
	properties (Constant)
		SENSOR_START = 0; % first sensor-id
		%%%%%%%%%%%%%%%%%%%
		ACCELEROMETER = 0,
		GRAVITY = 1,
		LINEAR_ACCELERATION = 2,
		GYROSCOPE = 3,
		MAGNETIC_FIELD = 4,
		PRESSURE = 5,
		ORIENTATION_NEW = 6,
		ROTATION_MATRIX = 7,
		WIFI = 8,
		IBEACON = 9,
		RELATIVE_HUMIDITY = 10,
		ORIENTATION_OLD = 11,
		ROTATION_VECTOR = 12,
		LIGHT = 13,
		AMBIENT_TEMPERATURE = 14,
		HEART_RATE = 15,
		GPS = 16,
		WIFIRTT = 17,
		GAME_ROTATION_VECTOR = 18,
		EDDISTONE_UUID = 19,
		DECAWAVE_UWB = 20,
		%%%%%%%%%%%%%%%%%%
		SENSOR_END = 20, % last sensor-id
		
		
		PEDESTRIAN_ACTIVITY = 50,
		GROUND_TRUTH = 99,
		GROUND_TRUTH_PATH = -1,
		FILE_METADATA = -2
	end
	
	% COMPUTED PROPERTIES (for octave compatibility)
	methods (Access = public, Static)
		function result = BASE_SENSOR_LIST()
			persistent value;
			if isempty(value)
				value = find(SensorType.argCntLookup > 0) - 3;
			end
			result = value;
		end
	end
	
	
	
	methods (Access = public, Static)		
		function result = isBaseSensor(sensorTypeId)
			assert(isnumeric(sensorTypeId), 'SensorId has to be a positive integer');
			result = (sensorTypeId >= SensorType.SENSOR_START && sensorTypeId <= SensorType.SENSOR_END);
		end

		function baseSensors = getBaseSensors()
			baseSensors = SensorType.BASE_SENSOR_LIST;
		end
				
		function argCnt = getSensorEventArgumentCnt(sensorTypeId)
			argCnt = SensorType.argCntLookup(sensorTypeId + 3);
		end
	end
end
