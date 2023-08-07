#include <sensorreadout/ParseLocaleContext.h>

namespace SensorReadoutParser {

	const LocaleHandle ParseLocaleContext::PARSE_LOCALE = LocaleHandle::from(LC_ALL_MASK, "C");

}
