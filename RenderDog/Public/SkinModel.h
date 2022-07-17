////////////////////////////////////////
//RenderDog <·,·>
//FileName: SkinModel.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "SkinMesh.h"
#include "FbxImporter.h"

#include <vector>

namespace RenderDog
{
	class Skeleton;

	class SkinModel
	{
	public:
		SkinModel();
		~SkinModel();

		SkinModel(const SkinModel& model) = default;
		SkinModel& operator=(const SkinModel& model) = default;

		bool						LoadFromRawMeshData(const std::vector<RDFbxImporter::RawMeshData>& rawMeshDatas,
														const RDFbxImporter::RawSkeletonData* pSkeletonData,
														const std::string& vsFile, const std::string& psFile,
														const std::string& fileName);
		


		bool						LoadTextureFromFile(const std::wstring& diffuseTexturePath, const std::wstring& normalTexturePath);

		void						RegisterToScene(IScene* pScene);

		void						SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

		const AABB&					GetAABB() const { return m_AABB; }
		const BoundingSphere&		GetBoundingSphere() const { return m_BoundingSphere; }

		void						Tick(float time);

	private:
		void						CalculateBoundings();
									//设置位姿的时候更新包围球
		void						UpdateBoundings();

	private:
		std::vector<SkinMesh>		m_Meshes;
		Skeleton*					m_pSkeleton;

		AABB						m_AABB;
		BoundingSphere				m_BoundingSphere;

		Vector3						m_WorldPosition;
		Vector3						m_EulerAngle;
		Vector3						m_Scale;
	};

}// namespace RenderDog
