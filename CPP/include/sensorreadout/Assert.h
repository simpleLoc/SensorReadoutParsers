#pragma once

namespace _internal {
	#define exceptUnreachable(exceptionStr) throw std::runtime_error(exceptionStr);
	#define exceptAssert(cond, exceptionStr) if(!(cond)) { throw std::runtime_error(exceptionStr); }
	#define exceptWhen(cond, exceptionStr) if((cond)) { throw std::runtime_error(exceptionStr); }
}
