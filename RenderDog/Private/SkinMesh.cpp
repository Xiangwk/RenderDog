////////////////////////////////////////
//RenderDog <·,·>
//FileName: SkinMesh.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "SkinMesh.h"
#include "Buffer.h"
#include "Shader.h"
#include "Transform.h"
#include "Utility.h"

#include <unordered_map>

namespace RenderDog
{
	struct StaticMeshRenderData
	{
		IVertexBuffer* pVB;
		IIndexBuffer* pIB;
		IConstantBuffer* pCB;

		IShader* pVS;
		IShader* pPS;

		StaticMeshRenderData() :
			pVB(nullptr),
			pIB(nullptr),
			pCB(nullptr),
			pVS(nullptr),
			pPS(nullptr)
		{}
	};

	struct VertexKey
	{
		Vector3		pos;
		uint32_t	smooth;

		bool operator==(const VertexKey& rhs) const
		{
			return (pos == rhs.pos && smooth == rhs.smooth);
		}
	};
}// namespace RenderDog

namespace std
{
	template<>
	struct hash<RenderDog::VertexKey>
	{
		typedef size_t				result_type;
		typedef RenderDog::Vector3	argument_type;

		size_t operator()(const RenderDog::VertexKey& vk) const
		{
			return hash<float>()(vk.pos.x)
				^ hash<float>()(vk.pos.y)
				^ hash<float>()(vk.pos.z)
				^ hash<uint32_t>()(vk.smooth);
		}
	};
}// namespace std

namespace RenderDog
{
	//FIXME!!! 如果后续发现RenderData可以和StaticMesh复用，则修改一下
	struct SkinMeshRenderData
	{
		IVertexBuffer*		pVB;
		IIndexBuffer*		pIB;
		IConstantBuffer*	pCB;

		IShader*			pVS;

		SkinMeshRenderData() :
			pVB(nullptr),
			pIB(nullptr),
			pCB(nullptr),
			pVS(nullptr)
		{}
	};

	SkinMesh::SkinMesh():
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

	SkinMesh::~SkinMesh()
	{
		ReleaseRenderData();

		m_Vertices.clear();
		m_Indices.clear();
	}

	SkinMesh::SkinMesh(const SkinMesh& mesh) :
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

	SkinMesh& SkinMesh::operator=(const SkinMesh& mesh)
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

	SkinMesh::SkinMesh(const std::string& name) :
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

	void SkinMesh::Render(IPrimitiveRenderer* pPrimitiveRenderer)
	{
		PrimitiveRenderParam renderParam = {};
		renderParam.pVB						= m_pRenderData->pVB;
		renderParam.pIB						= m_pRenderData->pIB;
		renderParam.pDiffuseTexture			= m_pDiffuseTexture;
		renderParam.pDiffuseTextureSampler	= m_pDiffuseTextureSampler;
		renderParam.pNormalTexture			= m_pNormalTexture;
		renderParam.pNormalTextureSampler	= m_pNormalTextureSampler;
		renderParam.pVS						= m_pRenderData->pVS;

		pPrimitiveRenderer->Render(renderParam);
	}

	void SkinMesh::LoadFromSkinData(const std::vector<SkinVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name)
	{
		m_Vertices.assign(vertices.begin(), vertices.end());

		m_Indices.assign(indices.begin(), indices.end());

		m_Name = name;
	}

	bool SkinMesh::LoadTextureFromFile(const std::wstring& diffuseTexturePath, const std::wstring& normalTexturePath)
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

	void SkinMesh::InitRenderData()
	{
		m_pRenderData = new SkinMeshRenderData();

		BufferDesc vbDesc = {};
		vbDesc.name = m_Name + "_Skin" + "_VertexBuffer";
		vbDesc.byteWidth = sizeof(SkinVertex) * (uint32_t)m_Vertices.size();
		vbDesc.stride = sizeof(SkinVertex);
		vbDesc.offset = 0;
		vbDesc.pInitData = &(m_Vertices[0]);
		vbDesc.isDynamic = false;
		m_pRenderData->pVB = (IVertexBuffer*)g_pIBufferManager->GetVertexBuffer(vbDesc);

		BufferDesc ibDesc = {};
		ibDesc.name = m_Name + "_Skin" + "_IndexBuffer";
		ibDesc.byteWidth = sizeof(uint32_t) * (uint32_t)m_Indices.size();
		ibDesc.pInitData = &(m_Indices[0]);
		ibDesc.isDynamic = false;
		m_pRenderData->pIB = (IIndexBuffer*)g_pIBufferManager->GetIndexBuffer(ibDesc);

		BufferDesc cbDesc = {};
		cbDesc.name = m_Name + "_Skin" + "_ConstantBuffer";
		cbDesc.byteWidth = sizeof(SkinModelPerObjectTransform);
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = true;
		m_pRenderData->pCB = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);

