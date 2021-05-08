///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Device.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include <cstdint>

namespace RenderDog
{
	
	class Texture2D;
	class RenderTargetView;
	class DepthStencilView;
	class VertexBuffer;
	class IndexBuffer;
	class VertexShader;
	class PixelShader;

	struct Texture2DDesc;
	struct RenderTargetDesc;
	struct VertexBufferDesc;
	struct IndexBufferDesc;

	class Device
	{
	public:
		Device() = default;
		~Device() = default;

		bool CreateTexture2D(const Texture2DDesc* pDesc, Texture2D** ppTexture);
		bool CreateRenderTargetView(Texture2D* pTexture, const RenderTargetDesc* pDesc, RenderTargetView** ppRenderTarget);
		bool CreateDepthStencilView(Texture2D* pTexture, DepthStencilView** ppDepthStencil);
		bool CreateVertexBuffer(const VertexBufferDesc& vbDesc, VertexBuffer** ppVertexBuffer);
		bool CreateIndexBuffer(const IndexBufferDesc& ibDesc, IndexBuffer** ppIndexBuffer);
		bool CreateVertexShader(VertexShader** ppVertexShader);
		bool CreatePixelShader(PixelShader** ppPixelShader);
	};

	
}