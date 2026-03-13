#pragma once

#ifdef __valid
#pragma push_macro("__valid")
#undef __valid
#define PERFORMANCE_TIMER_RESTORE_VALID_MACRO
#endif
#include <chrono>
#ifdef PERFORMANCE_TIMER_RESTORE_VALID_MACRO
#pragma pop_macro("__valid")
#undef PERFORMANCE_TIMER_RESTORE_VALID_MACRO
#endif
#include <string>

class PerformanceTimer
{
private:
	std::chrono::steady_clock::time_point m_startTime;

public:
	PerformanceTimer();
	void Reset();
	void PrintElapsedTime(const std::wstring &description);
};
