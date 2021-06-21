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

	bool StaticModel::LoadFromFile(const std::string& strFileName)
	{
		Assimp::Importer ModelImporter;
		const aiScene* AssimpModelScene = ModelImporter.ReadFile(strFileName, aiProcess_ConvertToLeftHanded | aiProcess_JoinIdenticalVertices);
		if (!AssimpModelScene || AssimpModelScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !AssimpModelScene->mRootNode)
		{
			return false;
		}

		ProcessNode(AssimpModelScene->mRootNode, AssimpModelScene);

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
		std::vector<Vertex> TempVertices;
		std::vector<uint32_t> TempIndices;

		for (unsigned int i = 0; i < pAssimpMesh->mNumVertices; ++i)
		{

			Vector3 Position = Vector3(pAssimpMesh->mVertices[i].x, pAssimpMesh->mVertices[i].y, pAssimpMesh->mVertices[i].z);
			Vector3 Normal = Vector3(pAssimpMesh->mNormals[i].x, pAssimpMesh->mNormals[i].y, pAssimpMesh->mNormals[i].z);
			Vector2 TexCoord = Vector2(pAssimpMesh->mTextureCoords[0][i].x, pAssimpMesh->mTextureCoords[0][i].y);

			Vertex Vert = { Vector3(Position.x, Position.y, Position.z), Vector3(1.0f, 1.0f, 1.0f), Vector3(Normal.x, Normal.y, Normal.z), Vector4(0.0f, 0.0f, 0.0f, 0.0f), Vector2(TexCoord.x, TexCoord.y) };

			TempVertices.push_back(Vert);
		}

		for (unsigned int i = 0; i < pAssimpMesh->mNumFaces; ++i)
		{
			aiFace Face = pAssimpMesh->mFaces[i];
			for (unsigned int j = 0; j < Face.mNumIndices; ++j)
			{
				TempIndices.push_back(Face.mIndices[j]);
			}
		}

		return StaticMesh(TempVertices, TempIndices);
	}

	void StaticModel::CalculateMeshTangents()
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			m_Meshes[i].CalculateTangents();
		}
	}

	bool StaticModel::Init(Device* pDevice)
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

	void StaticModel::Draw(DeviceContext* pDeviceContext)
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			m_Meshes[i].Draw(pDeviceContext);
		}
	}

	void StaticModel::DrawTangentSpace(DeviceContext* pDeviceContext)
	{

	}
}