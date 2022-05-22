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
	StaticModel::StaticModel() :
		m_Meshes(0)
	{}

	StaticModel::~StaticModel()
	{
		m_Meshes.clear();
	}

	void StaticModel::LoadFromStandardData(const std::vector<StandardVertex>& vertices, 
								   const std::vector<uint32_t>& indices,
								   VERTEX_TYPE vertType, 
								   const std::string& vsFile, 
								   const std::string& psFile)
	{
		StaticMesh mesh;
		mesh.LoadFromStandardData(vertices, indices);
		mesh.InitRenderData(vertType, vsFile, psFile);

		m_Meshes.push_back(mesh);
	}

	bool StaticModel::LoadTextureFromFile(const std::wstring& diffuseTexturePath)
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			StaticMesh& mesh = m_Meshes[i];
			if (!mesh.LoadTextureFromFile(diffuseTexturePath))
			{
				return false;
			}
		}

		return true;
	}

	bool StaticModel::LoadFromFile(const std::string& fileName,
								   VERTEX_TYPE vertType,
								   const std::string& vsFile,
								   const std::string& psFile)
	{
		Assimp::Importer modelImporter;
		const aiScene* assimpModelScene = modelImporter.ReadFile(fileName, aiProcess_ConvertToLeftHanded);
		if (!assimpModelScene || assimpModelScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpModelScene->mRootNode)
		{
			return false;
		}

		ProcessNode(assimpModelScene->mRootNode, assimpModelScene);

		CalculateMeshTangents();

		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			StaticMesh& mesh = m_Meshes[i];
			mesh.InitRenderData(vertType, vsFile, psFile);
		}

		return true;
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


	void StaticModel::ProcessNode(const aiNode* pAssimpNode, const aiScene* pAssimpScene)
	{
		for (uint32_t i = 0; i < pAssimpNode->mNumMeshes; ++i)
		{
			aiMesh* pMesh = pAssimpScene->mMeshes[pAssimpNode->mMeshes[i]];

			StaticMesh mesh = ProcessMesh(pMesh, pAssimpScene);
			m_Meshes.push_back(mesh);
		}

		for (uint32_t i = 0; i < pAssimpNode->mNumChildren; ++i)
		{
			ProcessNode(pAssimpNode->mChildren[i], pAssimpScene);
		}
	}

	StaticMesh StaticModel::ProcessMesh(const aiMesh* pAssimpMesh, const aiScene* pAssimpScene)
	{
		std::vector<StandardVertex> tempVertices;
		std::vector<uint32_t> tempIndices;

		for (unsigned int i = 0; i < pAssimpMesh->mNumVertices; ++i)
		{
			Vector3 position = Vector3(pAssimpMesh->mVertices[i].x, pAssimpMesh->mVertices[i].y, pAssimpMesh->mVertices[i].z);
			Vector3 normal = Vector3(pAssimpMesh->mNormals[i].x, pAssimpMesh->mNormals[i].y, pAssimpMesh->mNormals[i].z);
			Vector2 texCoord = Vector2(pAssimpMesh->mTextureCoords[0][i].x, pAssimpMesh->mTextureCoords[0][i].y);

			StandardVertex vert(position.x, position.y, position.z, 1.0f, 1.0f, 1.0f, 1.0f, normal.x, normal.y, normal.z, 0.0f, 0.0f, 0.0f, texCoord.x, texCoord.y);

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

		return StaticMesh(tempVertices, tempIndices);
	}

	void StaticModel::CalculateMeshTangents()
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			m_Meshes[i].CalculateTangents();
		}
	}


}// namespace RenderDog