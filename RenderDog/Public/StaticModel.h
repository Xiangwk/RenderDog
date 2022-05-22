////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: StaticModel.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "StaticMesh.h"

#include <string>

//assimp
struct aiNode;
struct aiMesh;
struct aiScene;

namespace RenderDog
{
	class IScene;

	//------------------------------------------------------------------------
	//   SimpleModel
	//------------------------------------------------------------------------

	class SimpleModel
	{
	public:
		SimpleModel();
		~SimpleModel();

		SimpleModel(const SimpleModel&) = default;
		SimpleModel& operator=(const SimpleModel&) = default;

		void						LoadFromSimpleData(const std::vector<SimpleVertex>& vertices, const std::vector<uint32_t>& indices,
														const std::string& vsFile, const std::string& psFile);

		void						RegisterToScene(IScene* pScene);

		void						ReleaseRenderData();

		void						SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

	private:
		std::vector<SimpleMesh>		m_Meshes;
	};


	//------------------------------------------------------------------------
	//   StaticModel
	//------------------------------------------------------------------------

	class StaticModel
	{
	public:
		StaticModel();
		~StaticModel();

		StaticModel(const StaticModel&) = default;
		StaticModel& operator=(const StaticModel&) = default;

		void						LoadFromStandardData(const std::vector<StandardVertex>& vertices, const std::vector<uint32_t>& indices, 
															const std::string& vsFile, const std::string& psFile);
		bool						LoadFromFile(const std::string& fileName,
													const std::string& vsFile, const std::string& psFile);
		bool						LoadTextureFromFile(const std::wstring& diffuseTexturePath);
		
		void						RegisterToScene(IScene* pScene);

		void						ReleaseRenderData();

		void						SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

	private:
		void						ProcessNode(const aiNode* pAssimpNode, const aiScene* pAssimpScene);
		StaticMesh					ProcessMesh(const aiMesh* pAssimpMesh, const aiScene* pAssimpScene);

		void						CalculateMeshTangents();

	private:
		std::vector<StaticMesh>		m_Meshes;
	};

}// namespace RenderDog
