#include <sensorreadout/Tokenizer.h>

#include <charconv>

namespace SensorReadoutParser {

	#define IMPLEMENT_FROM_STRINGVIEW_NUMERIC(NumberType) \
	template<> NumberType fromStringView(const std::string_view& str) { \
			NumberType result; \
			auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result); \
			exceptAssert(ec == std::errc(), "Failed to parse token to value" + std::string(str)); \
			return result; \
	}

	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(uint8_t);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(int8_t);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(uint16_t);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(int16_t);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(uint32_t);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(int32_t);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(uint64_t);
	IMPLEMENT_FROM_STRINGVIEW_NUMERIC(int64_t);

	#ifndef ANDROID
		IMPLEMENT_FROM_STRINGVIEW_NUMERIC(float);
		IMPLEMENT_FROM_STRINGVIEW_NUMERIC(double);
	#else // NDK26 will have from_chars, everything before... is missing from_chars<float> and from_chars<double>
		template<> float fromStringView(const std::string_view& strView) {
			float result; size_t endIdx = 0;
			std::string str{strView};
			result = std::stof(str, &endIdx);
			exceptAssert(endIdx == strView.size(), "Failed to parse token to float");
			return result;
		}
		template<> double fromStringView(const std::string_view& strView) {
			double result; size_t endIdx = 0;
			std::string str{strView};
			result = std::stod(str, &endIdx);
			exceptAssert(endIdx == strView.size(), "Failed to parse token to float");
			return result;
		}
	#endif

	template<> bool fromStringView<bool>(const std::string_view& str) {
		uint8_t data = fromStringView<uint8_t>(str);
		return (data != 0);
	}

	template<> std::vector<float> fromStringView<std::vector<float>>(const std::string_view& str) {
		std::vector<float> result;
		if (str.size() >= 2 && str.front() == '[' && str.back() == ']') {
			auto content = str.substr(1, str.size() - 2);
			std::string_view::size_type start = 0;
			std::string_view::size_type end;
			while ((end = content.find(',', start)) != std::string_view::npos) {
				std::string_view token = content.substr(start, end - start);
				result.push_back(std::strtof(token.data(), nullptr));
				start = end + 1;
			}
			if (start < content.size()) {
				std::string_view token = content.substr(start);
				result.push_back(std::strtof(token.data(), nullptr));
			}
		}
		return result;
	}

}
