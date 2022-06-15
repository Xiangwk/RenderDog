////////////////////////////////////////
//RenderDog <·,·>
//FileName: StaticMesh.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "StaticMesh.h"
#include "Matrix.h"
#include "Transform.h"
#include "Utility.h"

namespace RenderDog
{
	//------------------------------------------------------------------------
	//   SimpleMesh
	//------------------------------------------------------------------------

	SimpleMesh::SimpleMesh() :
		m_Vertices(0),
		m_Indices(0),
		m_pRenderData(nullptr),
		m_AABB()
	{}

	SimpleMesh::~SimpleMesh()
	{
		m_Vertices.clear();
		m_Indices.clear();
	}

	SimpleMesh::SimpleMesh(const std::vector<SimpleVertex>& vertices, const std::vector<uint32_t>& indices) :
		m_Vertices(0),
		m_Indices(0),
		m_pRenderData(nullptr)
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

	void SimpleMesh::Render(IPrimitiveRenderer* pPrimitiveRenderer)
	{
		PrimitiveRenderParam renderParam = {};
		renderParam.pVB				= m_pRenderData->pVB;
		renderParam.pIB				= m_pRenderData->pIB;
		renderParam.pPerObjCB		= m_pRenderData->pCB;
		renderParam.pTexture2D		= nullptr;
		renderParam.pSamplerState	= nullptr;
		renderParam.pVS				= m_pRenderData->pVS;
		renderParam.pPS				= m_pRenderData->pPS;

		pPrimitiveRenderer->Render(renderParam);
	}

	void SimpleMesh::LoadFromSimpleData(const std::vector<SimpleVertex>& vertices, const std::vector<uint32_t>& indices)
	{
		m_Vertices.assign(vertices.begin(), vertices.end());

		m_Indices.assign(indices.begin(), indices.end());
	}

	void SimpleMesh::InitRenderData(const std::string& vsFile, const std::string& psFile)
	{
		m_pRenderData = new StaticMeshRenderData();

		BufferDesc vbDesc = {};
		vbDesc.bufferBind = BUFFER_BIND::VERTEX;
		vbDesc.byteWidth = sizeof(SimpleVertex) * (uint32_t)m_Vertices.size();
		vbDesc.stride = sizeof(SimpleVertex);
		vbDesc.offset = 0;
		vbDesc.pInitData = &(m_Vertices[0]);
		vbDesc.isDynamic = false;
		m_pRenderData->pVB = (IVertexBuffer*)g_pIBufferManager->CreateBuffer(vbDesc);

		BufferDesc ibDesc = {};
		ibDesc.bufferBind = BUFFER_BIND::INDEX;
		ibDesc.byteWidth = sizeof(uint32_t) * (uint32_t)m_Indices.size();
		ibDesc.pInitData = &(m_Indices[0]);
		ibDesc.isDynamic = false;
		m_pRenderData->pIB = (IIndexBuffer*)g_pIBufferManager->CreateBuffer(ibDesc);

		BufferDesc cbDesc = {};
		cbDesc.bufferBind = BUFFER_BIND::CONSTANT;
		cbDesc.byteWidth = sizeof(Matrix4x4);
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = false;
		m_pRenderData->pCB = (IConstantBuffer*)g_pIBufferManager->CreateBuffer(cbDesc);

		ShaderCompileDesc vsDesc(vsFile, nullptr, "Main", "vs_5_0", 0);
		m_pRenderData->pVS = g_pIShaderManager->GetVertexShader(VERTEX_TYPE::SIMPLE, vsDesc);

		ShaderCompileDesc psDesc(psFile, nullptr, "Main", "ps_5_0", 0);
		m_pRenderData->pPS = g_pIShaderManager->GetPixelShader(psDesc);
	}

