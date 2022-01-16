////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: StaticMesh.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "StaticMesh.h"

namespace RenderDog
{
	StaticMesh::StaticMesh() :
		m_Vertices(0),
		m_Indices(0),
		m_pRenderData(nullptr)
	{}

	StaticMesh::~StaticMesh()
	{
		m_Vertices.clear();
		m_Indices.clear();
	}

	void StaticMesh::Render(IPrimitiveRenderer* pPrimitiveRenderer)
	{
		PrimitiveRenderParam renderParam = {};
		renderParam.pVB = m_pRenderData->pVB;
		renderParam.pIB = m_pRenderData->pIB;
		renderParam.pVS = m_pRenderData->pVS;
		renderParam.pPS = m_pRenderData->pPS;

		pPrimitiveRenderer->Render(renderParam);
	}

	void StaticMesh::LoadFromData(const std::vector<LocalVertex>& vertices, const std::vector<uint32_t>& indices)
	{
		m_Vertices.assign(vertices.begin(), vertices.end());

		m_Indices.assign(indices.begin(), indices.end());
	}

	void StaticMesh::InitRenderData()
	{
		m_pRenderData = new StaticMeshRenderData();

		BufferDesc vbDesc = {};
		vbDesc.byteWidth = sizeof(LocalVertex) * (uint32_t)m_Vertices.size();
		vbDesc.pInitData = &(m_Vertices[0]);
		vbDesc.isDynamic = false;
		m_pRenderData->pVB = g_pIBufferManager->CreateVertexBuffer();
		if (m_pRenderData->pVB)
		{
			m_pRenderData->pVB->Init(vbDesc, sizeof(LocalVertex), 0);
		}

		BufferDesc ibDesc = {};
		ibDesc.byteWidth = sizeof(uint32_t) * (uint32_t)m_Indices.size();
		ibDesc.pInitData = &(m_Indices[0]);
		ibDesc.isDynamic = false;
		m_pRenderData->pIB = g_pIBufferManager->CreateIndexBuffer();
		if (m_pRenderData->pIB)
		{
			m_pRenderData->pIB->Init(ibDesc, (uint32_t)m_Indices.size());
		}

		m_pRenderData->pVS = g_pIShaderManager->CreateVertexShader(RD_VERTEX_TYPE_STANDARD);
		m_pRenderData->pVS->CompileFromFile("Shaders/SingleColor.hlsl", nullptr, "VS", "vs_5_0", 0);
		m_pRenderData->pVS->Init();

		m_pRenderData->pPS = g_pIShaderManager->CreatePixelShader();
		m_pRenderData->pPS->CompileFromFile("Shaders/SingleColor.hlsl", nullptr, "PS", "ps_5_0", 0);
		m_pRenderData->pPS->Init();
	}

	void StaticMesh::ReleaseRenderData()
	{
		if (m_pRenderData)
		{
			m_pRenderData->pVB->Release();
			m_pRenderData->pIB->Release();
			m_pRenderData->pVS->Release();
			m_pRenderData->pPS->Release();

			delete m_pRenderData;
			m_pRenderData = nullptr;
		}
	}



}// namespace RenderDog