#pragma once

#include <map>
#include <string>
#include <fstream>
#include <unordered_set>
#include <unordered_map>

#include "SensorReadoutParser.h"

namespace SensorReadoutParser {

	enum class FingerprintType {
		Point, Path
	};

	struct ParameterParseHelper {
		static std::array<double, 3> parseVec3(const std::string& val) {
			std::array<double, 3> result;
			exceptAssert(sscanf(val.data(), "(%lf;%lf;%lf)", &result[0], &result[1], &result[2]), "Failed to parse vec3");
			return result;
		}
		template<typename TItem, typename TItemParseFn>
		static std::vector<TItem> parseArray(const std::unordered_map<std::string, std::string>& parameters, const std::string arrayName, TItemParseFn parseFn) {
			size_t arrayLen = std::stoul(parameters.at(arrayName + "[]"));
			std::vector<TItem> result;
			for(size_t i = 0; i < arrayLen; ++i) {
				std::string itemStr = parameters.at(arrayName + "[" + std::to_string(i) + "]");
				result.push_back(parseFn(itemStr));
			}
			return result;
		}
	};

	struct Fingerprint {
		FingerprintType fpType;
		std::string name;
		std::unordered_map<std::string, std::string> parameters;
		std::vector<SensorEvent> evts;

		size_t getFloorIdx() const { return std::stoul(parameters.at("floorIdx")); }
		std::string getFloorName() const { return parameters.at("floorName"); }

		// point
		std::array<double, 3> getPosition() const {
			return ParameterParseHelper::parseVec3(parameters.at("position"));
		}

		// path
		std::vector<std::string> getPointNames() const {
			return ParameterParseHelper::parseArray<std::string>(parameters, "points", [](const auto& str) { return str; });
		}
		std::vector<std::array<double, 3>> getPositions() const {
			return ParameterParseHelper::parseArray<std::array<double, 3>>(parameters, "positions", &ParameterParseHelper::parseVec3);
		}
	};

	class Fingerprints {
	private:
		std::vector<Fingerprint> fingerprints;
		std::multimap<std::string, size_t> nameMappings;

	public:
		void add(Fingerprint&& fp) {
			size_t fpIdx = fingerprints.size();
			nameMappings.insert({fp.name, fpIdx});
			fingerprints.push_back(std::forward<Fingerprint>(fp));
		}

		std::optional<std::reference_wrapper<const Fingerprint>> getFirstByName(const std::string& name) const {
			auto it = nameMappings.find(name);
			if(it != nameMappings.end()) {
				size_t fpIdx = it->second;
				return fingerprints.at(fpIdx);
			}
			return {};
		}
		std::optional<std::reference_wrapper<Fingerprint>> getFirstByName(const std::string& name) {
			auto it = nameMappings.find(name);
			if(it != nameMappings.end()) {
				size_t fpIdx = it->second;
				return fingerprints.at(fpIdx);
			}
			return {};
		}

		auto begin() { return fingerprints.begin(); }
		auto end() { return fingerprints.end(); }
		auto begin() const { return fingerprints.begin(); }
		auto end() const { return fingerprints.end(); }
		size_t size() { return fingerprints.size(); }
	};


	// ###########
	// # FingerprintParser
	// ######################

	class FingerprintParser {

	public:
		using EventFilter = std::unordered_set<EventType>;

	private: // Parser state
		std::istream& stream;
		FileVersion fileVersion;

	public: // API-Surface
		FingerprintParser(std::istream& stream, FileVersion fileVersion = FileVersion::V1) : stream(stream), fileVersion(fileVersion) {}

		Fingerprints parse(std::optional<EventFilter> eventFilter = {}) {
			stream.seekg(0, std::ifstream::beg);
			Fingerprints result;
			std::string line;

			while(true) {
				if(!stream.good()) {
					if(stream.fail()) { throw std::runtime_error("An error occured while reading the SensorReadout file."); }
					break;
				}
				// find fp header
				while(line.empty() || line.at(0) != '[') {
					if(!nextLine(line)) { return result; }
				}

				// parse fingerprint
				Fingerprint currentFp;
				if(line == "[fingerprint:point]") { currentFp.fpType = FingerprintType::Point; }
				else if(line == "[fingerprint:path]") { currentFp.fpType = FingerprintType::Path; }
				else { throw std::runtime_error("Encountered unsupported fingerprint type"); }

				while(true) {
					if(!nextLine(line)) { throw std::runtime_error("Unexpected end of file"); }
					if(line.empty()) { break; }
					auto [propKey, propValue] = parseParameterLine(line);
					currentFp.parameters.insert({propKey, propValue});
				}
				// require "name" property to be present
				exceptAssert(currentFp.parameters.find("name") != currentFp.parameters.end(), "Fingerprint missing required 'name' property");
				currentFp.name = currentFp.parameters.at("name");
				// parse events

				VisitingParser parser(stream, fileVersion);
				RawSensorEvent rawEvt;
				SensorEvent parsedEvt;
				while(stream.peek() != '\n' && parser.nextLine(rawEvt)) {
					parsedEvt = SensorEvent::parse(rawEvt);
					if(eventFilter.has_value() && eventFilter->find(parsedEvt.eventType) == eventFilter->end()) { continue; }
					currentFp.evts.push_back(std::move(parsedEvt));
				}
				result.add(std::move(currentFp));
			}

			return result;
		}

	private:
		std::pair<std::string, std::string> parseParameterLine(const std::string& line) {
			auto propSepIdx = line.find('=');
			if(propSepIdx == std::string::npos) { throw std::runtime_error("Invalid property line, missing key,value separator '='"); }
			return {
				line.substr(0, propSepIdx),
				line.substr(propSepIdx + 1)
			};
		}
		bool nextLine(std::string& line) {
			if(!stream.good()) {
				if(stream.fail()) { throw std::runtime_error("An error occured while reading the Fingerprint file."); }
				return false;
			}
			if(!std::getline(stream, line)) { return false; }
			return true;
		}
	};

}
