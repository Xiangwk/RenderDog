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
	class Device;
	class DeviceContext;

	class StaticModel
	{
	public:
		StaticModel();
		~StaticModel();

		bool LoadFromFile(const std::string& strFileName);

		bool Init(Device* pDevice);
		void Release();

		void Draw(DeviceContext* pDeviceContext);

	private:
		void ProcessNode(const aiNode* pAssimpNode, const aiScene* pAssimpScene);
		StaticMesh ProcessMesh(const aiMesh* pAssimpMesh, const aiScene* pAssimpScene);

	private:
		std::vector<StaticMesh> m_Meshes;
	};
}