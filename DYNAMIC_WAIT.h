#pragma once

#include <Windows.h>

//
// Class for progressive waits
//
typedef struct
{
	UINT    WaitTime;
	UINT    WaitCount;
}WAIT_BAND;

#define WAIT_BAND_COUNT 3
#define WAIT_BAND_STOP 0

class DYNAMIC_WAIT
{
public:
	DYNAMIC_WAIT::DYNAMIC_WAIT() : m_CurrentWaitBandIdx(0), m_WaitCountInCurrentBand(0)
	{
		m_QPCValid = QueryPerformanceFrequency(&m_QPCFrequency);
		m_LastWakeUpTime.QuadPart = 0L;
	}
	DYNAMIC_WAIT::~DYNAMIC_WAIT()
	{
	}

	void DYNAMIC_WAIT::Wait()
	{
		LARGE_INTEGER CurrentQPC = { 0 };

		// Is this wait being called with the period that we consider it to be part of the same wait sequence
		QueryPerformanceCounter(&CurrentQPC);
		if (m_QPCValid && (CurrentQPC.QuadPart <= (m_LastWakeUpTime.QuadPart + (m_QPCFrequency.QuadPart * m_WaitSequenceTimeInSeconds))))
		{
			// We are still in the same wait sequence, lets check if we should move to the next band
			if ((m_WaitBands[m_CurrentWaitBandIdx].WaitCount != WAIT_BAND_STOP) && (m_WaitCountInCurrentBand > m_WaitBands[m_CurrentWaitBandIdx].WaitCount))
			{
				m_CurrentWaitBandIdx++;
				m_WaitCountInCurrentBand = 0;
			}
		}
		else
		{
			// Either we could not get the current time or we are starting a new wait sequence
			m_WaitCountInCurrentBand = 0;
			m_CurrentWaitBandIdx = 0;
		}

		// Sleep for the required period of time
		Sleep(m_WaitBands[m_CurrentWaitBandIdx].WaitTime);

		// Record the time we woke up so we can detect wait sequences
		QueryPerformanceCounter(&m_LastWakeUpTime);
		m_WaitCountInCurrentBand++;
	}

private:

	//static const WAIT_BAND   m_WaitBands[WAIT_BAND_COUNT];

	constexpr static WAIT_BAND DYNAMIC_WAIT::m_WaitBands[WAIT_BAND_COUNT] = {
																 {250, 20},
																 {2000, 60},
																 {5000, WAIT_BAND_STOP}   // Never move past this band
	};


	// Period in seconds that a new wait call is considered part of the same wait sequence
	static const UINT       m_WaitSequenceTimeInSeconds = 2;

	UINT                    m_CurrentWaitBandIdx;
	UINT                    m_WaitCountInCurrentBand;
	LARGE_INTEGER           m_QPCFrequency;
	LARGE_INTEGER           m_LastWakeUpTime;
	BOOL                    m_QPCValid;
};
