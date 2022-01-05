///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Framework.cpp
//Written by Xiang Weikang
//Desc: Application Framework Implementation
///////////////////////////////////

#include "Framework.h"
#include "Renderer.h"

namespace RenderDog
{
	class Framework : public IFramework
	{
	public:
		Framework() = default;
		virtual ~Framework() = default;

		virtual bool Init();
		virtual void Release();

		virtual void Frame();
		virtual void OnResize(uint32_t width, uint32_t height);
	};

	Framework g_Framework;
	IFramework* g_pIFramework = &g_Framework;

	//---------------------------------------------------------------------------
	//     Function Definition
	//---------------------------------------------------------------------------
	bool Framework::Init()
	{
		return true;
	}

	void Framework::Release()
	{
		return;
	}

	void Framework::Frame()
	{
		g_pIRenderer->Render();

		return;
	}

	void Framework::OnResize(uint32_t width, uint32_t height)
	{
		g_pIRenderer->OnResize(width, height);

		return;
	}

}// namespace RenderDog