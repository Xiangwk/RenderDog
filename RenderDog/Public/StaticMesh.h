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

	class StaticMesh : public IPrimitive
	{
	public:
		StaticMesh();
		~StaticMesh();

		virtual void Render(IPrimitiveRenderer* pPrimitiveRenderer) override;

		void LoadFromData(const std::vector<LocalVertex>& vertices, 
						  const std::vector<uint32_t>& indices, 
						  const std::wstring& diffuseTexturePath);

		void InitRenderData();
		void ReleaseRenderData();

		void SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

	private:
		std::vector<LocalVertex>	m_Vertices;
		std::vector<uint32_t>		m_Indices;

		StaticMeshRenderData*		m_pRenderData;

		ITexture2D*					m_pDiffuseTexture;
		ISamplerState*				m_pLinearSampler;
	};

}// namespace RenderDog
