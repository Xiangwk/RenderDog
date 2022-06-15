////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Scene.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Scene.h"
#include "RefCntObject.h"
#include "Primitive.h"
#include "Light.h"
#include "Camera.h"
#include "Bounding.h"

#include <vector>

namespace RenderDog
{
	//==============================================================
	//                   Scene
	//==============================================================
	class Scene : public IScene
	{
		friend class SceneManager;

	public:
		Scene();
		Scene(const SceneInitDesc& desc);
		virtual ~Scene();

		virtual void					Release() override;
		virtual void					RegisterPrimitive(IPrimitive* pPrimitive) override;
		virtual IPrimitive*				GetPrimitive(uint32_t index) override { return m_Primitives[index]; }
		virtual uint32_t				GetPrimitivesNum() const override { return (uint32_t)m_Primitives.size(); }

		virtual	void					RegisterLight(ILight* pLight) override;
		virtual ILight*					GetLight(uint32_t index) override { return m_Lights[index]; }
		virtual uint32_t				GetLightsNum() const override { return (uint32_t)m_Lights.size(); }

		virtual const BoundingSphere&	GetBoundingSphere() const override { return m_BoundingSphere; }
		virtual BoundingSphere&			GetBoundingSphere() override { return m_BoundingSphere; }

	private:
		std::string						m_Name;
		std::vector<IPrimitive*>		m_Primitives;
		std::vector<ILight*>			m_Lights;

		BoundingSphere					m_BoundingSphere;
	};

	//==============================================================
	//                   SceneManager
	//==============================================================
	class SceneManager : public ISceneManager
	{
	public:
		SceneManager() = default;
		virtual ~SceneManager() = default;

		virtual IScene* CreateScene(const SceneInitDesc& sceneDesc);
		virtual void	ReleaseScene(Scene* pScene);
	};

	SceneManager g_SceneManager;
	ISceneManager* g_pISceneManager = &g_SceneManager;

	//==============================================================
	//                Function Implementation
	//==============================================================
	Scene::Scene() :
		m_Name(""),
		m_Primitives(0),
		m_Lights(0),
		m_BoundingSphere()
	{}

	Scene::Scene(const SceneInitDesc& desc) :
		m_Name(desc.name),
		m_Primitives(0),
		m_Lights(0),
		m_BoundingSphere()
	{}

	Scene::~Scene()
	{
		m_Primitives.clear();
		m_Lights.clear();
	}

	void Scene::Release()
	{
		g_SceneManager.ReleaseScene(this);
	}

	void Scene::RegisterPrimitive(IPrimitive* pPrimitive)
	{
		m_Primitives.push_back(pPrimitive);
	}

	void Scene::RegisterLight(ILight* pLight)
	{
		m_Lights.push_back(pLight);
	}

	IScene* SceneManager::CreateScene(const SceneInitDesc& sceneDesc)
	{
		Scene* pScene = new Scene(sceneDesc);
		if (!pScene)
		{
			//FIXME!!! May have some log here
			return nullptr;
		}

		return pScene;
	}

	void SceneManager::ReleaseScene(Scene* pScene)
	{
		if (pScene)
		{
			delete pScene;
			pScene = nullptr;
		}
	}

}// namespace RenderDog