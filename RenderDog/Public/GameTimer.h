//------------------------------------------------------
//Created by Xiang Weikang
//Desc: 计时器，主要用于对游戏计时
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

		void				Reset();                 //重置GameTimer，这个函数需要在程序开始运行的时候调用一次
		void				Tick();                  //在每一帧中调用一次，用来获取当前这一帧的时间状态

		void				Start();                 //只有在暂停状态下调用该函数才有用，更新m_PausedTime，通过当前时间减去StopTime得到PausedTime
		void				Pause();                 //在玩家按下暂停键或者游戏窗口最小化时，游戏都应该暂停

		double				GetDeltaTime() const;   //单位为秒，返回m_DeltaTimeInSeconds
		double				GetTotalTime() const;   //单位为秒，计算游戏开始到当前帧的总用时
		double				GetCurrTime() const;    //单位为秒，获取当前时间

	private:
		bool				m_bIsPaused;             //游戏是否处于暂停状态

		double				m_SecondsPerCount;     //一次Count有多少秒，对应PerformanceFrequency的倒数
		double				m_DeltaTimeInSeconds;  //单位为秒，两帧之间的时间差

		int64_t				m_StartTimeInCounts;  //游戏开始时的时间
		int64_t				m_CurrTimeInCounts;   //当前帧的时间
		int64_t				m_PrevTimeInCounts;   //用于保存上一帧的时间，用于计算DeltaTime
		int64_t				m_StopTimeInCounts;   //暂停时的时间
		int64_t				m_PausedTimeInCounts; //游戏从开始到现在暂停的总时间（可能多次出现暂停状态）
	};
}