	void SimpleMesh::ReleaseRenderData()
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
	}

	void SimpleMesh::SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale)
	{
		Matrix4x4 transMat = GetTranslationMatrix(pos.x, pos.y, pos.z);
		Matrix4x4 rotMat = GetRotationMatrix(euler.x, euler.y, euler.z);
		Matrix4x4 scaleMat = GetScaleMatrix(scale.x, scale.y, scale.z);

		Matrix4x4 worldMat = scaleMat * rotMat * transMat;
		worldMat = worldMat;

		m_pRenderData->pCB->Update(&worldMat, sizeof(Matrix4x4));
	}

	//------------------------------------------------------------------------
	//   StaticMesh
	//------------------------------------------------------------------------

	StaticMesh::StaticMesh() :
		m_Vertices(0),
		m_Indices(0),
		m_RawVertices(0),
		m_RawIndices(0),
		m_pRenderData(nullptr),
		m_pDiffuseTexture(nullptr),
		m_pLinearSampler(nullptr),
		m_AABB()
	{}

	StaticMesh::~StaticMesh()
	{
		m_RawVertices.clear();
		m_RawIndices.clear();

		m_Vertices.clear();
		m_Indices.clear();
	}

	StaticMesh::StaticMesh(const std::vector<StandardVertex>& vertices, const std::vector<uint32_t>& indices) :
		m_Vertices(0),
		m_Indices(0),
		m_RawVertices(0),
		m_RawIndices(0),
		m_pRenderData(nullptr),
		m_pDiffuseTexture(nullptr),
		m_pLinearSampler(nullptr)
	{
		m_RawVertices.reserve(vertices.size());
		m_RawIndices.reserve(indices.size());

		for (uint32_t i = 0; i < vertices.size(); ++i)
		{
			m_RawVertices.push_back(vertices[i]);
		}

		for (uint32_t i = 0; i < indices.size(); ++i)
		{
			m_RawIndices.push_back(indices[i]);
		}
	}

	void StaticMesh::Render(IPrimitiveRenderer* pPrimitiveRenderer)
	{
		PrimitiveRenderParam renderParam = {};
		renderParam.pVB				= m_pRenderData->pVB;
		renderParam.pIB				= m_pRenderData->pIB;
		renderParam.pPerObjCB		= m_pRenderData->pCB;
		renderParam.pTexture2D		= m_pDiffuseTexture;
		renderParam.pSamplerState	= m_pLinearSampler;
		renderParam.pVS				= m_pRenderData->pVS;
		renderParam.pPS				= m_pRenderData->pPS;

		pPrimitiveRenderer->Render(renderParam);
	}

	void StaticMesh::LoadFromStandardData(const std::vector<StandardVertex>& vertices, const std::vector<uint32_t>& indices)
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
		//samplerDesc.filterMode = SamplerFilterMode::RD_SAMPLER_FILTER_LINEAR;
		samplerDesc.filterMode = SAMPLER_FILTER::POINT;
		samplerDesc.addressMode = SAMPLER_ADDRESS::WRAP;
		m_pLinearSampler = g_pISamplerStateManager->CreateSamplerState();
		if (!m_pLinearSampler->Init(samplerDesc))
		{
			return false;
		}

		return true;
	}

	void StaticMesh::InitRenderData(const std::string& vsFile, const std::string& psFile)
	{
		m_pRenderData = new StaticMeshRenderData();

		BufferDesc vbDesc = {};
		vbDesc.bufferBind = BUFFER_BIND::VERTEX;
		vbDesc.byteWidth = sizeof(StandardVertex) * (uint32_t)m_Vertices.size();
		vbDesc.stride = sizeof(StandardVertex);
		vbDesc.offset = 0;
		vbDesc.pInitData = &(m_Vertices[0]);
		vbDesc.isDynamic = false;
		m_pRenderData->pVB = (IVertexBuffer*)g_pIBufferManager->CreateBuffer(vbDesc);

		BufferDesc ibDesc = {};
		ibDesc.bufferBind = BUFFER_BIND::INDEX;
		ibDesc.byteWidth = sizeof(uint32_t) * (uint32_t)m_Indices.size();
		ibDesc.pInitData = &(m_Indices[0]);
		ibDesc.isDynamic = false;
		m_pRenderData->pIB = (IIndexBuffer*)g_pIBufferManager->CreateBuffer(ibDesc);

		BufferDesc cbDesc = {};
		cbDesc.bufferBind = BUFFER_BIND::CONSTANT;
		cbDesc.byteWidth = sizeof(Matrix4x4);
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = false;
		m_pRenderData->pCB = (IConstantBuffer*)g_pIBufferManager->CreateBuffer(cbDesc);

		ShaderCompileDesc vsDesc(vsFile, nullptr, "Main", "vs_5_0", 0);
		m_pRenderData->pVS = g_pIShaderManager->GetVertexShader(VERTEX_TYPE::STANDARD, vsDesc);

		ShaderCompileDesc psDesc(psFile, nullptr, "Main", "ps_5_0", 0);
		m_pRenderData->pPS = g_pIShaderManager->GetPixelShader(psDesc);
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

		UpdateAABB(worldMat);

		m_pRenderData->pCB->Update(&worldMat, sizeof(Matrix4x4));
	}

	void StaticMesh::CalculateTangents()
	{
		//1. 按索引拆分顶点
		std::vector<StandardVertex> tempVertices;
		tempVertices.reserve(m_RawIndices.size());

		for (uint32_t i = 0; i < m_RawIndices.size(); ++i)
		{
			tempVertices.push_back(m_RawVertices[m_RawIndices[i]]);
		}

		//2. 按三角形计算TBN
		for (uint32_t i = 0; i < tempVertices.size(); i += 3)
		{
			StandardVertex& v0 = tempVertices[i];
			StandardVertex& v1 = tempVertices[i + 1];
			StandardVertex& v2 = tempVertices[i + 2];

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

			//重新计算法线
			Vector3 faceNormal0 = CrossProduct(pos1 - pos0, pos2 - pos0);
			Vector3 faceNormal1 = CrossProduct(pos2 - pos1, pos0 - pos1);
			Vector3 faceNormal2 = CrossProduct(pos0 - pos2, pos1 - pos2);

			float rawHandParty = DotProduct(CrossProduct(faceTangent, faceBiTangent), faceNormal0) > 0.0f ? 1.0f : -1.0f;

			v0.tangent = v1.tangent = v2.tangent = Vector4(faceTangent, rawHandParty);

			v0.normal = faceNormal0;
			v1.normal = faceNormal1;
			v2.normal = faceNormal2;
		}

		//3. 合并相同的顶点
		m_Vertices.reserve(m_RawVertices.size());
		m_Indices.reserve(tempVertices.size());

		if (tempVertices.size() < 3)
		{
			return;
		}

		//加入第一个三角形
		for (uint32_t i = 0; i < 3; ++i)
		{
			m_Vertices.push_back(tempVertices[i]);
			m_Indices.push_back(i);
		}

		for (uint32_t i = 3; i < tempVertices.size(); ++i)
		{
			StandardVertex& rawVert = tempVertices[i];
			Vector3 weightedAverNormal = rawVert.normal;

			bool bHasSameVertex = false;
			uint32_t theSameIndex = 0;
			for (uint32_t j = 0; j < (uint32_t)m_Vertices.size(); ++j)
			{
				StandardVertex& vert = m_Vertices[j];
				//TEMP!!! 暂时没有光滑组信息，临时增加一个夹角大于45度则不做法线的加权平均的条件
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

		//4. 将所有顶点的T和N规范正交化
		for (uint32_t i = 0; i < m_Vertices.size(); ++i)
		{
			StandardVertex& vert = m_Vertices[i];
			vert.normal.Normalize();
			Vector3 tangent = Vector3(vert.tangent.x, vert.tangent.y, vert.tangent.z);
			tangent = Normalize(tangent - vert.normal * DotProduct(vert.normal, tangent));
			vert.tangent = Vector4(tangent, vert.tangent.w);
		}
	}

	void StaticMesh::CalculateAABB()
	{
		m_AABB.Reset();
		for (uint32_t i = 0; i < m_Vertices.size(); ++i)
		{
			const Vector3 pos = m_Vertices[i].position;
			m_AABB.minPoint.x = m_AABB.minPoint.x < pos.x ? m_AABB.minPoint.x : pos.x;
			m_AABB.minPoint.y = m_AABB.minPoint.y < pos.y ? m_AABB.minPoint.y : pos.y;
			m_AABB.minPoint.z = m_AABB.minPoint.z < pos.z ? m_AABB.minPoint.z : pos.z;

			m_AABB.maxPoint.x = m_AABB.maxPoint.x > pos.x ? m_AABB.maxPoint.x : pos.x;
			m_AABB.maxPoint.y = m_AABB.maxPoint.y > pos.y ? m_AABB.maxPoint.y : pos.y;
			m_AABB.maxPoint.z = m_AABB.maxPoint.z > pos.z ? m_AABB.maxPoint.z : pos.z;
		}
	}

	void StaticMesh::UpdateAABB(const Matrix4x4& absTransMatrix)
	{
		m_AABB.Reset();
		for (uint32_t i = 0; i < m_Vertices.size(); ++i)
		{
			Vector4 pos = Vector4(m_Vertices[i].position, 1.0f);
			pos = pos * absTransMatrix;

			m_AABB.minPoint.x = m_AABB.minPoint.x < pos.x ? m_AABB.minPoint.x : pos.x;
			m_AABB.minPoint.y = m_AABB.minPoint.y < pos.y ? m_AABB.minPoint.y : pos.y;
			m_AABB.minPoint.z = m_AABB.minPoint.z < pos.z ? m_AABB.minPoint.z : pos.z;

			m_AABB.maxPoint.x = m_AABB.maxPoint.x > pos.x ? m_AABB.maxPoint.x : pos.x;
			m_AABB.maxPoint.y = m_AABB.maxPoint.y > pos.y ? m_AABB.maxPoint.y : pos.y;
			m_AABB.maxPoint.z = m_AABB.maxPoint.z > pos.z ? m_AABB.maxPoint.z : pos.z;
		}
	}

}// namespace RenderDog