////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: StaticMesh.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "StaticMesh.h"
#include "Matrix.h"
#include "Transform.h"

namespace RenderDog
{
	StaticMesh::StaticMesh() :
		m_Vertices(0),
		m_Indices(0),
		m_pRenderData(nullptr),
		m_pDiffuseTexture(nullptr),
		m_pLinearSampler(nullptr)
	{}

	StaticMesh::~StaticMesh()
	{
		m_Vertices.clear();
		m_Indices.clear();
	}

	void StaticMesh::Render(IPrimitiveRenderer* pPrimitiveRenderer)
	{
		PrimitiveRenderParam renderParam = {};
		renderParam.pVB			= m_pRenderData->pVB;
		renderParam.pIB			= m_pRenderData->pIB;
		renderParam.pGlobalCB	= pPrimitiveRenderer->GetVSConstantBuffer();
		renderParam.pPerObjCB	= m_pRenderData->pCB;
		renderParam.pLightingCB = pPrimitiveRenderer->GetLightingConstantbuffer();
		renderParam.pVS			= m_pRenderData->pVS;
		renderParam.pPS			= m_pRenderData->pPS;

		pPrimitiveRenderer->Render(renderParam, m_pDiffuseTexture, m_pLinearSampler);
	}

	void StaticMesh::LoadFromData(const std::vector<LocalVertex>& vertices, 
								  const std::vector<uint32_t>& indices, 
								  const std::wstring& diffuseTexturePath)
	{
		m_Vertices.assign(vertices.begin(), vertices.end());

		m_Indices.assign(indices.begin(), indices.end());

		TextureDesc texDesc;
		m_pDiffuseTexture = g_pITextureManager->CreateTexture2D(texDesc);
		m_pDiffuseTexture->LoadFromFile(diffuseTexturePath);

		SamplerDesc samplerDesc = {};
		samplerDesc.filterMode = SamplerFilterMode::RD_SAMPLER_FILTER_LINEAR;
		samplerDesc.addressMode = SamplerAddressMode::RD_SAMPLER_ADDRESS_WRAP;
		m_pLinearSampler = g_pISamplerStateManager->CreateSamplerState();
		m_pLinearSampler->Init(samplerDesc);
	}

	void StaticMesh::InitRenderData()
	{
		m_pRenderData = new StaticMeshRenderData();

		BufferDesc vbDesc = {};
		vbDesc.bufferBind = BufferBind::VERTEX;
		vbDesc.byteWidth = sizeof(LocalVertex) * (uint32_t)m_Vertices.size();
		vbDesc.stride = sizeof(LocalVertex);
		vbDesc.offset = 0;
		vbDesc.pInitData = &(m_Vertices[0]);
		vbDesc.isDynamic = false;
		m_pRenderData->pVB = (IVertexBuffer*)g_pIBufferManager->CreateBuffer(vbDesc);

		BufferDesc ibDesc = {};
		ibDesc.bufferBind = BufferBind::INDEX;
		ibDesc.byteWidth = sizeof(uint32_t) * (uint32_t)m_Indices.size();
		ibDesc.pInitData = &(m_Indices[0]);
		ibDesc.isDynamic = false;
		m_pRenderData->pIB = (IIndexBuffer*)g_pIBufferManager->CreateBuffer(ibDesc);

		BufferDesc cbDesc = {};
		cbDesc.bufferBind = BufferBind::CONSTANT;
		cbDesc.byteWidth = sizeof(Matrix4x4);
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = false;
		m_pRenderData->pCB = (IConstantBuffer*)g_pIBufferManager->CreateBuffer(cbDesc);

		m_pRenderData->pVS = g_pIShaderManager->CreateVertexShader(RD_VERTEX_TYPE_STANDARD);
		m_pRenderData->pVS->CompileFromFile("Shaders/StaticModelVertexShader.hlsl", nullptr, "Main", "vs_5_0", 0);
		m_pRenderData->pVS->Init();

		m_pRenderData->pPS = g_pIShaderManager->CreatePixelShader();
		m_pRenderData->pPS->CompileFromFile("Shaders/PhongLightingPixelShader.hlsl", nullptr, "Main", "ps_5_0", 0);
		m_pRenderData->pPS->Init();
	}

	void StaticMesh::ReleaseRenderData()
	{
		if (m_pRenderData)
		{
			m_pRenderData->pVB->Release();
			m_pRenderData->pIB->Release();
			m_pRenderData->pCB->Release();
			m_pRenderData->pVS->Release();
			m_pRenderData->pPS->Release();

			delete m_pRenderData;
			m_pRenderData = nullptr;
		}

		if (m_pDiffuseTexture)
		{
			m_pDiffuseTexture->Release();

			g_pITextureManager->ReleaseTexture(m_pDiffuseTexture);
		}

		if (m_pLinearSampler)
		{
			m_pLinearSampler->Release();

			g_pISamplerStateManager->ReleaseSamplerState(m_pLinearSampler);
		}
	}

	void StaticMesh::SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale)
	{
		Matrix4x4 transMat = GetTranslationMatrix(pos.x, pos.y, pos.z);
		Matrix4x4 rotMat = GetRotationMatrix(euler.x, euler.y, euler.z);
		Matrix4x4 scaleMat = GetScaleMatrix(scale.x, scale.y, scale.z);

		Matrix4x4 worldMat = transMat * rotMat * scaleMat;

		m_pRenderData->pCB->Update(&worldMat, sizeof(Matrix4x4));
	}

}// namespace RenderDog