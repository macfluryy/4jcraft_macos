#include "../Build/stdafx.h"
#include "PerformanceTimer.h"

PerformanceTimer::PerformanceTimer()
{
#if !defined (__linux__)
	// Get the frequency of the timer
	LARGE_INTEGER qwTicksPerSec;
	QueryPerformanceFrequency( &qwTicksPerSec );
	m_fSecsPerTick = 1.0f / (float)qwTicksPerSec.QuadPart;
	
	Reset();
#endif
}

void PerformanceTimer::Reset()
{
#if !defined (__linux__)
	QueryPerformanceCounter( &m_qwStartTime );
#endif
}

void PerformanceTimer::PrintElapsedTime(const wstring &description)
{
#if !defined (__linux__)
	LARGE_INTEGER qwNewTime, qwDeltaTime;

	QueryPerformanceCounter( &qwNewTime );

	qwDeltaTime.QuadPart = qwNewTime.QuadPart - m_qwStartTime.QuadPart;
	float fElapsedTime = m_fSecsPerTick * ((FLOAT)(qwDeltaTime.QuadPart));

	app.DebugPrintf("TIMER: %ls: Elapsed time %f\n", description.c_str(), fElapsedTime);
#endif
}
