////////////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Framework.h
//Written by Xiang Weikang
//Desc: Application Framework Interface
////////////////////////////////////////////////

#pragma once

namespace RenderDog
{
	class IFramework
	{
	public:
		IFramework() = default;
		virtual ~IFramework() = default;

		virtual bool Init() = 0;
		virtual void Release() = 0;

		virtual void Frame() = 0;
		virtual void OnResize() = 0;
	};

	extern IFramework* g_pIFramework;

}// namespace RenderDog