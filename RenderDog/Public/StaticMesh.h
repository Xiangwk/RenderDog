////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: StaticMesh.h
//Written by Xiang Weikang
////////////////////////////////////////

#include "Vertex.h"
#include "Primitive.h"
#include "Shader.h"
#include "Texture.h"

#include <vector>

#pragma once

namespace RenderDog
{
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

		SimpleMesh(const std::vector<SimpleVertex>& vertices, const std::vector<uint32_t>& indices);

		virtual void					Render(IPrimitiveRenderer* pPrimitiveRenderer) override;

		virtual PRIMITIVE_TYPE			GetPriType() const override { return PRIMITIVE_TYPE::SIMPLE_PRI; }

		void							LoadFromSimpleData(const std::vector<SimpleVertex>& vertices, const std::vector<uint32_t>& indices);
		void							InitRenderData(const std::string& vsFile, const std::string& psFile);
		void							ReleaseRenderData();

		void							SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

	private:
		std::vector<SimpleVertex>		m_Vertices;
		std::vector<uint32_t>			m_Indices;

		StaticMeshRenderData*			m_pRenderData;
	};

	//------------------------------------------------------------------------
	//   StaticMesh
	//------------------------------------------------------------------------

	class StaticMesh : public IPrimitive
	{
	public:
		StaticMesh();
		~StaticMesh();

		StaticMesh(const std::vector<StandardVertex>& vertices, const std::vector<uint32_t>& indices);

		virtual void					Render(IPrimitiveRenderer* pPrimitiveRenderer) override;

		virtual PRIMITIVE_TYPE			GetPriType() const override { return PRIMITIVE_TYPE::OPAQUE_PRI; }

		void							LoadFromStandardData(const std::vector<StandardVertex>& vertices, const std::vector<uint32_t>& indices);
		bool							LoadTextureFromFile(const std::wstring& diffuseTexturePath);
		void							InitRenderData(const std::string& vsFile, const std::string& psFile);
		void							ReleaseRenderData();
		void							SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);
		void							CalculateTangents();

	private:
		std::vector<StandardVertex>		m_RawVertices;
		std::vector<uint32_t>			m_RawIndices;

		std::vector<StandardVertex>		m_Vertices;
		std::vector<uint32_t>			m_Indices;

		StaticMeshRenderData*			m_pRenderData;

		ITexture2D*						m_pDiffuseTexture;
		ISamplerState*					m_pLinearSampler;
	};

}// namespace RenderDog
