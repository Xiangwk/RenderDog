////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Scene.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Scene.h"
#include "Primitive.h"
#include "Light.h"

#include <vector>

namespace RenderDog
{
	//==============================================================
	//                   Scene
	//==============================================================
	class Scene : public IScene
	{
	public:
		Scene();
		~Scene();

		virtual bool				Init(const SceneInitDesc& desc) override;
		virtual void				Release() override;

		virtual void				RegisterPrimitive(IPrimitive* pPrimitive) override;
		virtual IPrimitive*			GetPrimitive(uint32_t index) override { return m_Primitives[index]; }
		virtual uint32_t			GetPrimitivesNum() const override { return (uint32_t)m_Primitives.size(); }

		virtual	void				RegisterLight(ILight* pLight) override;
		virtual ILight*				GetLight(uint32_t index) override { return m_Lights[index]; }
		virtual uint32_t			GetLightsNum() const override { return (uint32_t)m_Lights.size(); }

	private:
		std::string						m_Name;
		std::vector<IPrimitive*>		m_Primitives;
		std::vector<ILight*>			m_Lights;
	};

	Scene::Scene() :
		m_Name(),
		m_Primitives(0),
		m_Lights(0)
	{}

	Scene::~Scene()
	{
		m_Primitives.clear();
		m_Lights.clear();
	}

	bool Scene::Init(const SceneInitDesc& desc)
	{
		m_Name = desc.name;

		return true;
	}

	void Scene::Release()
	{
		g_pISceneManager->ReleaseScene(this);
	}

	void Scene::RegisterPrimitive(IPrimitive* pPrimitive)
	{
		m_Primitives.push_back(pPrimitive);
	}

	void Scene::RegisterLight(ILight* pLight)
	{
		m_Lights.push_back(pLight);
	}

	//==============================================================
	//                   SceneManager
	//==============================================================
	class SceneManager : public ISceneManager
	{
	public:
		SceneManager() = default;
		virtual ~SceneManager() = default;

		virtual IScene* CreateScene(const SceneInitDesc& sceneDesc);
		virtual void	ReleaseScene(IScene* pScene);
	};

	SceneManager g_SceneManager;
	ISceneManager* g_pISceneManager = &g_SceneManager;

	IScene* SceneManager::CreateScene(const SceneInitDesc& sceneDesc)
	{
		IScene* pScene = new Scene();
		if (!pScene->Init(sceneDesc))
		{
			return nullptr;
		}

		pScene->AddRef();

		return pScene;
	}

	void SceneManager::ReleaseScene(IScene* pScene)
	{
		pScene->SubRef();
	}


}// namespace RenderDog