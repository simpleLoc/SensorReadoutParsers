% NormalizedAccelFeature - This feature calculates an orientation
% independent (2-channel) version of the acceleration data using the
% approach from: " Physical activity recognition with mobile phones: challenges, methods, and applications"
% This approach calculates one horizontal and one vertical channel. The
% vertical channel contains the acceleration along the gravitation-axis
% within the data, and the horizontal channel contains the magnitude of the
% data in the plane perpendicular to that gravitation vector. This makes
% the approach very orientation independent.
classdef NormalizedAccelFeature < VirtualSensorBase
	methods(Access = private, Static)
		function [filter, filterHalfSize] = getLowpassFilter(sampleInterval, cutoffFreq)
			filterHalfSize = 29;
			FILTER_LENGTH = 2 * filterHalfSize + 1;

			filter = sinc(2 * cutoffFreq * sampleInterval * ([1:FILTER_LENGTH] - (FILTER_LENGTH - 1) / 2));
			filter = filter .* hamming(FILTER_LENGTH)';
			filter = filter / sum(filter);
		end
	end
	
	properties(Access = private)
		filterHz = false;
		filterOrder = false;
	end

	methods
		function self = NormalizedAccelFeature()
			if isOctave()
				pkg load quaternion;
			end
			
			isPosNonNegInteger = @(v) isscalar(v) && floor(v) == v && isfinite(v) && v > 0;
			p = inputParser();
			p.PartialMatching = false;
			p.addParameter('filterHz', false, isPosNonNegInteger);
			p.addParameter('filterOrder', false, isPosNonNegInteger);
			p.parse(varargin{:});
			opts = p.Results;
			
			self@VirtualSensorBase(1, [SensorType.ACCELEROMETER]);
			self.filterHz = opts.filterHz;
			self.filterOrder = opts.filterOrder;
		end

		function featureStream = calculate(self, dataContainer)
			accelStream = dataContainer.getSensorStream(SensorType.LINEAR_ACCELERATION);
			gravityStream = dataContainer.getSensorStream(SensorType.GRAVITY);
			featureStream = zeros(2, columns(accelStream));

			if self.filterHz ~= false
				[filter, filterHalfSize] = NormalizedAccelFeature.getLowpassFilter(dataContainer.getSampleInterval(), self.filterHz);
				for i = 1:rows(accelStream) % filter causes delay. shift by half filter-length
					tmp = conv(accelStream(i, :), filter);
					accelStream(i,:) = tmp(filterHalfSize:end-filterHalfSize-1);
				end
			elseif self.filterOrder ~= false
				accelStream = movmean(accelStream, self.filterOrder, 2);
			end
			
			% split up into horizontal / vertical
			gravityStream = gravityStream ./ vecnorm(gravityStream);
			% magnitude along gravity-axis (== world y-axis) within
			% accelerometer-stream is vertical part of this feature)
			featureStream(1,:) = dot(accelStream, gravityStream);
			% calculate horizontal part of this feature by removing the
			% vertical (3-dimensional) part from the signal, and then
			% calculating the magnitude on it (for
			% orientation-independence)
			featureStream(2,:) = vecnorm(accelStream - (featureStream(1,:) .* gravityStream));
		end
	end
end
