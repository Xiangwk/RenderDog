////////////////////////////////////////
//RenderDog <��,��>
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

		//������ͣʱ��
		m_StopTimeInCounts = 0;
		m_PausedTimeInCounts = 0;

		m_bIsPaused = false;
	}

	void GameTimer::Tick()
	{
		//�����ʱ��������ͣ״̬����DeltaTime����Ϊ0��
		//��ʹ��UpdateScene�����еĲ���ֹͣ
		if (m_bIsPaused)
		{
			m_DeltaTimeInSeconds = 0.0;
			return;
		}

		QueryPerformanceCounter((LARGE_INTEGER*)&m_CurrTimeInCounts);

		int64_t deltaTime = m_CurrTimeInCounts - m_PrevTimeInCounts;
		m_DeltaTimeInSeconds = (double)deltaTime * m_SecondsPerCount;
		//������������ʡ��ģʽ�����ڶ�˴���ʱ�л�������������ʱ��DeltaTime���ܳ��ָ�ֵ
		//������ǿ�ƽ�������Ϊ�Ǹ�ֵ
		if (m_DeltaTimeInSeconds < 0.0)
		{
			m_DeltaTimeInSeconds = 0.0;
		}

		//ʹ��PrevTime��¼��ǰʱ�䣬����һ֡��ʹ��
		m_PrevTimeInCounts = m_CurrTimeInCounts;
	}

	void GameTimer::Start()
	{
		if (m_bIsPaused)
		{
			int64_t StartTime = 0;
			QueryPerformanceCounter((LARGE_INTEGER*)&StartTime);

			//�ۼƸ���PausedTime
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
			//�����PausedTime���ڱ�����ͣ״̬֮ǰ���ܳ��ֹ�����ͣ״̬��ʱ��
			totalTime = m_StopTimeInCounts - m_StartTimeInCounts - m_PausedTimeInCounts;
		}
		else
		{
			//�����PausedTimeҲ���ڱ�����ͣ״̬֮ǰ���ܳ��ֹ�����ͣ״̬��ʱ��
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