% timedResample  Take a sequence of timed values, and resample it with the given newSampleInterval, starting from startTs.
%   [output, tsOffset] = timedResample(dataIn, dataTimestamps, newSampleInterval, startTs) Resample dataIn with newSampleInterval, starting at startTs.
%
% Arguments:
% - dataIn: the (optionally) multidimensional timed signal-values. One row per timed sample.
% - dataTimestamps: Timestamps for the signal in dataIn. One time-value per timed sample(=row). In time-units.
% - startTs: Timestamp at which the resulting signal starts, in time-units. (has to be >= first data timestamp).
% - newSampleInterval: Amount of time-units (as in dataTimestamps) between two samples.
%
% Results:
% - dataOut: Resampled signal from dataIn, starting at startTs, resampled to have one sample each newSampleInterval time-units.
%            Resampling will stop when the last sample in the timed signal dataIn is reached.
% - newTimestamps: Timestamps of this data, First value is startTs; incremented by newSampleInterval each sample.
function [dataOut, newTimestamps] = timedResample(dataIn, dataTimestamps, startTs, newSampleInterval)
	[tsOffset, sampleInterval] = timedSignalInfo(dataIn, dataTimestamps);
	assert(numel(newSampleInterval) == 1, 'newSampleInterval has to be one number');
	assert(numel(startTs) == 1, 'newSampleInterval has to be one number');
	assert(tsOffset <= startTs, 'Start-Timestamp has to be >= tsOffset in given signal.');
	
	firstSampleIdx = find(dataTimestamps >= startTs, 1);
	newSampleCnt = uint64((dataTimestamps(end) - startTs) / newSampleInterval);
	dataOut = zeros(newSampleCnt, size(dataIn, 2));
	newTimestamps = cumsum([startTs; repmat(newSampleInterval,newSampleCnt-1,1)]);
	
	jB = 1;
	for i = 1:size(newTimestamps, 1)
		cTimestamp = newTimestamps(i); % Timestamp to interpolate in this iteration
		while(dataTimestamps(jB+1) < cTimestamp)
			jB = jB + 1;
		end
		if dataTimestamps(jB) == cTimestamp
			dataOut(i,:) = dataIn(jB,:);
		else
			jA = jB+1;
			cBeforeTimestamp = dataTimestamps(jB);
			cAfterTimestamp = dataTimestamps(jA);
			alpha = (cTimestamp - cBeforeTimestamp) / (cAfterTimestamp - cBeforeTimestamp);
			dataOut(i,:) = (1.0-alpha) * dataIn(jB,:) + alpha * dataIn(jA,:);
		end
	end
end
