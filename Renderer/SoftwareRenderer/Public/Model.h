///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Mesh.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include "Mesh.h"

#include <string>

struct aiNode;
struct aiMesh;
struct aiScene;

namespace RenderDog
{
	class ISRDevice;
	class ISRDeviceContext;

	class StaticModel
	{
	public:
		StaticModel();
		~StaticModel();

		bool LoadFromFile(const std::string& fileName);

		bool Init(ISRDevice* pDevice);
		void Release();

		void Draw(ISRDeviceContext* pDeviceContext);

	private:
		void ProcessNode(const aiNode* pAssimpNode, const aiScene* pAssimpScene);
		StaticMesh ProcessMesh(const aiMesh* pAssimpMesh, const aiScene* pAssimpScene);

		void CalculateMeshTangents();

	private:
		std::vector<StaticMesh> m_Meshes;
	};
}