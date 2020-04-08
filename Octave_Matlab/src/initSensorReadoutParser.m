function initSensorReadoutParser()
	if isOctave()
		addpath('compat/octave');
	else
		addpath('compat/matlab');
	end
end
