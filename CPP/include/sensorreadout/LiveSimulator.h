#pragma once

#include <thread>
#include <chrono>
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
				auto startTime = std::chrono::steady_clock::now();
				for(size_t i = 0; i < evts.size(); ++i) {
					if(shouldExit) { return; }
					const auto& nextEvt = evts[i];
					auto nextEvtTime = startTime + std::chrono::nanoseconds(nextEvt.timestamp);
					std::this_thread::sleep_until(nextEvtTime);
					callback(nextEvt);
				}
			});
		}

		void wait() {
			simulationThread.join();
		}

		void stop() {
			shouldExit.store(true);
			wait();
		}

	};

}
