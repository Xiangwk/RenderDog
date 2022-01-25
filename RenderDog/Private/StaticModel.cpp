////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: StaticModel.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "StaticModel.h"
#include "Scene.h"

namespace RenderDog
{
	StaticModel::StaticModel() :
		m_Meshes(0)
	{}

	StaticModel::~StaticModel()
	{
		m_Meshes.clear();
	}

	void StaticModel::LoadFromData(const std::vector<LocalVertex>& vertices, 
								   const std::vector<uint32_t>& indices, 
								   const std::wstring& diffuseTexturePath)
	{
		StaticMesh mesh;
		mesh.LoadFromData(vertices, indices, diffuseTexturePath);
		mesh.InitRenderData();

		m_Meshes.push_back(mesh);
	}

	void StaticModel::SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale)
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			StaticMesh* pMesh = &(m_Meshes[i]);
			pMesh->SetPosGesture(pos, euler, scale);
		}
	}

	void StaticModel::RegisterToScene(IScene* pScene)
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			IPrimitive* pMesh = &(m_Meshes[i]);
			pScene->RegisterPrimitive(pMesh);
		}
	}

	void StaticModel::ReleaseRenderData()
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			StaticMesh* pMesh = &(m_Meshes[i]);
			pMesh->ReleaseRenderData();
		}
	}


}// namespace RenderDog