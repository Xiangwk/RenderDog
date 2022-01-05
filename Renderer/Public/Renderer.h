////////////////////////////////////////
//RenderDog <·,·>
//FileName: Renderer.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include <cstdint>
#include <d3d11.h>

namespace RenderDog
{
	struct RendererInitDesc
	{
		HWND				hWnd;
		uint32_t			backBufferWidth;
		uint32_t			backBufferHeight;
	};

	class IRenderer
	{
	public:
		IRenderer() = default;
		virtual ~IRenderer() = default;

		virtual bool Init(const RendererInitDesc& desc) = 0;
		virtual void Release() = 0;
		
		virtual void Render() = 0;

		//这里暂时添加该接口来重新创建Renderer需要的资源，后续应使用一个资源管理器管理Renderer的资源
		virtual bool OnResize(uint32_t width, uint32_t height) = 0;
	};

	extern IRenderer* g_pIRenderer;
}