#pragma once

#include <clocale>

namespace SensorReadoutParser {

	class LocaleHandle {
	private:
		locale_t hndl = nullptr;
	public:
		LocaleHandle(locale_t hndl) : hndl(hndl) {}
		~LocaleHandle() {
			if(hndl != nullptr) { ::freelocale(hndl); }
		}

		locale_t use() const { return ::uselocale(hndl); }

		static LocaleHandle from(int mask, const char* locale) {
			return LocaleHandle( ::newlocale(mask, locale, nullptr) );
		}
	};

	class ParseLocaleContext {
	private:
		static const LocaleHandle PARSE_LOCALE;
		locale_t prevLocale;
	public:
		ParseLocaleContext() {  prevLocale = PARSE_LOCALE.use(); }
		~ParseLocaleContext() { ::uselocale(prevLocale); }
	};

}
