////////////////////////////////////////////////
// RenderDog <·,·>
// FileName: SimpleModel.cpp
// Written by Xiang Weikang
// Desc: 用于绘制标识物体，如地面网格，坐标系箭头等
////////////////////////////////////////////////

#include "SimpleModel.h"
#include "Scene.h"

namespace RenderDog
{
	SimpleModel::SimpleModel() :
		m_Meshes(0)
	{}

	SimpleModel::~SimpleModel()
	{
		m_Meshes.clear();
	}

	void SimpleModel::LoadFromSimpleData(const std::vector<SimpleVertex>& vertices,
		const std::vector<uint32_t>& indices,
		const std::string& vsFile,
		const std::string& psFile,
		const std::string& name)
	{
		SimpleMesh mesh;
		mesh.LoadFromSimpleData(vertices, indices, name);
		mesh.InitRenderData(vsFile, psFile);

		m_Meshes.push_back(mesh);
	}

	void SimpleModel::RegisterToScene(IScene* pScene)
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			IPrimitive* pMesh = &(m_Meshes[i]);
			pScene->RegisterPrimitive(pMesh);
		}
	}

	void SimpleModel::SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale)
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			SimpleMesh* pMesh = &(m_Meshes[i]);
			pMesh->SetPosGesture(pos, euler, scale);
		}
	}
}// namespace RenderDog