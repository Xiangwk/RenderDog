///////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: RenderDog.h
//Written by Xiang Weikang
///////////////////////////////////////////

#pragma once

#include "Window.h"
#include "Framework.h"

namespace RenderDog
{
	class FPSCamera;

	struct InitDesc
	{
		WindowDesc		wndDesc;
		FPSCamera*		pMainCamera;
	};

	class IRenderDog
	{
	public:
		virtual ~IRenderDog() = default;

		virtual bool	Init(const InitDesc& desc) = 0;
		virtual void	Release() = 0;
	};

	bool CreateRenderDog(IRenderDog** ppRenderDog);
	void DestoryRenderDog(IRenderDog** ppRenderDog);

}// namespace RenderDog