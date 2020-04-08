function loadSensorReadoutParser(srpBasepath)
	addpath(srpBasepath)
	if isOctave()
		addpath([srpBasepath, 'compat/octave']);
	else
		addpath([srpBasepath, 'compat/matlab']);
	end
end
