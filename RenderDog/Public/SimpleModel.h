////////////////////////////////////////////////
// RenderDog <·,·>
// FileName: SimpleModel.h
// Written by Xiang Weikang
// Desc: 用于绘制标识物体，如地面网格，坐标系箭头等
////////////////////////////////////////////////

#pragma once

#include "SimpleMesh.h"
#include "Vertex.h"
#include "FbxImporter.h"

#include <vector>
#include <string>

namespace RenderDog
{
	class IScene;

	class SimpleModel
	{
	public:
		SimpleModel();
		~SimpleModel();

		SimpleModel(const SimpleModel&) = default;
		SimpleModel& operator=(const SimpleModel&) = default;

		void						LoadFromSimpleData(const std::vector<SimpleVertex>& vertices, const std::vector<uint32_t>& indices,
													   const std::string& name);

		bool						LoadFromRawMeshData(const std::vector<RDFbxImporter::RawMeshData>& rawMeshDatas,
														const std::string& fileName);

		void						RegisterToScene(IScene* pScene);

		void						SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

	private:
		std::vector<SimpleMesh>		m_Meshes;
	};
}// namespace RenderDog