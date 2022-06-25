////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: StaticMesh.h
//Written by Xiang Weikang
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
	class Matrix4x4;

	struct StaticMeshRenderData
	{
		IVertexBuffer*		pVB;
		IIndexBuffer*		pIB;
		IConstantBuffer*	pCB;

		IShader*			pVS;
		IShader*			pPS;

		StaticMeshRenderData() :
			pVB(nullptr),
			pIB(nullptr),
			pCB(nullptr),
			pVS(nullptr),
			pPS(nullptr)
		{}
	};

	//------------------------------------------------------------------------
	//   SimpleMesh
	//------------------------------------------------------------------------

	class SimpleMesh : public IPrimitive
	{
	public:
		SimpleMesh();
		~SimpleMesh();

		virtual void					Render(IPrimitiveRenderer* pPrimitiveRenderer) override;

		virtual PRIMITIVE_TYPE			GetPriType() const override { return PRIMITIVE_TYPE::SIMPLE_PRI; }

		virtual const AABB&				GetAABB() const override { return m_AABB; }

		void							LoadFromSimpleData(const std::vector<SimpleVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name);
		void							InitRenderData(const std::string& vsFile, const std::string& psFile);
		void							ReleaseRenderData();

		void							SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

	private:
		std::string						m_Name;

		std::vector<SimpleVertex>		m_Vertices;
		std::vector<uint32_t>			m_Indices;

		StaticMeshRenderData*			m_pRenderData;

		AABB							m_AABB;
	};

	//------------------------------------------------------------------------
	//   StaticMesh
	//------------------------------------------------------------------------

	class StaticMesh : public IPrimitive
	{
	public:
		StaticMesh();
		~StaticMesh();

		StaticMesh(const std::vector<StandardVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name);

		virtual void					Render(IPrimitiveRenderer* pPrimitiveRenderer) override;

		virtual PRIMITIVE_TYPE			GetPriType() const override { return PRIMITIVE_TYPE::OPAQUE_PRI; }

		virtual const AABB&				GetAABB() const override { return m_AABB; }

		void							LoadFromStandardData(const std::vector<StandardVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name);
		bool							LoadTextureFromFile(const std::wstring& diffuseTexturePath, const std::wstring& normalTexturePath);

		void							InitRenderData(const std::string& vsFile, const std::string& psFile);
		void							ReleaseRenderData();

		void							SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

		void							CalculateTangents();
		void							CalculateAABB();
		
	private:
		// Use to update aabb when setPosGesture;
		void							UpdateAABB(const Matrix4x4& absTransMatrix);

	private:
		std::string						m_Name;

		std::vector<StandardVertex>		m_RawVertices;
		std::vector<uint32_t>			m_RawIndices;

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
