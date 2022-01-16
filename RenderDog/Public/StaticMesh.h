////////////////////////////////////////
//RenderDog <��,��>
//FileName: StaticMesh.h
//Written by Xiang Weikang
////////////////////////////////////////

#include "Vertex.h"
#include "Primitive.h"
#include "Shader.h"

#include <vector>

#pragma once

namespace RenderDog
{
	struct StaticMeshRenderData
	{
		IVertexBuffer*	pVB;
		IIndexBuffer*	pIB;

		IShader*		pVS;
		IShader*		pPS;

		StaticMeshRenderData() :
			pVB(nullptr),
			pIB(nullptr),
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

		void LoadFromData(const std::vector<LocalVertex>& vertices, const std::vector<uint32_t>& indices);

		void InitRenderData();
		void ReleaseRenderData();

	private:
		std::vector<LocalVertex>	m_Vertices;
		std::vector<uint32_t>		m_Indices;

		StaticMeshRenderData*		m_pRenderData;
	};

}// namespace RenderDog
