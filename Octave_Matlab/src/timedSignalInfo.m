% timedSignalInfo - Analyse timed signal and return information
%
% Arguments:
% - dataIn: the (optionally) multidimensional timed signal-values. One row per timed sample.
% - dataTimestamps: Timestamps for the signal in dataIn. One time-value per timed sample(=row). In time-units.
%
% Results:
% - startTs: Timestamp (in time-units) of the first sample in the dataIn signal.
% - sampleInterval: Average amount of time-units (as in dataTimestamps) between two samples.
function [startTs, sampleInterval, sampleIntervalDev] = timedSignalInfo(dataIn, dataTimestamps)
	assert(rows(dataIn) == rows(dataTimestamps), 'dataIn and dataTimestamps have to have equal amount of rows');
	assert(columns(dataTimestamps) == 1, 'dataTimestamps has to be a vector (= 1 column)');
	
	startTs = dataTimestamps(1);
	sampleInterval = (dataTimestamps(end) - dataTimestamps(1)) / length(dataTimestamps);
	sampleIntervalDev = std(cumsum(dataTimestamps) - repmat(sampleInterval, rows(dataTimestamps), 1));
end
