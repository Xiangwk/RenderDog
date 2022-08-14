////////////////////////////////////////////////
// RenderDog <·,·>
// FileName: SimpleMesh.h
// Written by Xiang Weikang
// Desc: 用于绘制标识物体，如地面网格，坐标系箭头等
////////////////////////////////////////////////

#pragma once

#include "Primitive.h"
#include "Bounding.h"

#include <vector>

namespace RenderDog
{
	struct	SimpleMeshRenderData;

	//------------------------------------------------------------------------
	//   SimpleMesh
	//------------------------------------------------------------------------

	class SimpleMesh : public IPrimitive
	{
	public:
		SimpleMesh();
		~SimpleMesh();

		SimpleMesh(const SimpleMesh& mesh);
		SimpleMesh& operator=(const SimpleMesh& mesh);

		virtual void					Render(IPrimitiveRenderer* pPrimitiveRenderer) override;

		virtual PRIMITIVE_TYPE			GetPriType() const override { return PRIMITIVE_TYPE::SIMPLE_PRI; }

		virtual const AABB&				GetAABB() const override { return m_AABB; }

		void							LoadFromSimpleData(const std::vector<SimpleVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name);
		void							InitRenderData();


		void							SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

	private:
		void							CloneRenderData(const SimpleMesh& mesh);
		void							ReleaseRenderData();

	private:
		std::string						m_Name;

		std::vector<SimpleVertex>		m_Vertices;
		std::vector<uint32_t>			m_Indices;

		SimpleMeshRenderData*			m_pRenderData;

		AABB							m_AABB;

		Matrix4x4						m_LocalToWorldMatrix;
	};
}// namespace RenderDog
