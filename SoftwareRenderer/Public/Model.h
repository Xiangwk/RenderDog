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
	class IDevice;
	class IDeviceContext;

	class StaticModel
	{
	public:
		StaticModel();
		~StaticModel();

		bool LoadFromFile(const std::string& strFileName);

		bool Init(IDevice* pDevice);
		void Release();

		void Draw(IDeviceContext* pDeviceContext);

	private:
		void ProcessNode(const aiNode* pAssimpNode, const aiScene* pAssimpScene);
		StaticMesh ProcessMesh(const aiMesh* pAssimpMesh, const aiScene* pAssimpScene);

		void CalculateMeshTangents();

	private:
		std::vector<StaticMesh> m_Meshes;
	};
}