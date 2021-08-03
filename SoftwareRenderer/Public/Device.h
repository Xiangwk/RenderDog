///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Device.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include <cstdint>

#include "Resource.h"

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

	class IDevice : public IResource
	{
	public:
		virtual bool CreateTexture2D(const Texture2DDesc* pDesc, Texture2D** ppTexture) = 0;
		virtual bool CreateRenderTargetView(Texture2D* pTexture, const RenderTargetDesc* pDesc, RenderTargetView** ppRenderTarget) = 0;
		virtual bool CreateDepthStencilView(Texture2D* pTexture, DepthStencilView** ppDepthStencil) = 0;
		virtual bool CreateVertexBuffer(const VertexBufferDesc& vbDesc, VertexBuffer** ppVertexBuffer) = 0;
		virtual bool CreateIndexBuffer(const IndexBufferDesc& ibDesc, IndexBuffer** ppIndexBuffer) = 0;
		virtual bool CreateVertexShader(VertexShader** ppVertexShader) = 0;
		virtual bool CreatePixelShader(PixelShader** ppPixelShader) = 0;
	};
}