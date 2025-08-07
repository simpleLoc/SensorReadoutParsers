#pragma once

#include <clocale>
#include <locale>
#include <string>
#include <optional>
#include <cstdint>
#include <vector>

#include "Assert.h"
#include "ParseLocaleContext.h"

namespace SensorReadoutParser {

	/** fromStringView() template that parses any arbitrary type from a std::string_view */
	template<typename TValue> TValue fromStringView([[maybe_unused]] const std::string_view& str) { return TValue::unimplemented_function(); }

	namespace _internal {
		static const char* PARSE_LOCALE = std::setlocale(LC_NUMERIC, "C");
	}

	template<const char SEPERATOR>
	class Tokenizer {
	private:
		std::string_view str;
		size_t ptr = 0;
		ParseLocaleContext localeCtx;

	public:
		/**
		 * @brief Tokenizer ctor
		 * @param str String to construct the Tokenizer on
		 * @details If the passed str is empty, this Tokenizer will go straight to being EOF
		 */
		Tokenizer(const std::string_view str) : str(str), ptr((str.length() == 0) ? 1 : 0) {}
		~Tokenizer() noexcept(false) {
			exceptAssert(ptr >= str.length(), "Remaining unparsed tokens. This is regarded as error.");
		}

		std::optional<std::string_view> peekNext() {
			std::string_view result;
			if(isEOS()) { return {}; }
			auto nextSepPtr = str.find(SEPERATOR, ptr);
			if(nextSepPtr == std::string::npos) { // reached EOS, no further tokens
				nextSepPtr = str.length();
			}
			result = str.substr(ptr, (nextSepPtr - ptr));
			return result;
		}
		std::string_view next() {
			std::string_view result;
			exceptAssert(!isEOS(), "Unexpected EOS");
			auto nextSepPtr = str.find(SEPERATOR, ptr);
			if(nextSepPtr == std::string::npos) { // reached EOS, no further tokens
				nextSepPtr = str.length();
			}
			result = str.substr(ptr, (nextSepPtr - ptr));
			ptr = nextSepPtr + 1;
			return result;
		}

		template<typename TValue, const char* SKIP_CTRL_CHARS = nullptr> TValue nextAs() {
			try {
				std::string_view nextValue = next();
				if constexpr(SKIP_CTRL_CHARS != nullptr) {
					// trim control characters
					auto startTrimPos = nextValue.find_first_not_of(SKIP_CTRL_CHARS);
					if(startTrimPos != nextValue.npos) {
						nextValue.remove_prefix(startTrimPos);
					}
					auto endTrimPos = nextValue.find_last_not_of(SKIP_CTRL_CHARS);
					if(endTrimPos != nextValue.npos) { // backtrim required
						nextValue.remove_suffix(nextValue.size() - endTrimPos);
					}
				}
				return fromStringView<TValue>(nextValue);
			} catch (std::runtime_error& e) {
				ptr = str.length();
				throw e;
			}
		}

		void skipNext() {
			exceptAssert(!isEOS(), "Unexpected EOS");
			auto nextSepPtr = str.find(SEPERATOR, ptr);
			if(nextSepPtr == std::string::npos) { // reached EOS, no further tokens
				nextSepPtr = str.length();
			}
			ptr = nextSepPtr + 1;
		}

		void skipRemaining() {
			ptr = str.length() + 1;
		}

		std::string_view remainder() {
			exceptAssert(!isEOS(), "Unexpected EOS");
			std::string_view result = str.substr(ptr);
			ptr = str.length() + 1;
			return result;
		}

		bool isEOS() const {
			return ptr > str.length();
		}
	};

	// declarations of fromStringView implementations/specializations
	template<> bool fromStringView(const std::string_view&);
	template<> uint8_t fromStringView(const std::string_view&);
	template<> int8_t fromStringView(const std::string_view&);
	template<> uint16_t fromStringView(const std::string_view&);
	template<> int16_t fromStringView(const std::string_view&);
	template<> uint32_t fromStringView(const std::string_view&);
	template<> int32_t fromStringView(const std::string_view&);
	template<> uint64_t fromStringView(const std::string_view&);
	template<> int64_t fromStringView(const std::string_view&);
	template<> float fromStringView(const std::string_view&);
	template<> double fromStringView(const std::string_view&);
	template<> std::vector<float> fromStringView(const std::string_view&);
}
