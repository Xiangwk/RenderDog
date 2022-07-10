////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: StaticModel.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "StaticMesh.h"
#include "FbxImporter.h"

#include <string>

namespace RenderDog
{
	class IScene;

	class StaticModel
	{
	public:
		StaticModel();
		~StaticModel();

		StaticModel(const StaticModel&) = default;
		StaticModel& operator=(const StaticModel&) = default;

		void						LoadFromStandardData(const std::vector<StandardVertex>& vertices, const std::vector<uint32_t>& indices, 
														 const std::string& vsFile, const std::string& psFile,
														 const std::string& name);

		bool						LoadFromRawMeshData(const std::vector<RDFbxImporter::RawMeshData>& rawMeshDatas, 
														const std::string& vsFile, const std::string& psFile, 
														const std::string& fileName);

		bool						LoadTextureFromFile(const std::wstring& diffuseTexturePath, const std::wstring& normalTexturePath);
		
		void						RegisterToScene(IScene* pScene);

		void						SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

		const AABB&					GetAABB() const { return m_AABB; }
		const BoundingSphere&		GetBoundingSphere() const { return m_BoundingSphere; }

	private:
		void						CalculateBoundings();
		//Use to update aabb and bounding sphere when set pos and gesture
		void						UpdateBoundings();

	private:
		std::vector<StaticMesh>		m_Meshes;

		AABB						m_AABB;
		BoundingSphere				m_BoundingSphere;
	};

}// namespace RenderDog
