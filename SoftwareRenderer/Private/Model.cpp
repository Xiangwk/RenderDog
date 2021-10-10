///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Model.cpp
//Written by Xiang Weikang
///////////////////////////////////

#include "Model.h"

#include "assimp/Importer.hpp"
#include "assimp/Scene.h"
#include "assimp/postprocess.h"

namespace RenderDog
{
	StaticModel::StaticModel()
	{
		m_Meshes = {};
	}

	StaticModel::~StaticModel()
	{
		m_Meshes.clear();
	}

	bool StaticModel::LoadFromFile(const std::string& fileName)
	{
		Assimp::Importer modelImporter;
		const aiScene* assimpModelScene = modelImporter.ReadFile(fileName, aiProcess_ConvertToLeftHanded/* | aiProcess_JoinIdenticalVertices*/);
		if (!assimpModelScene || assimpModelScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpModelScene->mRootNode)
		{
			return false;
		}

		ProcessNode(assimpModelScene->mRootNode, assimpModelScene);

		CalculateMeshTangents();

		return true;
	}

	void StaticModel::ProcessNode(const aiNode* pAssimpNode, const aiScene* pAssimpScene)
	{
		for (uint32_t i = 0; i < pAssimpNode->mNumMeshes; ++i)
		{
			aiMesh* pMesh = pAssimpScene->mMeshes[pAssimpNode->mMeshes[i]];
			m_Meshes.push_back(ProcessMesh(pMesh, pAssimpScene));
		}

		for (uint32_t i = 0; i < pAssimpNode->mNumChildren; ++i)
		{
			ProcessNode(pAssimpNode->mChildren[i], pAssimpScene);
		}
	}

	StaticMesh StaticModel::ProcessMesh(const aiMesh* pAssimpMesh, const aiScene* pAssimpScene)
	{
		std::vector<Vertex> tempVertices;
		std::vector<uint32_t> tempIndices;

		for (unsigned int i = 0; i < pAssimpMesh->mNumVertices; ++i)
		{

			Vector3 position = Vector3(pAssimpMesh->mVertices[i].x, pAssimpMesh->mVertices[i].y, pAssimpMesh->mVertices[i].z);
			Vector3 normal = Vector3(pAssimpMesh->mNormals[i].x, pAssimpMesh->mNormals[i].y, pAssimpMesh->mNormals[i].z);
			Vector2 texCoord = Vector2(pAssimpMesh->mTextureCoords[0][i].x, pAssimpMesh->mTextureCoords[0][i].y);

			Vertex vert = { Vector3(position.x, position.y, position.z), Vector3(1.0f, 1.0f, 1.0f), Vector3(normal.x, normal.y, normal.z), Vector4(0.0f, 0.0f, 0.0f, 0.0f), Vector2(texCoord.x, texCoord.y) };

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

	bool StaticModel::Init(IDevice* pDevice)
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			if (!m_Meshes[i].Init(pDevice))
			{
				return false;
			}
		}

		return true;
	}

	void StaticModel::Release()
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			m_Meshes[i].Release();
		}
	}

	void StaticModel::Draw(IDeviceContext* pDeviceContext)
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			m_Meshes[i].Draw(pDeviceContext);
		}
	}
}