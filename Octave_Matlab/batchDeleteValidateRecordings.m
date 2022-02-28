clear all;
close all;
loadSensorReadoutParser('src/');

recFileFolder = uigetdir('Record-File Folder');
assert(ischar(recFileFolder), 'Did not select any folder');
result = inputdlg('Warning. This is going to delete record files!!! Continue?', 'WARNING', [1], {'no'});

if(strcmp(result{1}, 'yes') == 1)
	fileList = dir([recFileFolder '/*.csv']);
	for(recFileIdx = [1:length(fileList)])
		recFile = fileList(recFileIdx);
		recFilePath = [recFile.folder '/' recFile.name];
		printf('Validating File: %s\n', recFilePath);
		% #########################################################################
		% # Parse & Validate
		% #########################################################################
		parser = SensorReadoutParser(recFilePath, true);
		
		try
			[errCnt, warnCnt] = parser.validateRecording();
		catch e
			printf('Recording caused assertion\n');
			continue;
		end
		
		if(errCnt == 0)
			delete(recFilePath)
		end
	end
end