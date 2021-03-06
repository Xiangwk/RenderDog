////////////////////////////////////////
// RenderDog <·,·>
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
	struct	StaticMeshRenderData;

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
		bool							LoadTextureFromFile(const std::wstring& diffuseTexturePath, const std::wstring& normalTexturePath);

		void							InitRenderData(const std::string& vsFile, const std::string& psFile);

		void							SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

										//该函数必须保证顶点数组中为拆散的顶点，即顶点数为三角形数乘以3
		void							CalcTangentsAndGenIndices(std::vector<StandardVertex>& rawVertices, const std::vector<uint32_t>& smoothGroup);
		void							CalculateAABB();

		ITexture2D*						GetDiffuseTexture() { return m_pDiffuseTexture; }
		ISamplerState*					GetDiffuseSampler() { return m_pDiffuseTextureSampler; }

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

		ITexture2D*						m_pDiffuseTexture;
		ISamplerState*					m_pDiffuseTextureSampler;

		ITexture2D*						m_pNormalTexture;
		ISamplerState*					m_pNormalTextureSampler;

		AABB							m_AABB;
	};

}// namespace RenderDog
