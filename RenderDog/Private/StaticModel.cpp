////////////////////////////////////////
// RenderDog <·,·>
// FileName: StaticModel.cpp
// Written by Xiang Weikang
////////////////////////////////////////

#include "StaticModel.h"
#include "Scene.h"

#include <fstream>


namespace RenderDog
{
	StaticModel::StaticModel() :
		m_Meshes(0),
		m_AABB(),
		m_BoundingSphere(),
		m_Name(),
		m_Directory()
	{}

	StaticModel::~StaticModel()
	{
		m_Meshes.clear();
	}

	void StaticModel::LoadFromStandardData(const std::vector<StandardVertex>& vertices,
		const std::vector<uint32_t>& indices,
		const std::string& name)
	{
		StaticMesh mesh;
		mesh.LoadFromStandardData(vertices, indices, name);
		mesh.InitRenderData();

		CalculateBoundings();

		m_Meshes.push_back(mesh);
	}

	bool StaticModel::CreateMaterialInstance(IMaterial* pMtl, const std::vector<MaterialParam>* pMtlParams /*= nullptr*/)
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			StaticMesh& mesh = m_Meshes[i];
			if (!mesh.CreateMaterialInstance(pMtl, pMtlParams))
			{
				return false;
			}
		}

		return true;
	}

	bool StaticModel::LoadMaterialInstance()
	{
		std::string mtlinsMapName = m_Directory + m_Name + ".mtlinsmap";

		std::vector<std::string> mtlinsFiles;

		if (!g_pMaterialManager->LoadMaterialInstanceMap(mtlinsMapName, mtlinsFiles))
		{
			return false;
		}

		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			StaticMesh& mesh = m_Meshes[i];
			if (!mesh.LoadMaterialInstance(m_Directory + MTLINS_DIR + mtlinsFiles[i]))
			{
				return false;
			}
		}

		return true;
	}

	bool StaticModel::LoadFromRawMeshData(const std::vector<RDFbxImporter::RawMeshData>& rawMeshDatas, const std::string& fileName)
	{
		for (uint32_t i = 0; i < rawMeshDatas.size(); ++i)
		{
			const RDFbxImporter::RawMeshData& meshData = rawMeshDatas[i];

			std::vector<StandardVertex> vertices;
			vertices.reserve(meshData.postions.size());

			//FBX中为Z轴朝上的右手系，使用下面的矩阵顶点坐标转换为Y轴朝上的左手系
			Matrix4x4 transAxisMatrix(1.0f, 0.0f, 0.0f, 0.0f,
									  0.0f, 0.0f, 1.0f, 0.0f,
									  0.0f, 1.0f, 0.0f, 0.0f,
									  0.0f, 0.0f, 0.0f, 1.0f);

			for (uint32_t index = 0; index < meshData.postions.size(); ++index)
			{
				StandardVertex vert;
				Vector4 tempPos = Vector4(meshData.postions[index], 1.0f);
				tempPos = tempPos * transAxisMatrix;
				vert.position = Vector3(tempPos.x, tempPos.y, tempPos.z);
				vert.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
				vert.normal = Vector3(0.0f, 0.0f, 0.0f);
				vert.tangent = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
				vert.texcoord = meshData.texcoords[index];

				vertices.push_back(vert);
			}

			size_t nameStart = fileName.rfind("/") + 1;
			size_t nameEnd = fileName.rfind(".");
			m_Name = fileName.substr(nameStart, nameEnd - nameStart);
			m_Directory = fileName.substr(0, fileName.rfind("/") + 1);

			std::string meshName = fileName + "_" + meshData.name;
			StaticMesh mesh(meshName);
			mesh.CalcTangentsAndGenIndices(vertices, meshData.smoothGroup);

			m_Meshes.push_back(mesh);
		}

		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			StaticMesh& mesh = m_Meshes[i];
			mesh.InitRenderData();
		}

		CalculateBoundings();

		LoadMaterialInstance();

		return true;
	}

	void StaticModel::SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale)
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			StaticMesh* pMesh = &(m_Meshes[i]);
			pMesh->SetPosGesture(pos, euler, scale);
		}

		UpdateBoundings();
	}

	void StaticModel::RegisterToScene(IScene* pScene)
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			IPrimitive* pMesh = &(m_Meshes[i]);
			pScene->RegisterPrimitive(pMesh);
		}

		//注册模型时要更新场景的包围球，默认场景的中心点在世界空间的原点
		BoundingSphere& sceneBoundingSphere = pScene->GetBoundingSphere();
		float modelMaxDisToSceneCenter = m_BoundingSphere.center.Length() + m_BoundingSphere.radius;
		sceneBoundingSphere.radius = sceneBoundingSphere.radius > modelMaxDisToSceneCenter ? sceneBoundingSphere.radius : modelMaxDisToSceneCenter;
	}

	void StaticModel::CalculateBoundings()
	{
		m_AABB.Reset();
		m_BoundingSphere.Reset();
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			m_Meshes[i].CalculateAABB();
			const AABB& meshAABB = m_Meshes[i].GetAABB();

			m_AABB.minPoint.x = m_AABB.minPoint.x < meshAABB.minPoint.x ? m_AABB.minPoint.x : meshAABB.minPoint.x;
			m_AABB.minPoint.y = m_AABB.minPoint.y < meshAABB.minPoint.y ? m_AABB.minPoint.y : meshAABB.minPoint.y;
			m_AABB.minPoint.z = m_AABB.minPoint.z < meshAABB.minPoint.z ? m_AABB.minPoint.z : meshAABB.minPoint.z;

			m_AABB.maxPoint.x = m_AABB.maxPoint.x > meshAABB.maxPoint.x ? m_AABB.maxPoint.x : meshAABB.maxPoint.x;
			m_AABB.maxPoint.y = m_AABB.maxPoint.y > meshAABB.maxPoint.y ? m_AABB.maxPoint.y : meshAABB.maxPoint.y;
			m_AABB.maxPoint.z = m_AABB.maxPoint.z > meshAABB.maxPoint.z ? m_AABB.maxPoint.z : meshAABB.maxPoint.z;
		}

		m_BoundingSphere.center = (m_AABB.minPoint + m_AABB.maxPoint) * 0.5f;
		m_BoundingSphere.radius = (m_AABB.maxPoint - m_AABB.minPoint).Length() * 0.5f;
	}

	void StaticModel::UpdateBoundings()
	{
		m_AABB.Reset();
		m_BoundingSphere.Reset();
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			const AABB& meshAABB = m_Meshes[i].GetAABB();

			m_AABB.minPoint.x = m_AABB.minPoint.x < meshAABB.minPoint.x ? m_AABB.minPoint.x : meshAABB.minPoint.x;
			m_AABB.minPoint.y = m_AABB.minPoint.y < meshAABB.minPoint.y ? m_AABB.minPoint.y : meshAABB.minPoint.y;
			m_AABB.minPoint.z = m_AABB.minPoint.z < meshAABB.minPoint.z ? m_AABB.minPoint.z : meshAABB.minPoint.z;

			m_AABB.maxPoint.x = m_AABB.maxPoint.x > meshAABB.maxPoint.x ? m_AABB.maxPoint.x : meshAABB.maxPoint.x;
			m_AABB.maxPoint.y = m_AABB.maxPoint.y > meshAABB.maxPoint.y ? m_AABB.maxPoint.y : meshAABB.maxPoint.y;
			m_AABB.maxPoint.z = m_AABB.maxPoint.z > meshAABB.maxPoint.z ? m_AABB.maxPoint.z : meshAABB.maxPoint.z;
		}

		m_BoundingSphere.center = (m_AABB.minPoint + m_AABB.maxPoint) * 0.5f;
		m_BoundingSphere.radius = (m_AABB.maxPoint - m_AABB.minPoint).Length() * 0.5f;
	}


}// namespace RenderDog