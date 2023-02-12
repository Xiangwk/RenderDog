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

	void SimpleModel::LoadFromSimpleData(const std::vector<SimpleVertex>& vertices, const std::vector<uint32_t>& indices,
		const std::string& name)
	{
		SimpleMesh mesh;
		mesh.LoadFromSimpleData(vertices, indices, name);
		mesh.InitRenderData();

		m_Meshes.push_back(mesh);
	}

	bool SimpleModel::LoadFromRawMeshData(const std::vector<RDFbxImporter::RawMeshData>& rawMeshDatas, const std::string& fileName)
	{
		for (uint32_t i = 0; i < rawMeshDatas.size(); ++i)
		{
			const RDFbxImporter::RawMeshData& meshData = rawMeshDatas[i];

			std::vector<SimpleVertex> vertices;
			vertices.reserve(meshData.postions.size());

			//FBX中为Z轴朝上的右手系，使用下面的矩阵顶点坐标转换为Y轴朝上的左手系
			Matrix4x4 transAxisMatrix(1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);

			for (uint32_t index = 0; index < meshData.postions.size(); ++index)
			{
				SimpleVertex vert;
				Vector4 tempPos = Vector4(meshData.postions[index], 1.0f);
				tempPos = tempPos * transAxisMatrix;
				vert.position = Vector3(tempPos.x, tempPos.y, tempPos.z);
				vert.color = Vector4(meshData.color[index].x, meshData.color[index].y, meshData.color[index].z, meshData.color[index].w);

				vertices.push_back(vert);
			}

			std::string meshName = fileName + "_" + meshData.name;
			
			SimpleMesh mesh(meshName);
			mesh.GenVerticesAndIndices(vertices);

			m_Meshes.push_back(mesh);
		}

		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			SimpleMesh& mesh = m_Meshes[i];
			mesh.InitRenderData();
		}

		return true;
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

	void SimpleModel::SetRenderLine(bool bRenderLine)
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			SimpleMesh* pMesh = &(m_Meshes[i]);
			pMesh->SetRenderLine(bRenderLine);
		}
	}
}// namespace RenderDog