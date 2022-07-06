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
		m_Name(""),
		m_Vertices(0),
		m_Indices(0),
		m_pRenderData(nullptr),
		m_AABB()
	{}

	SimpleMesh::~SimpleMesh()
	{
		ReleaseRenderData();

		m_Vertices.clear();
		m_Indices.clear();
	}

	SimpleMesh::SimpleMesh(const SimpleMesh& mesh):
		m_Name(mesh.m_Name),
		m_Vertices(mesh.m_Vertices),
		m_Indices(mesh.m_Indices),
		m_pRenderData(nullptr),
		m_AABB(mesh.m_AABB)
	{
		CloneRenderData(mesh);
	}

	SimpleMesh& SimpleMesh::operator=(const SimpleMesh& mesh)
	{
		if (&mesh == this)
		{
			return *this;
		}

		m_Name = mesh.m_Name;
		m_Vertices = mesh.m_Vertices;
		m_Indices = mesh.m_Indices;
		m_AABB = mesh.m_AABB;

		ReleaseRenderData();

		CloneRenderData(mesh);

		return *this;
	}

	void SimpleMesh::Render(IPrimitiveRenderer* pPrimitiveRenderer)
	{
		PrimitiveRenderParam renderParam = {};
		renderParam.pVB						= m_pRenderData->pVB;
		renderParam.pIB						= m_pRenderData->pIB;
		renderParam.pPerObjCB				= m_pRenderData->pCB;
		renderParam.pDiffuseTexture			= nullptr;
		renderParam.pDiffuseTextureSampler	= nullptr;
		renderParam.pVS						= m_pRenderData->pVS;
		renderParam.pPS						= m_pRenderData->pPS;

		pPrimitiveRenderer->Render(renderParam);
	}

	void SimpleMesh::LoadFromSimpleData(const std::vector<SimpleVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name)
	{
		m_Vertices.assign(vertices.begin(), vertices.end());

		m_Indices.assign(indices.begin(), indices.end());

		m_Name = name;
	}

	void SimpleMesh::InitRenderData(const std::string& vsFile, const std::string& psFile)
	{
		m_pRenderData = new StaticMeshRenderData();

		BufferDesc vbDesc = {};
		vbDesc.name = m_Name + "_VertexBuffer";
		vbDesc.byteWidth = sizeof(SimpleVertex) * (uint32_t)m_Vertices.size();
		vbDesc.stride = sizeof(SimpleVertex);
		vbDesc.offset = 0;
		vbDesc.pInitData = &(m_Vertices[0]);
		vbDesc.isDynamic = false;
		m_pRenderData->pVB = (IVertexBuffer*)g_pIBufferManager->GetVertexBuffer(vbDesc);

		BufferDesc ibDesc = {};
		ibDesc.name = m_Name + "_IndexBuffer";
		ibDesc.byteWidth = sizeof(uint32_t) * (uint32_t)m_Indices.size();
		ibDesc.pInitData = &(m_Indices[0]);
		ibDesc.isDynamic = false;
		m_pRenderData->pIB = (IIndexBuffer*)g_pIBufferManager->GetIndexBuffer(ibDesc);

		BufferDesc cbDesc = {};
		cbDesc.name = m_Name + "_ConstantBuffer";
		cbDesc.byteWidth = sizeof(Matrix4x4);
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = false;
		m_pRenderData->pCB = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);

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

	void SimpleMesh::CloneRenderData(const SimpleMesh& mesh)
	{
		if (mesh.m_pRenderData)
		{
			const std::string& vsFile = mesh.m_pRenderData->pVS->GetFileName();
			const std::string& psFile = mesh.m_pRenderData->pPS->GetFileName();

			InitRenderData(vsFile, psFile);
		}
	}

	//------------------------------------------------------------------------
	//   StaticMesh
	//------------------------------------------------------------------------

	StaticMesh::StaticMesh() :
		m_Name(""),
		m_Vertices(0),
		m_Indices(0),
		m_pRenderData(nullptr),
		m_pDiffuseTexture(nullptr),
		m_pDiffuseTextureSampler(nullptr),
		m_pNormalTexture(nullptr),
		m_pNormalTextureSampler(nullptr),
		m_AABB()
	{}

	StaticMesh::~StaticMesh()
	{
		ReleaseRenderData();

		m_Vertices.clear();
		m_Indices.clear();
	}

	StaticMesh::StaticMesh(const StaticMesh& mesh):
		m_Name(mesh.m_Name),
		m_Vertices(mesh.m_Vertices),
		m_Indices(mesh.m_Indices),
		m_pRenderData(nullptr),
		m_pDiffuseTexture(nullptr),
		m_pDiffuseTextureSampler(nullptr),
		m_pNormalTexture(nullptr),
		m_pNormalTextureSampler(nullptr),
		m_AABB(mesh.m_AABB)
	{
		CloneRenderData(mesh);
	}

	StaticMesh& StaticMesh::operator=(const StaticMesh& mesh)
	{
		if (&mesh == this)
		{
			return *this;
		}

		m_Name		= mesh.m_Name;
		m_Vertices	= mesh.m_Vertices;
		m_Indices	= mesh.m_Indices;
		m_AABB		= mesh.m_AABB;

		ReleaseRenderData();

		CloneRenderData(mesh);

		return *this;
	}

	StaticMesh::StaticMesh(const std::string& name):
		m_Name(name),
		m_Vertices(0),
		m_Indices(0),
		m_pRenderData(nullptr),
		m_pDiffuseTexture(nullptr),
		m_pDiffuseTextureSampler(nullptr),
		m_pNormalTexture(nullptr),
		m_pNormalTextureSampler(nullptr),
		m_AABB()
	{}

	void StaticMesh::Render(IPrimitiveRenderer* pPrimitiveRenderer)
	{
		PrimitiveRenderParam renderParam = {};
		renderParam.pVB						= m_pRenderData->pVB;
		renderParam.pIB						= m_pRenderData->pIB;
		renderParam.pPerObjCB				= m_pRenderData->pCB;
		renderParam.pDiffuseTexture			= m_pDiffuseTexture;
		renderParam.pDiffuseTextureSampler	= m_pDiffuseTextureSampler;
		renderParam.pNormalTexture			= m_pNormalTexture;
		renderParam.pNormalTextureSampler	= m_pNormalTextureSampler;
		renderParam.pVS						= m_pRenderData->pVS;
		renderParam.pPS						= m_pRenderData->pPS;

		pPrimitiveRenderer->Render(renderParam);
	}

	void StaticMesh::LoadFromStandardData(const std::vector<StandardVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name)
	{
		m_Vertices.assign(vertices.begin(), vertices.end());

		m_Indices.assign(indices.begin(), indices.end());

		m_Name = name;
	}

	bool StaticMesh::LoadTextureFromFile(const std::wstring& diffuseTexturePath, const std::wstring& normalTexturePath)
	{
		if (!diffuseTexturePath.empty())
		{
			m_pDiffuseTexture = g_pITextureManager->CreateTexture2D(diffuseTexturePath);
			if (!m_pDiffuseTexture)
			{
				return false;
			}

			SamplerDesc samplerDesc = {};
			samplerDesc.filterMode = SAMPLER_FILTER::LINEAR;
			samplerDesc.addressMode = SAMPLER_ADDRESS::WRAP;
			m_pDiffuseTextureSampler = g_pISamplerStateManager->CreateSamplerState(samplerDesc);
			if (!m_pDiffuseTextureSampler)
			{
				return false;
			}
		}

		if (!normalTexturePath.empty())
		{
			m_pNormalTexture = g_pITextureManager->CreateTexture2D(normalTexturePath);
			if (!m_pNormalTexture)
			{
				return false;
			}

			SamplerDesc samplerDesc = {};
			samplerDesc.filterMode = SAMPLER_FILTER::LINEAR;
			samplerDesc.addressMode = SAMPLER_ADDRESS::WRAP;
			m_pNormalTextureSampler = g_pISamplerStateManager->CreateSamplerState(samplerDesc);
			if (!m_pNormalTextureSampler)
			{
				return false;
			}
		}

		return true;
	}

	void StaticMesh::InitRenderData(const std::string& vsFile, const std::string& psFile)
	{
		m_pRenderData = new StaticMeshRenderData();

		BufferDesc vbDesc = {};
		vbDesc.byteWidth = sizeof(StandardVertex) * (uint32_t)m_Vertices.size();
		vbDesc.stride = sizeof(StandardVertex);
		vbDesc.offset = 0;
		vbDesc.pInitData = &(m_Vertices[0]);
		vbDesc.isDynamic = false;
		m_pRenderData->pVB = (IVertexBuffer*)g_pIBufferManager->GetVertexBuffer(vbDesc);

		BufferDesc ibDesc = {};
		ibDesc.byteWidth = sizeof(uint32_t) * (uint32_t)m_Indices.size();
		ibDesc.pInitData = &(m_Indices[0]);
		ibDesc.isDynamic = false;
		m_pRenderData->pIB = (IIndexBuffer*)g_pIBufferManager->GetIndexBuffer(ibDesc);

		BufferDesc cbDesc = {};
		cbDesc.byteWidth = sizeof(Matrix4x4);
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = false;
		m_pRenderData->pCB = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);

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
			m_pDiffuseTexture = nullptr;
		}

		if (m_pDiffuseTextureSampler)
		{
			m_pDiffuseTextureSampler->Release();
			m_pDiffuseTextureSampler = nullptr;
		}

		if (m_pNormalTexture)
		{
			m_pNormalTexture->Release();
			m_pNormalTexture = nullptr;
		}

		if (m_pNormalTextureSampler)
		{
			m_pNormalTextureSampler->Release();
			m_pNormalTextureSampler = nullptr;
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

	void StaticMesh::CalcTangentsAndGenIndices(std::vector<StandardVertex>& rawVertices, const std::vector<uint32_t>& smoothGroup)
	{
		//1. 按三角形计算TBN
		for (uint32_t i = 0; i < rawVertices.size(); i += 3)
		{
			StandardVertex& v0 = rawVertices[i];
			StandardVertex& v1 = rawVertices[i + 1];
			StandardVertex& v2 = rawVertices[i + 2];

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

		//2. 合并相同的顶点
		m_Vertices.reserve(rawVertices.size());
		m_Indices.reserve(rawVertices.size());

		//NOTE!!! newSmoothIndices的数量与顶点数相同对应
		std::vector<uint32_t> newSmoothGroup;
		newSmoothGroup.reserve(rawVertices.size());  

		if (rawVertices.size() < 3)
		{
			return;
		}

		//加入第一个三角形
		for (uint32_t i = 0; i < 3; ++i)
		{
			m_Vertices.push_back(rawVertices[i]);
			m_Indices.push_back(i);
			newSmoothGroup.push_back(smoothGroup[i]);
		}

		for (uint32_t i = 3; i < rawVertices.size(); ++i)
		{
			uint32_t rawSmooth = smoothGroup[i];

			StandardVertex& rawVert = rawVertices[i];
			Vector3 weightedAverNormal = rawVert.normal;

			bool bHasSameVertex = false;
			uint32_t theSameIndex = 0;
			for (uint32_t j = 0; j < (uint32_t)m_Vertices.size(); ++j)
			{
				StandardVertex& vert = m_Vertices[j];
				uint32_t newSmooth = newSmoothGroup[j];
				if (rawVert.position == vert.position && rawSmooth == newSmooth)
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
				newSmoothGroup.push_back(rawSmooth);
			}
			else
			{
				m_Indices.push_back(theSameIndex);
			}
		}

		//3. 将所有顶点的T和N规范正交化
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

	void StaticMesh::CloneRenderData(const StaticMesh& mesh)
	{
		if (mesh.m_pRenderData)
		{
			const std::string& vsFile = mesh.m_pRenderData->pVS->GetFileName();
			const std::string& psFile = mesh.m_pRenderData->pPS->GetFileName();

			InitRenderData(vsFile, psFile);
		}

		if (mesh.m_pDiffuseTexture)
		{
			const std::wstring& diffTexName = mesh.m_pDiffuseTexture->GetName();
			m_pDiffuseTexture = g_pITextureManager->CreateTexture2D(diffTexName);

			SamplerDesc samplerDesc = {};
			samplerDesc.filterMode = SAMPLER_FILTER::LINEAR;
			samplerDesc.addressMode = SAMPLER_ADDRESS::WRAP;
			m_pDiffuseTextureSampler = g_pISamplerStateManager->CreateSamplerState(samplerDesc);
		}

		if (mesh.m_pNormalTexture)
		{
			const std::wstring& normTexName = mesh.m_pNormalTexture->GetName();
			m_pNormalTexture = g_pITextureManager->CreateTexture2D(normTexName);

			SamplerDesc samplerDesc = {};
			samplerDesc.filterMode = SAMPLER_FILTER::LINEAR;
			samplerDesc.addressMode = SAMPLER_ADDRESS::WRAP;
			m_pNormalTextureSampler = g_pISamplerStateManager->CreateSamplerState(samplerDesc);
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