////////////////////////////////////////
// RenderDog <��,��>
// FileName: StaticMesh.h
// Written by Xiang Weikang
////////////////////////////////////////

#include "Vertex.h"
#include "Primitive.h"
#include "Shader.h"
#include "Texture.h"
#include "Bounding.h"

#include <vector>
#include <string>

#pragma once

namespace RenderDog
{
	class	Matrix4x4;
	class   IMaterial;

	struct StaticMeshRenderData
	{
		IVertexBuffer*		pVB;
		IIndexBuffer*		pIB;

		IShader*			pVS;
		IShader*			pShadowVS;
		IConstantBuffer*	pLocalToWorldCB;

		StaticMeshRenderData() :
			pVB(nullptr),
			pIB(nullptr),
			pVS(nullptr),
			pShadowVS(nullptr),
			pLocalToWorldCB(nullptr)
		{}
	};

	class StaticMesh : public IPrimitive
	{
	public:
		StaticMesh();
		~StaticMesh();

		StaticMesh(const StaticMesh& mesh);
		StaticMesh& operator=(const StaticMesh& mesh);

		explicit StaticMesh(const std::string& name);

		virtual void					Render(IPrimitiveRenderer* pPrimitiveRenderer) override;

		virtual PRIMITIVE_TYPE			GetPriType() const override { return PRIMITIVE_TYPE::STATIC_PRI; }

		virtual const AABB&				GetAABB() const override { return m_AABB; }

		void							LoadFromStandardData(const std::vector<StandardVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name);
		bool							CreateMaterialInstance(IMaterial* pMtl);

		void							InitRenderData();

		void							SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

										//�ú������뱣֤����������Ϊ��ɢ�Ķ��㣬��������Ϊ������������3
		void							CalcTangentsAndGenIndices(std::vector<StandardVertex>& rawVertices, const std::vector<uint32_t>& smoothGroup);
		void							CalculateAABB();

		IMaterialInstance*				GetMaterialInstance() const { return m_pMtlIns; }

	private:
		void							CloneRenderData(const StaticMesh& mesh);
		void							ReleaseRenderData();
										// Use to update aabb when setPosGesture;
		void							UpdateAABB(const Matrix4x4& absTransMatrix);

	private:
		std::string						m_Name;

		std::vector<StandardVertex>		m_Vertices;
		std::vector<uint32_t>			m_Indices;

		StaticMeshRenderData*			m_pRenderData;

		IMaterialInstance*				m_pMtlIns;

		AABB							m_AABB;
	};

}// namespace RenderDog
