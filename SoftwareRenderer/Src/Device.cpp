#include "Device.h"
#include "SwapChain.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "Buffer.h"
#include "Shader.h"
#include "Vertex.h"

namespace RenderDog
{
	bool Device::CreateRenderTargetView(Texture2D* pTexture, const RenderTargetDesc* pDesc, RenderTargetView** ppRenderTarget)
	{
		if (!pDesc)
		{
			RenderTargetView* pRT = new RenderTargetView();
			*ppRenderTarget = pRT;

			pRT->GetView() = pTexture->GetData();
			pRT->SetWidth(pTexture->GetWidth());
			pRT->SetHeight(pTexture->GetHeight());
		}
		else
		{
			//ToDo: Create RenderTargetView by Desc
		}

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