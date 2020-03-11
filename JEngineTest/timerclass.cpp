///////////////////////////////////////////////////////////////////////////////
// Filename: timerclass.cpp
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "timerclass.h"


TimerClass::TimerClass()
{
}


TimerClass::TimerClass(const TimerClass& other)
{
}


TimerClass::~TimerClass()
{
}


bool TimerClass::Initialize()
{
	// Check to see if this system supports high performance timers.
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);
	if(m_frequency == 0)
	{
		return false;
	}

	perfCounterSeconds = 1.0 / (double)m_frequency;
	previousTime = m_frequency;

	// Find out how many times the frequency counter ticks every millisecond.
	m_ticksPerMs = (float)(m_frequency / 1000);

	QueryPerformanceCounter((LARGE_INTEGER*)&m_startTime);

	return true;
}


void TimerClass::Frame()
{
	INT64 currentTime;
	float timeDifference;


	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	timeDifference = (float)(currentTime - m_startTime);

	m_frameTime = timeDifference / m_ticksPerMs;

	//m_startTime = currentTime;

	///

	/*deltaTime*/m_frameTime = max((float)((currentTime - previousTime) * perfCounterSeconds), 0.0f);
	totalTime = (float)((currentTime - m_startTime) * perfCounterSeconds);
	
	previousTime = currentTime;

	return;
}


float TimerClass::GetTime()
{
	return m_frameTime;
}