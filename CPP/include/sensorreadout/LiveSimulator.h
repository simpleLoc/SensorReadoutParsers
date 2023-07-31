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
		Callback callback;

		// simulation state
		std::chrono::steady_clock::time_point simulationStartTime;

	public:
		LiveSimulator() {}
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
				simulationStartTime = std::chrono::steady_clock::now();
				for(size_t i = 0; i < evts.size(); ++i) {
					if(shouldExit) { return; }
					const auto& nextEvt = evts[i];
					auto nextEvtTime = simulationStartTime + std::chrono::nanoseconds(nextEvt.timestamp);
					std::this_thread::sleep_until(nextEvtTime);
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
		Timestamp runningTimeNs() const {
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - simulationStartTime).count();
		}

	};

}
