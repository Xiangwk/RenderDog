///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Framework.cpp
//Written by Xiang Weikang
//Desc: Application Framework Implementation
///////////////////////////////////

#include "Framework.h"
#include "Renderer.h"
#include "Scene.h"

#include <vector>

namespace RenderDog
{
	class Framework : public IFramework
	{
	public:
		Framework():
			m_Scenes(0)
		{}

		virtual ~Framework()
		{
			m_Scenes.clear();
		}

		virtual bool			Init() override;
		virtual void			Release() override;

		virtual void			Frame() override;
		virtual void			OnResize(uint32_t width, uint32_t height) override;

		virtual void			RegisterScene(IScene* pScene) override;

	private:
		std::vector<IScene*>	m_Scenes;
	};

	Framework	g_Framework;
	IFramework*	g_pIFramework = &g_Framework;


	///////////////////////////////////////////////////////////////////////////////////
	// -----------------     Public Function Definition       -----------------------//
	///////////////////////////////////////////////////////////////////////////////////

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
		for (uint32_t i = 0; i < m_Scenes.size(); ++i)
		{
			g_pIRenderer->Update(m_Scenes[i]);
		}

		for (uint32_t i = 0; i < m_Scenes.size(); ++i)
		{
			g_pIRenderer->Render(m_Scenes[i]);
		}
	}

	void Framework::OnResize(uint32_t width, uint32_t height)
	{
		g_pIRenderer->OnResize(width, height);

		return;
	}

	void Framework::RegisterScene(IScene* pScene)
	{
		m_Scenes.push_back(pScene);
	}

}// namespace RenderDog