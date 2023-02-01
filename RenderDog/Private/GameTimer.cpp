////////////////////////////////////////
//RenderDog <·,·>
//FileName: GameTimer.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "GameTimer.h"

#include <Windows.h>

namespace RenderDog
{
	GameTimer::GameTimer()
		:m_DeltaTimeInSeconds(0.0),
		m_bIsPaused(true),
		m_StartTimeInCounts(0),
		m_CurrTimeInCounts(0),
		m_PrevTimeInCounts(0),
		m_StopTimeInCounts(0),
		m_PausedTimeInCounts(0)
	{
		int64_t countsPerSecond = 0;
		QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSecond);

		m_SecondsPerCount = 1.0 / (double)countsPerSecond;
	}

	GameTimer::~GameTimer()
	{}

	void GameTimer::Reset()
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&m_StartTimeInCounts);

		//重置暂停时间
		m_StopTimeInCounts = 0;
		m_PausedTimeInCounts = 0;

		m_bIsPaused = false;
	}

	void GameTimer::Tick()
	{
		//如果计时器处于暂停状态，则将DeltaTime设置为0，
		//以使得UpdateScene函数中的操作停止
		if (m_bIsPaused)
		{
			m_DeltaTimeInSeconds = 0.0;
			return;
		}

		QueryPerformanceCounter((LARGE_INTEGER*)&m_CurrTimeInCounts);

		int64_t deltaTime = m_CurrTimeInCounts - m_PrevTimeInCounts;
		m_DeltaTimeInSeconds = (double)deltaTime * m_SecondsPerCount;
		//当处理器进入省电模式或者在多核处理时切换到其他处理器时，DeltaTime可能出现负值
		//在这里强制将其设置为非负值
		if (m_DeltaTimeInSeconds < 0.0)
		{
			m_DeltaTimeInSeconds = 0.0;
		}

		//使用PrevTime记录当前时间，在下一帧中使用
		m_PrevTimeInCounts = m_CurrTimeInCounts;
	}

	void GameTimer::Start()
	{
		if (m_bIsPaused)
		{
			int64_t StartTime = 0;
			QueryPerformanceCounter((LARGE_INTEGER*)&StartTime);

			//累计更新PausedTime
			m_PausedTimeInCounts += (StartTime - m_StopTimeInCounts);
			m_PrevTimeInCounts = StartTime;

			m_StopTimeInCounts = 0;

			m_bIsPaused = false;
		}
	}

	void GameTimer::Pause()
	{
		if (!m_bIsPaused)
		{
			QueryPerformanceCounter((LARGE_INTEGER*)&m_StopTimeInCounts);
			m_bIsPaused = true;
		}
	}

	double GameTimer::GetDeltaTime() const
	{
		return m_DeltaTimeInSeconds;
	}

	double GameTimer::GetTotalTime() const
	{
		int64_t totalTime = 0;
		if (m_bIsPaused)
		{
			//这里的PausedTime是在本次暂停状态之前可能出现过的暂停状态总时间
			totalTime = m_StopTimeInCounts - m_StartTimeInCounts - m_PausedTimeInCounts;
		}
		else
		{
			//这里的PausedTime也是在本次暂停状态之前可能出现过的暂停状态总时间
			totalTime = m_CurrTimeInCounts - m_StartTimeInCounts - m_PausedTimeInCounts;
		}

		return (static_cast<double>(totalTime) * m_SecondsPerCount);
	}

	double GameTimer::GetCurrTime() const
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&m_CurrTimeInCounts);

		return m_SecondsPerCount * static_cast<double>(m_CurrTimeInCounts);
	}

}// namespace RenderDog