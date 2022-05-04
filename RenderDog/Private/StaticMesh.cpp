////////////////////////////////////////
//RenderDog <��,��>
//FileName: StaticMesh.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "StaticMesh.h"
#include "Matrix.h"
#include "Transform.h"
#include "Utility.h"

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

	StaticMesh::StaticMesh(const std::vector<LocalVertex>& vertices, const std::vector<uint32_t>& indices) :
		m_Vertices(0),
		m_Indices(0),
		m_pRenderData(nullptr),
		m_pDiffuseTexture(nullptr),
		m_pLinearSampler(nullptr)
	{
		m_Vertices.reserve(vertices.size());
		m_Indices.reserve(indices.size());

		for (uint32_t i = 0; i < vertices.size(); ++i)
		{
			m_Vertices.push_back(vertices[i]);
		}

		for (uint32_t i = 0; i < indices.size(); ++i)
		{
			m_Indices.push_back(indices[i]);
		}
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
								  const std::vector<uint32_t>& indices)
	{
		m_Vertices.assign(vertices.begin(), vertices.end());

		m_Indices.assign(indices.begin(), indices.end());
	}

	bool StaticMesh::LoadTextureFromFile(const std::wstring& diffuseTexturePath)
	{
		TextureDesc texDesc;
		m_pDiffuseTexture = g_pITextureManager->CreateTexture2D(texDesc);
		if (!m_pDiffuseTexture->LoadFromFile(diffuseTexturePath))
		{
			return false;
		}

		SamplerDesc samplerDesc = {};
		samplerDesc.filterMode = SamplerFilterMode::RD_SAMPLER_FILTER_LINEAR;
		samplerDesc.addressMode = SamplerAddressMode::RD_SAMPLER_ADDRESS_WRAP;
		m_pLinearSampler = g_pISamplerStateManager->CreateSamplerState();
		if (!m_pLinearSampler->Init(samplerDesc))
		{
			return false;
		}

		return true;
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

		m_pRenderData->pVS = g_pIShaderManager->CreateVertexShader(VertexType::RD_VERTEX_TYPE_STANDARD);
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

		Matrix4x4 worldMat = scaleMat * rotMat * transMat;
		worldMat = worldMat;

		m_pRenderData->pCB->Update(&worldMat, sizeof(Matrix4x4));
	}

	void StaticMesh::CalculateTangents()
	{
		//1. ��������ֶ���
		std::vector<LocalVertex> tempVertices;
		tempVertices.reserve(m_Indices.size());

		for (uint32_t i = 0; i < m_Indices.size(); ++i)
		{
			tempVertices.push_back(m_Vertices[m_Indices[i]]);
		}

		//2. �������μ���TBN
		for (uint32_t i = 0; i < tempVertices.size(); i += 3)
		{
			LocalVertex& v0 = tempVertices[i];
			LocalVertex& v1 = tempVertices[i + 1];
			LocalVertex& v2 = tempVertices[i + 2];

			const Vector3& pos0 = v0.position;
			const Vector3& pos1 = v1.position;
			const Vector3& pos2 = v2.position;

			const Vector2& tex0 = v0.texcoord;
			const Vector2& tex1 = v1.texcoord;
			const Vector2& tex2 = v2.texcoord;

			float x1 = pos1.x - pos0.x;
			float x2 = pos2.x - pos0.x;
			float y1 = pos1.y - pos0.y;
			float y2 = pos2.y - pos0.y;
			float z1 = pos1.z - pos0.z;
			float z2 = pos2.z - pos0.z;

			float s1 = tex1.x - tex0.x;
			float s2 = tex2.x - tex0.x;
			float t1 = tex1.y - tex0.y;
			float t2 = tex2.y - tex0.y;

			Vector3 faceTangent;
			Vector3 faceBiTangent;

			float div = s1 * t2 - s2 * t1;
			if (div == 0)
			{
				faceTangent = Vector3(1.0f, 0.0f, 0.0f);
				faceBiTangent = Vector3(0.0f, 1.0f, 0.0f);
			}
			else
			{
				float inv = 1.0f / div;

				float tx = inv * (t2 * x1 - t1 * x2);
				float ty = inv * (t2 * y1 - t1 * y2);
				float tz = inv * (t2 * z1 - t1 * z2);

				float bx = inv * (s1 * x2 - s2 * x1);
				float by = inv * (s1 * y2 - s2 * y1);
				float bz = inv * (s1 * z2 - s2 * z1);

				faceTangent = Normalize(Vector3(tx, ty, tz)) * abs(div);
				faceBiTangent = Normalize(Vector3(bx, by, bz)) * abs(div);
			}

			//���¼��㷨��
			Vector3 faceNormal0 = CrossProduct(pos1 - pos0, pos2 - pos0);
			Vector3 faceNormal1 = CrossProduct(pos2 - pos1, pos0 - pos1);
			Vector3 faceNormal2 = CrossProduct(pos0 - pos2, pos1 - pos2);

			float rawHandParty = DotProduct(CrossProduct(faceTangent, faceBiTangent), faceNormal0) > 0.0f ? 1.0f : -1.0f;

			v0.tangent = v1.tangent = v2.tangent = Vector4(faceTangent, rawHandParty);

			v0.normal = faceNormal0;
			v1.normal = faceNormal1;
			v2.normal = faceNormal2;
		}

		//3. �ϲ���ͬ�Ķ���
		m_Vertices.reserve(m_Vertices.size());
		m_Indices.reserve(tempVertices.size());

		if (tempVertices.size() < 3)
		{
			return;
		}

		//�����һ��������
		for (uint32_t i = 0; i < 3; ++i)
		{
			m_Vertices.push_back(tempVertices[i]);
			m_Indices.push_back(i);
		}

		for (uint32_t i = 3; i < tempVertices.size(); ++i)
		{
			LocalVertex& rawVert = tempVertices[i];
			Vector3 weightedAverNormal = rawVert.normal;

			bool bHasSameVertex = false;
			uint32_t theSameIndex = 0;
			for (uint32_t j = 0; j < (uint32_t)m_Vertices.size(); ++j)
			{
				LocalVertex& vert = m_Vertices[j];
				//TEMP!!! ��ʱû�й⻬����Ϣ����ʱ����һ���нǴ���45���������ߵļ�Ȩƽ��������
				if (rawVert.position == vert.position &&
					rawVert.texcoord == vert.texcoord &&
					abs(DotProduct(Normalize(rawVert.normal), Normalize(vert.normal))) > 0.7f)
				{
					weightedAverNormal = rawVert.normal + vert.normal;
					vert.normal = weightedAverNormal;

					if (FloatEqual(rawVert.tangent.w, vert.tangent.w))
					{
						float w = vert.tangent.w;

						vert.tangent = vert.tangent + rawVert.tangent;
						vert.tangent.w = w;

						theSameIndex = j;

						bHasSameVertex = true;
					}
				}
			}

			if (!bHasSameVertex)
			{
				m_Indices.push_back((uint32_t)m_Vertices.size());

				rawVert.normal = weightedAverNormal;
				m_Vertices.push_back(rawVert);
			}
			else
			{
				m_Indices.push_back(theSameIndex);
			}
		}

		//4. �����ж����T��N�淶������
		for (uint32_t i = 0; i < m_Vertices.size(); ++i)
		{
			LocalVertex& vert = m_Vertices[i];
			vert.normal.Normalize();
			Vector3 tangent = Vector3(vert.tangent.x, vert.tangent.y, vert.tangent.z);
			tangent = Normalize(tangent - vert.normal * DotProduct(vert.normal, tangent));
			vert.tangent = Vector4(tangent, vert.tangent.w);
		}
	}

}// namespace RenderDog