////////////////////////////////////////
//RenderDog <��,��>
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

	class IScene;

	class IRenderer
	{
	public:
		virtual ~IRenderer() = default;

		virtual bool	Init(const RendererInitDesc& desc) = 0;
		virtual void	Release() = 0;
		
		virtual void	Render(IScene* pScene) = 0;

		//������ʱ��Ӹýӿ������´���Renderer��Ҫ����Դ������Ӧʹ��һ����Դ����������Renderer����Դ
		virtual bool	OnResize(uint32_t width, uint32_t height) = 0;
	};

	extern IRenderer* g_pIRenderer;

}// namespace RenderDog