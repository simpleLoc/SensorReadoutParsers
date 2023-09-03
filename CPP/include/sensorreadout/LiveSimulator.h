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
		using Callback = std::function<void(const SensorEvent&)>;

	private:
		std::vector<SensorEvent> evts;
		std::thread simulationThread;
		std::atomic<bool> shouldExit = false;
		double playbackSpeed;
		Callback callback;

		// simulation state
		// the advantage of not using absolute timestamps is that pausing the debugger
		// will not make the application race on to catch up afterwards, and that it's
		// easier to change the playback speed
		Timestamp runningTime;

	public:
		LiveSimulator(double playbackSpeed = 1.0) : playbackSpeed(playbackSpeed) {}
		LiveSimulator(const std::vector<SensorEvent>& evts, Callback callback) {
			setEvents(evts);
			setCallback(callback);
		}

		void setEvents(const std::vector<SensorEvent>& evts) {
			this->evts = evts;
		}
		void setCallback(Callback callback) {
			this->callback = callback;
		}

		void start() {
			shouldExit.store(false);
			simulationThread = std::thread([&]() {
				for(size_t i = 0; i < evts.size(); ++i) {
					if(shouldExit) { return; }
					const auto& nextEvt = evts[i];
					if (runningTime < nextEvt.timestamp) {
						double sleepTimeNs = static_cast<double>(nextEvt.timestamp - runningTime) / playbackSpeed;
						std::this_thread::sleep_for(std::chrono::nanoseconds(static_cast<int64_t>(sleepTimeNs)));
					}
					runningTime = nextEvt.timestamp;
					callback(nextEvt);
				}
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
