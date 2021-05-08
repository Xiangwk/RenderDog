#include "Device.h"
#include "SwapChain.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"
#include "Buffer.h"
#include "Shader.h"
#include "Vertex.h"

namespace RenderDog
{
	bool Device::CreateTexture2D(const Texture2DDesc* pDesc, Texture2D** ppTexture)
	{
		Texture2D* pTex = new Texture2D(pDesc->width, pDesc->height, pDesc->format);
		if (!pTex)
		{
			return false;
		}

		*ppTexture = pTex;

		return true;
	}

	bool Device::CreateRenderTargetView(Texture2D* pTexture, const RenderTargetDesc* pDesc, RenderTargetView** ppRenderTarget)
	{
		if (!pDesc)
		{
			RenderTargetView* pRT = new RenderTargetView();
			if (!pRT)
			{
				return false;
			}

			*ppRenderTarget = pRT;

			pRT->GetView() = (uint32_t*)pTexture->GetData();
			pRT->SetWidth(pTexture->GetWidth());
			pRT->SetHeight(pTexture->GetHeight());
		}
		else
		{
			//ToDo: Create RenderTargetView by Desc
		}

		return true;
	}

	bool Device::CreateDepthStencilView(Texture2D* pTexture, DepthStencilView** ppDepthStencil)
	{
		DepthStencilView* pDS = new DepthStencilView();
		if (!pDS)
		{
			return false;
		}

		*ppDepthStencil = pDS;

		pDS->GetView() = (float*)pTexture->GetData();
		pDS->SetWidth(pTexture->GetWidth());
		pDS->SetHeight(pTexture->GetHeight());

		return true;
	}

	bool Device::CreateVertexBuffer(const VertexBufferDesc& vbDesc, VertexBuffer** ppVertexBuffer)
	{
		if (*ppVertexBuffer)
		{
			(*ppVertexBuffer)->Release();
			*ppVertexBuffer = nullptr;
		}

		VertexBuffer* pVB = new VertexBuffer(vbDesc);
		if (!pVB)
		{
			return false;
		}

		*ppVertexBuffer = pVB;
		
		return true;
	}

	bool Device::CreateIndexBuffer(const IndexBufferDesc& ibDesc, IndexBuffer** ppIndexBuffer)
	{
		if (*ppIndexBuffer)
		{
			(*ppIndexBuffer)->Release();
			*ppIndexBuffer = nullptr;
		}

		IndexBuffer* pIB = new IndexBuffer(ibDesc);
		if (!pIB)
		{
			return false;
		}

		*ppIndexBuffer = pIB;

		return true;
	}

	bool Device::CreateVertexShader(VertexShader** ppVertexShader)
	{
		if (*ppVertexShader)
		{
			delete *ppVertexShader;
		}

		VertexShader* pVS = new VertexShader();
		if (!pVS)
		{
			return false;
		}

		*ppVertexShader = pVS;

		return true;
	}

	bool Device::CreatePixelShader(PixelShader** ppPixelShader)
	{
		if (*ppPixelShader)
		{
			delete* ppPixelShader;
		}

		PixelShader* pPS = new PixelShader();
		if (!pPS)
		{
			return false;
		}

		*ppPixelShader = pPS;

		return true;
	}
}