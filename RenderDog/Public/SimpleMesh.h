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
	struct SimpleMeshRenderData
	{
		IVertexBuffer*		pVB;
		IIndexBuffer*		pIB;

		IShader*			pVS;
		IConstantBuffer*	pLocalToWorldCB;

		SimpleMeshRenderData() :
			pVB(nullptr),
			pIB(nullptr),
			pVS(nullptr),
			pLocalToWorldCB(nullptr)
		{}
	};

	//------------------------------------------------------------------------
	//   SimpleMesh
	//------------------------------------------------------------------------

	class SimpleMesh : public IPrimitive
	{
	public:
		SimpleMesh();
		explicit SimpleMesh(const std::string& name);
		~SimpleMesh();

		SimpleMesh(const SimpleMesh& mesh);
		SimpleMesh& operator=(const SimpleMesh& mesh);

		virtual void					Render(IPrimitiveRenderer* pPrimitiveRenderer) override;

		virtual PRIMITIVE_TYPE			GetPriType() const override { return PRIMITIVE_TYPE::SIMPLE_PRI; }

		virtual const AABB&				GetAABB() const override { return m_AABB; }

		void							LoadFromSimpleData(const std::vector<SimpleVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name);

		void							GenVerticesAndIndices(std::vector<SimpleVertex>& rawVertices);

		void							InitRenderData();

		void							SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

		void							SetRenderLine(bool bRenderLine) { m_bIsLine = bRenderLine; }

	private:
		void							CloneRenderData(const SimpleMesh& mesh);
		void							ReleaseRenderData();

	private:
		std::string						m_Name;

		std::vector<SimpleVertex>		m_Vertices;
		std::vector<uint32_t>			m_Indices;

		SimpleMeshRenderData*			m_pRenderData;

		AABB							m_AABB;

		bool							m_bIsLine;
	};
}// namespace RenderDog
