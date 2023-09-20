#pragma once

#include <thread>
#include <chrono>
#include <atomic>
#include <functional>

#include "SensorReadoutParser.h"

namespace SensorReadoutParser {

	// ###########
	// # LiveSimulator
	// ######################
	class LiveSimulator {
	public: // Associated types
		using SensorEventCallback = std::function<void(const SensorEvent&)>;
		using RecordingEndedCallback = std::function<void()>;

	private:
		std::vector<SensorEvent> evts;
		std::thread simulationThread;
		std::atomic<bool> shouldExit = false;
		double playbackSpeed;
		SensorEventCallback sensorEventCallback;
		RecordingEndedCallback recordingEndedCallback;

		// simulation state
		// the advantage of not using absolute timestamps is that pausing the debugger
		// will not make the application race on to catch up afterwards, and that it's
		// easier to change the playback speed
		std::atomic<Timestamp> runningTime = 0;

	public:
		/**
		 * @brief LiveSimulator ctor
		 * @param playbackSpeed Specifies a factor for the playback. (<= 0 means no artificial slowdown)
		 */
		LiveSimulator(double playbackSpeed = 1.0) : playbackSpeed(playbackSpeed) {}
		LiveSimulator(const std::vector<SensorEvent>& evts, SensorEventCallback callback) {
			setEvents(evts);
			setSensorEventCallback(callback);
		}

		void setEvents(const std::vector<SensorEvent>& evts) {
			this->evts = evts;
		}
		void setSensorEventCallback(SensorEventCallback callback) { this->sensorEventCallback = callback; }
		void setRecordingEndedCallback(RecordingEndedCallback callback) { this->recordingEndedCallback = callback; }

		void start() {
			shouldExit.store(false);
			simulationThread = std::thread([&]() {
				for(size_t i = 0; i < evts.size(); ++i) {
					if(shouldExit) { return; }
					const auto& nextEvt = evts[i];
					if (runningTime < nextEvt.timestamp && playbackSpeed > 0) {
						double sleepTimeNs = static_cast<double>(nextEvt.timestamp - runningTime) / playbackSpeed;
						std::this_thread::sleep_for(std::chrono::nanoseconds(static_cast<int64_t>(sleepTimeNs)));
					}
					runningTime = nextEvt.timestamp;
					if (sensorEventCallback) { this->sensorEventCallback(nextEvt); }
				}
				if (this->recordingEndedCallback) { this->recordingEndedCallback(); }
			});
		}

		void wait() {
			if(simulationThread.joinable()) {
				simulationThread.join();
			}
		}

		void stop() {
			shouldExit.store(true);
			wait();
		}

		/** amount of nanoseconds that passed since the recording simulation started */
		Timestamp runningTimeNs() const { return runningTime; }
	};

}
