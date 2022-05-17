//------------------------------------------------------
//Created by Xiang Weikang
//Desc: ��ʱ������Ҫ���ڶ���Ϸ��ʱ
//Date: 2019.1.3
//------------------------------------------------------

#pragma once

#include <cstdint>

namespace RenderDog
{
	class GameTimer
	{
	public:
		GameTimer();
		~GameTimer();

		void				Reset();                 //����GameTimer�����������Ҫ�ڳ���ʼ���е�ʱ�����һ��
		void				Tick();                  //��ÿһ֡�е���һ�Σ�������ȡ��ǰ��һ֡��ʱ��״̬

		void				Start();                 //ֻ������ͣ״̬�µ��øú��������ã�����m_PausedTime��ͨ����ǰʱ���ȥStopTime�õ�PausedTime
		void				Pause();                 //����Ұ�����ͣ��������Ϸ������С��ʱ����Ϸ��Ӧ����ͣ

		double				GetDeltaTime() const;   //��λΪ�룬����m_DeltaTimeInSeconds
		double				GetTotalTime() const;   //��λΪ�룬������Ϸ��ʼ����ǰ֡������ʱ
		double				GetCurrTime() const;    //��λΪ�룬��ȡ��ǰʱ��

	private:
		bool				m_bIsPaused;             //��Ϸ�Ƿ�����ͣ״̬

		double				m_SecondsPerCount;     //һ��Count�ж����룬��ӦPerformanceFrequency�ĵ���
		double				m_DeltaTimeInSeconds;  //��λΪ�룬��֮֡���ʱ���

		int64_t				m_StartTimeInCounts;  //��Ϸ��ʼʱ��ʱ��
		int64_t				m_CurrTimeInCounts;   //��ǰ֡��ʱ��
		int64_t				m_PrevTimeInCounts;   //���ڱ�����һ֡��ʱ�䣬���ڼ���DeltaTime
		int64_t				m_StopTimeInCounts;   //��ͣʱ��ʱ��
		int64_t				m_PausedTimeInCounts; //��Ϸ�ӿ�ʼ��������ͣ����ʱ�䣨���ܶ�γ�����ͣ״̬��
	};
}