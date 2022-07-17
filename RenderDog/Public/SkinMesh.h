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

#include <vector>
#include <string>

namespace RenderDog
{
	struct	SkinMeshRenderData;

	class SkinMesh : public IPrimitive
	{
	public:
		struct SkinModelPerObjectTransform
		{
			Matrix4x4			LocalToWorldMatrix;
			Matrix4x4			BoneFinalTransformMatrix[256];

			SkinModelPerObjectTransform() :
				LocalToWorldMatrix()
			{
				LocalToWorldMatrix.Identity();

				for (int i = 0; i < 256; ++i)
				{
					BoneFinalTransformMatrix[i].Identity();
				}
			}
		};

	public:
		SkinMesh();
		~SkinMesh();

		SkinMesh(const SkinMesh& mesh);
		SkinMesh& operator=(const SkinMesh& mesh);

		explicit SkinMesh(const std::string& name);

		virtual void					Render(IPrimitiveRenderer* pPrimitiveRenderer) override;
		virtual PRIMITIVE_TYPE			GetPriType() const override { return PRIMITIVE_TYPE::OPAQUE_PRI; }
		virtual const AABB&				GetAABB() const override { return m_AABB; }

		void							LoadFromSkinData(const std::vector<SkinVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name);
		
		bool							LoadTextureFromFile(const std::wstring& diffuseTexturePath, const std::wstring& normalTexturePath);

		void							InitRenderData(const std::string& vsFile, const std::string& psFile);

		void							SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

										//传入该函数的顶点数组必须保证其顶点为拆散的顶点，即顶点数为三角形数乘以3
		void							CalcTangentsAndGenIndices(std::vector<SkinVertex>& rawVertices, const std::vector<uint32_t>& smoothGroup);
		void							CalculateAABB();

		ITexture2D*						GetDiffuseTexture() { return m_pDiffuseTexture; }
		ISamplerState*					GetDiffuseSampler() { return m_pDiffuseTextureSampler; }

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

		ITexture2D*						m_pDiffuseTexture;
		ISamplerState*					m_pDiffuseTextureSampler;

		ITexture2D*						m_pNormalTexture;
		ISamplerState*					m_pNormalTextureSampler;

		AABB							m_AABB;
	};

}// namespace RenderDog
