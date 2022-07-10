////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SkinModel.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "SkinMesh.h"
#include "FbxImporter.h"

#include <vector>

namespace RenderDog
{
	class SkinModel
	{
	public:
		SkinModel();
		~SkinModel();

		SkinModel(const SkinModel& model);
		SkinModel& operator=(const SkinModel& model);

		bool						LoadFromRawMeshData(const std::vector<RDFbxImporter::RawMeshData>& rawMeshDatas,
														const std::string& vsFile, const std::string& psFile,
														const std::string& fileName);

		bool						LoadTextureFromFile(const std::wstring& diffuseTexturePath, const std::wstring& normalTexturePath);

		void						RegisterToScene(IScene* pScene);

		void						SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

		const AABB&					GetAABB() const { return m_AABB; }
		const BoundingSphere&		GetBoundingSphere() const { return m_BoundingSphere; }

	private:
		std::vector<SkinMesh>		m_Meshes;

		AABB						m_AABB;
		BoundingSphere				m_BoundingSphere;
	};

}// namespace RenderDog
