///////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: RenderDog.cpp
//Written by Xiang Weikang
///////////////////////////////////////////

#include "RenderDog.h"

namespace RenderDog
{
	class RenderDog : public IRenderDog
	{
	public:
		RenderDog() = default;
		virtual ~RenderDog() = default;

		virtual bool Init(const InitDesc& desc);
		virtual void Release();
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

		return true;
	}

	void RenderDog::Release()
	{
		g_pIFramework->Release();

		g_pIWindow->Release();
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