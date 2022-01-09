////////////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Framework.h
//Written by Xiang Weikang
//Desc: Application Framework Interface
////////////////////////////////////////////////

#pragma once

#include <cstdint>

namespace RenderDog
{
	class IScene;

	class IFramework
	{
	public:
		virtual ~IFramework() = default;

		virtual bool Init() = 0;
		virtual void Release() = 0;

		virtual void Frame() = 0;
		virtual void OnResize(uint32_t width, uint32_t height) = 0;

		virtual void RegisterScene(IScene* pScene) = 0;
	};

	extern IFramework* g_pIFramework;

}// namespace RenderDog