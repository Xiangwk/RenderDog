////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: StaticModel.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "StaticModel.h"
#include "Scene.h"

#include "assimp/Importer.hpp"
#include "assimp/Scene.h"
#include "assimp/postprocess.h"


namespace RenderDog
{
	//------------------------------------------------------------------------
	//   SimpleModel
	//------------------------------------------------------------------------

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

	void SimpleModel::ReleaseRenderData()
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			SimpleMesh* pMesh = &(m_Meshes[i]);
			pMesh->ReleaseRenderData();
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

	//------------------------------------------------------------------------
	//   StaticModel
	//------------------------------------------------------------------------

	StaticModel::StaticModel() :
		m_Meshes(0),
		m_AABB(),
		m_BoundingSphere()
	{}

	StaticModel::~StaticModel()
	{
		ReleaseRenderData();

		m_Meshes.clear();
	}

	void StaticModel::LoadFromStandardData(const std::vector<StandardVertex>& vertices, 
								   const std::vector<uint32_t>& indices,
								   const std::string& vsFile, 
								   const std::string& psFile,
								   const std::string& name)
	{
		StaticMesh mesh;
		mesh.LoadFromStandardData(vertices, indices, name);
		mesh.InitRenderData(vsFile, psFile);

		CalculateBoundings();

		m_Meshes.push_back(mesh);
	}

	bool StaticModel::LoadTextureFromFile(const std::wstring& diffuseTexturePath, const std::wstring& normalTexturePath)
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			StaticMesh& mesh = m_Meshes[i];
			if (!mesh.LoadTextureFromFile(diffuseTexturePath, normalTexturePath))
			{
				return false;
			}
		}

		return true;
	}

	bool StaticModel::LoadFromFile(const std::string& fileName, const std::string& vsFile, const std::string& psFile)
	{
		Assimp::Importer modelImporter;
		const aiScene* assimpModelScene = modelImporter.ReadFile(fileName, aiProcess_ConvertToLeftHanded);
		if (!assimpModelScene || assimpModelScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpModelScene->mRootNode)
		{
			return false;
		}

		ProcessNode(assimpModelScene->mRootNode, assimpModelScene, fileName);

		CalculateMeshTangents();

		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			StaticMesh& mesh = m_Meshes[i];
			mesh.InitRenderData(vsFile, psFile);
		}

		CalculateBoundings();

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

		BoundingSphere& sceneBoundingSphere = pScene->GetBoundingSphere();
		float modelMaxDisToSceneCenter = m_BoundingSphere.center.Length() + m_BoundingSphere.radius;
		sceneBoundingSphere.radius = sceneBoundingSphere.radius > modelMaxDisToSceneCenter ? sceneBoundingSphere.radius : modelMaxDisToSceneCenter;
	}

	void StaticModel::ReleaseRenderData()
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			StaticMesh* pMesh = &(m_Meshes[i]);
			pMesh->ReleaseRenderData();
		}
	}


	void StaticModel::ProcessNode(const aiNode* pAssimpNode, const aiScene* pAssimpScene, const std::string& modelName)
	{
		for (uint32_t i = 0; i < pAssimpNode->mNumMeshes; ++i)
		{
			aiMesh* pMesh = pAssimpScene->mMeshes[pAssimpNode->mMeshes[i]];

			StaticMesh mesh = ProcessMesh(pMesh, pAssimpScene, modelName);
			m_Meshes.push_back(mesh);
		}

		for (uint32_t i = 0; i < pAssimpNode->mNumChildren; ++i)
		{
			ProcessNode(pAssimpNode->mChildren[i], pAssimpScene, modelName);
		}
	}

	StaticMesh StaticModel::ProcessMesh(const aiMesh* pAssimpMesh, const aiScene* pAssimpScene, const std::string& modelName)
	{
		std::vector<StandardVertex> tempVertices;
		std::vector<uint32_t> tempIndices;
		std::string meshName;

		for (unsigned int i = 0; i < pAssimpMesh->mNumVertices; ++i)
		{
			Vector3 position = Vector3(pAssimpMesh->mVertices[i].x, pAssimpMesh->mVertices[i].y, pAssimpMesh->mVertices[i].z);
			Vector2 texCoord = Vector2(pAssimpMesh->mTextureCoords[0][i].x, pAssimpMesh->mTextureCoords[0][i].y);
			//Vector3 normal = Vector3(pAssimpMesh->mNormals[i].x, pAssimpMesh->mNormals[i].y, pAssimpMesh->mNormals[i].z);
			meshName = std::string(pAssimpMesh->mName.C_Str());

			StandardVertex vert(position.x, position.y, position.z, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, texCoord.x, texCoord.y);

			tempVertices.push_back(vert);
		}

		for (unsigned int i = 0; i < pAssimpMesh->mNumFaces; ++i)
		{
			aiFace face = pAssimpMesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; ++j)
			{
				tempIndices.push_back(face.mIndices[j]);
			}
		}

		return StaticMesh(tempVertices, tempIndices, modelName + "_" + meshName);
	}

	void StaticModel::CalculateMeshTangents()
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			m_Meshes[i].CalculateTangents();
		}
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