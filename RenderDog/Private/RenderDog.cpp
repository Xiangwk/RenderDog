///////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: RenderDog.cpp
//Written by Xiang Weikang
///////////////////////////////////////////

#include "RenderDog.h"
#include "Renderer.h"

namespace RenderDog
{
	class RenderDog : public IRenderDog
	{
	public:
		RenderDog() = default;
		virtual ~RenderDog() = default;

		virtual bool Init(const InitDesc& desc) override;
		virtual void Release() override;
	};

	//---------------------------------------------------------------------------
	//   Public Function Definition
	//---------------------------------------------------------------------------
	bool RenderDog::Init(const InitDesc& desc)
	{
		if (!g_pIFramework->Init())
		{
			return false;
		}

		if (!g_pIWindow->Init(desc.wndDesc))
		{
			return false;
		}

		RendererInitDesc rendererDesc;
		rendererDesc.hWnd = g_pIWindow->GetHandle();
		rendererDesc.backBufferWidth = g_pIWindow->GetWidth();
		rendererDesc.backBufferHeight = g_pIWindow->GetHeight();
		if (!g_pIRenderer->Init(rendererDesc))
		{
			return false;
		}

		return true;
	}

	void RenderDog::Release()
	{
		g_pIFramework->Release();

		g_pIWindow->Release();

		g_pIRenderer->Release();
	}

	//---------------------------------------------------------------------------
	//   Global Function Definition
	//---------------------------------------------------------------------------
	bool CreateRenderDog(IRenderDog** ppRenderDog)
	{
		RenderDog* pRenderDog = new RenderDog();
		if (!pRenderDog)
		{
			return false;
		}

		*ppRenderDog = pRenderDog;

		return true;
	}

	void DestoryRenderDog(IRenderDog** ppRenderDog)
	{
		IRenderDog* pRenderDog = *ppRenderDog;
		delete pRenderDog;

		*ppRenderDog = nullptr;
	}

}// namespace RenderDog