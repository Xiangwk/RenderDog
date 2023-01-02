////////////////////////////////////////
// RenderDog <·,·>
// FileName: StaticModel.h
// Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "StaticMesh.h"
#include "FbxImporter.h"
#include "Material.h"

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

		void						LoadFromStandardData(const std::vector<StandardVertex>& vertices, 
														 const std::vector<uint32_t>& indices,
														 const std::string& name);

		bool						LoadFromRawMeshData(const std::vector<RDFbxImporter::RawMeshData>& rawMeshDatas,
														const std::string& fileName);

		bool						CreateMaterialInstance(IMaterial* pMtl, const std::vector<MaterialParam>* pMtlParams = nullptr);
		bool						CreateMaterialInstance(const std::string& mtlinsMapFileName);
		
		void						RegisterToScene(IScene* pScene);

		void						SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

		const AABB&					GetAABB() const { return m_AABB; }
		const BoundingSphere&		GetBoundingSphere() const { return m_BoundingSphere; }

	private:
		void						CalculateBoundings();
									//设置位姿的时候更新包围球
		void						UpdateBoundings();

	private:
		std::vector<StaticMesh>		m_Meshes;

		AABB						m_AABB;
		BoundingSphere				m_BoundingSphere;

		std::string					m_Name;
		std::string					m_Directory;
	};

}// namespace RenderDog