		ShaderCompileDesc vsDesc(g_SkinModelVertexShaderFilePath, nullptr, "Main", "vs_5_0", 0);
		m_pRenderData->pVS = g_pIShaderManager->GetModelVertexShader(VERTEX_TYPE::SKIN, vsDesc);
	}

	void SkinMesh::SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale)
	{
		Matrix4x4 transMat = GetTranslationMatrix(pos.x, pos.y, pos.z);
		Matrix4x4 rotMat = GetRotationMatrix(euler.x, euler.y, euler.z);
		Matrix4x4 scaleMat = GetScaleMatrix(scale.x, scale.y, scale.z);

		Matrix4x4 worldMat = scaleMat * rotMat * transMat;

		UpdateAABB(worldMat);
	}

	void SkinMesh::CalcTangentsAndGenIndices(std::vector<SkinVertex>& rawVertices, const std::vector<uint32_t>& smoothGroup)
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

			//对于SkinModel来说，坐标系转换矩阵作用在根骨骼上，因此这里仍然按照3dsMax中的右手坐标系来计算面法线
			Vector3 faceNormal0 = CrossProduct(pos2 - pos0, pos1 - pos0);
			Vector3 faceNormal1 = CrossProduct(pos0 - pos1, pos2 - pos1);
			Vector3 faceNormal2 = CrossProduct(pos1 - pos2, pos0 - pos2);


			float rawHandParty = DotProduct(CrossProduct(faceTangent, faceBiTangent), faceNormal0) > 0.0f ? 1.0f : -1.0f;

			v0.tangent = v1.tangent = v2.tangent = Vector4(faceTangent, rawHandParty);

			v0.normal = faceNormal0;
			v1.normal = faceNormal1;
			v2.normal = faceNormal2;
		}

		//2. 合并相同的顶点
		m_Vertices.reserve(rawVertices.size());
		m_Indices.reserve(rawVertices.size());

		if (rawVertices.size() < 3)
		{
			return;
		}

		struct VertexValue
		{
			SkinVertex*			pVert;
			uint32_t			index;
		};

		std::unordered_multimap<VertexKey, VertexValue> vertMap;
		//加入第一个三角形
		for (uint32_t i = 0; i < 3; ++i)
		{
			m_Vertices.push_back(rawVertices[i]);
			m_Indices.push_back(i);

			vertMap.insert({ {m_Vertices[i].position, smoothGroup[i]}, {&m_Vertices[i], i} });
		}

		for (uint32_t i = 3; i < rawVertices.size(); ++i)
		{
			SkinVertex& rawVert = rawVertices[i];
			uint32_t rawSmooth = smoothGroup[i];

			Vector3 weightedAverNormal = rawVert.normal;

			size_t sameVertsNum = vertMap.count({ rawVert.position, rawSmooth });
			auto findedVert = vertMap.find({ rawVert.position, rawSmooth });
			if (sameVertsNum == 1)
			{
				//HashMap中只找到一个相同点，则有可能手相性不同，若相同则合并，否则加入到新的顶点数组以及HashMap中
				SkinVertex* pTheSameVert = findedVert->second.pVert;
				weightedAverNormal = rawVert.normal + pTheSameVert->normal;
				pTheSameVert->normal = weightedAverNormal;

				if (FloatEqual(pTheSameVert->tangent.w, rawVert.tangent.w))
				{
					float w = pTheSameVert->tangent.w;

					pTheSameVert->tangent = pTheSameVert->tangent + rawVert.tangent;
					pTheSameVert->tangent.w = w;

					m_Indices.push_back(findedVert->second.index);
				}
				else
				{
					uint32_t newIndex = (uint32_t)m_Vertices.size();
					m_Indices.push_back(newIndex);

					//手相性不同时，仅计算法线的加权平均
					rawVert.normal = weightedAverNormal;
					m_Vertices.push_back(rawVert);

					SkinVertex* pNewVert = &m_Vertices[newIndex];
					vertMap.insert({ { pNewVert->position, rawSmooth }, { pNewVert, newIndex} });
				}
			}
			else if (sameVertsNum > 1)
			{
				//相同点的数量大于1，则其中一定有一个手相性相同的可以合并
				for (size_t n = 0; n < sameVertsNum; ++n)
				{
					SkinVertex* pTheSameVert = findedVert->second.pVert;
					weightedAverNormal = rawVert.normal + pTheSameVert->normal;
					pTheSameVert->normal = weightedAverNormal;

					if (FloatEqual(pTheSameVert->tangent.w, rawVert.tangent.w))
					{
						float w = pTheSameVert->tangent.w;

						pTheSameVert->tangent = pTheSameVert->tangent + rawVert.tangent;
						pTheSameVert->tangent.w = w;

						m_Indices.push_back(findedVert->second.index);
					}

					findedVert++;
				}
			}
			else
			{
				//没有找到相同点，直接加入到新的顶点数组以及HashMap中
				uint32_t newIndex = (uint32_t)m_Vertices.size();
				m_Indices.push_back(newIndex);

				m_Vertices.push_back(rawVert);

				SkinVertex* pNewVert = &m_Vertices[newIndex];
				vertMap.insert({ { pNewVert->position, rawSmooth }, { pNewVert, newIndex} });
			}
		}

		//3. 将所有顶点的T和N规范正交化
		for (uint32_t i = 0; i < m_Vertices.size(); ++i)
		{
			SkinVertex& vert = m_Vertices[i];
			vert.normal.Normalize();
			Vector3 tangent = Vector3(vert.tangent.x, vert.tangent.y, vert.tangent.z);
			tangent = Normalize(tangent - vert.normal * DotProduct(vert.normal, tangent));
			vert.tangent = Vector4(tangent, vert.tangent.w);
		}
	}

	void SkinMesh::CalculateAABB()
	{
		//FIXME!!! 后续还要考虑绑骨以及骨骼动画
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

	void SkinMesh::Update(SkinModelPerObjectTransform& perModelTransform)
	{
		m_pRenderData->pCB->Update(&perModelTransform, sizeof(SkinModelPerObjectTransform));
	}
	

	void SkinMesh::CloneRenderData(const SkinMesh& mesh)
	{
		if (mesh.m_pRenderData)
		{
			InitRenderData();
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

	void SkinMesh::ReleaseRenderData()
	{
		if (m_pRenderData)
		{
			m_pRenderData->pVB->Release();
			m_pRenderData->pIB->Release();
			m_pRenderData->pCB->Release();
			m_pRenderData->pVS->Release();

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

	void SkinMesh::UpdateAABB(const Matrix4x4& absTransMatrix)
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