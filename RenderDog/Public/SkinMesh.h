////////////////////////////////////////
//RenderDog <·,·>
//FileName: SkinMesh.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "Primitive.h"
#include "Vertex.h"
#include "Texture.h"
#include "Bounding.h"
#include "Matrix.h"
#include "GlobalValue.h"
#include "Material.h"

#include <vector>
#include <string>

namespace RenderDog
{
	struct SkinMeshRenderData
	{
		IVertexBuffer*		pVB;
		IIndexBuffer*		pIB;

		IShader*			pVS;
		IShader*			pShadowVS;
		IConstantBuffer*	pLocalToWorldCB;
		IConstantBuffer*	pBoneTransformCB;

		SkinMeshRenderData() :
			pVB(nullptr),
			pIB(nullptr),
			pVS(nullptr),
			pShadowVS(nullptr),
			pLocalToWorldCB(nullptr),
			pBoneTransformCB(nullptr)
		{}
	};

	class SkinMesh : public IPrimitive
	{
	public:
		SkinMesh();
		~SkinMesh();

		SkinMesh(const SkinMesh& mesh);
		SkinMesh& operator=(const SkinMesh& mesh);

		explicit SkinMesh(const std::string& name);

		virtual void					Render(IPrimitiveRenderer* pPrimitiveRenderer) override;
		virtual PRIMITIVE_TYPE			GetPriType() const override { return PRIMITIVE_TYPE::SKIN_PRI; }
		virtual const AABB&				GetAABB() const override { return m_AABB; }

		void							LoadFromSkinData(const std::vector<SkinVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name);
								
		bool							CreateMaterialInstance(IMaterial* pMtl, const std::vector<MaterialParam>* pMtlParams = nullptr);

		void							InitRenderData();

		void							SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

										//传入该函数的顶点数组必须保证其顶点为拆散的顶点，即顶点数为三角形数乘以3
		void							CalcTangentsAndGenIndices(std::vector<SkinVertex>& rawVertices, const std::vector<uint32_t>& smoothGroup);
		void							CalculateAABB();

		void							Update(SkinModelPerObjectTransform& perModelTransform);

	private:
		void							CloneRenderData(const SkinMesh& mesh);
		void							ReleaseRenderData();
										// 设置位姿以及每帧播动画的时候需要更新AABB
		void							UpdateAABB(const Matrix4x4& absTransMatrix);


	private:
		std::string						m_Name;

		std::vector<SkinVertex>			m_Vertices;
		std::vector<uint32_t>			m_Indices;

		SkinMeshRenderData*				m_pRenderData;

		IMaterialInstance*				m_pMtlIns;

		AABB							m_AABB;
	};

}// namespace RenderDog
