////////////////////////////////////////
//RenderDog <��,��>
//FileName: D3D11Renderer.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include <cstdint>
#include <d3d11.h>

#include "Renderer.h"

namespace RenderDog
{
	class ID3D11Renderer : public IRenderer
	{
	public:
		ID3D11Renderer() = default;
		virtual ~ID3D11Renderer() = default;

		virtual bool Init(const RendererInitDesc& desc) = 0;
		virtual void Release() = 0;
		
		virtual void Render() = 0;

		//������ʱ��Ӹýӿ������´���Renderer��Ҫ����Դ������Ӧʹ��һ����Դ����������Renderer����Դ
		virtual bool OnResize(uint32_t width, uint32_t height) = 0;
	};
}