////////////////////////////////////////
// RenderDog <·,·>
// FileName: StaticMesh.cpp
// Written by Xiang Weikang
////////////////////////////////////////

#include "StaticMesh.h"
#include "Matrix.h"
#include "Transform.h"
#include "Utility.h"
#include "Material.h"

#include <unordered_map>

namespace RenderDog
{
	struct VertexKey
	{
		Vector3		pos;
		Vector2		tex;
		uint32_t	smooth;

		bool operator==(const VertexKey& rhs) const
		{
			return (pos == rhs.pos && tex == rhs.tex && smooth == rhs.smooth);
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
	StaticMesh::StaticMesh() :
		m_Name(""),
		m_Vertices(0),
		m_Indices(0),
		m_pRenderData(nullptr),
		m_pMtlIns(nullptr),
		m_AABB()
	{}

	StaticMesh::StaticMesh(const StaticMesh& mesh) :
		m_Name(mesh.m_Name),
		m_Vertices(mesh.m_Vertices),
		m_Indices(mesh.m_Indices),
		m_pRenderData(nullptr),
		m_pMtlIns(nullptr),
		m_AABB(mesh.m_AABB)
	{
		CloneRenderData(mesh);
	}

	StaticMesh::StaticMesh(const std::string& name) :
		m_Name(name),
		m_Vertices(0),
		m_Indices(0),
		m_pRenderData(nullptr),
		m_pMtlIns(nullptr),
		m_AABB()
	{}

	StaticMesh::~StaticMesh()
	{
		ReleaseRenderData();

		m_Vertices.clear();
		m_Indices.clear();
	}

	StaticMesh& StaticMesh::operator=(const StaticMesh& mesh)
	{
		if (&mesh == this)
		{
			return *this;
		}

		m_Name					= mesh.m_Name;
		m_Vertices				= mesh.m_Vertices;
		m_Indices				= mesh.m_Indices;
		m_AABB					= mesh.m_AABB;

		ReleaseRenderData();

		CloneRenderData(mesh);

		return *this;
	}

	void StaticMesh::Render(IPrimitiveRenderer* pPrimitiveRenderer)
	{
		PrimitiveRenderParam renderParam = {};
		renderParam.pVB							= m_pRenderData->pVB;
		renderParam.pIB							= m_pRenderData->pIB;
		renderParam.pMtlIns						= m_pMtlIns;
		renderParam.pVS							= m_pRenderData->pVS;
		renderParam.pShadowVS					= m_pRenderData->pShadowVS;
		renderParam.PerObjParam.pPerObjectCB	= m_pRenderData->pLocalToWorldCB;

		pPrimitiveRenderer->Render(renderParam);
	}

	void StaticMesh::LoadFromStandardData(const std::vector<StandardVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name)
	{
		m_Vertices.assign(vertices.begin(), vertices.end());

		m_Indices.assign(indices.begin(), indices.end());

		m_Name = name;
	}

	bool StaticMesh::CreateMaterialInstance(IMaterial* pMtl)
	{
		m_pMtlIns = g_pMaterialManager->GetMaterialInstance(pMtl);

		return true;
	}

	void StaticMesh::InitRenderData()
	{
		m_pRenderData = new StaticMeshRenderData();

		BufferDesc vbDesc = {};
		vbDesc.name = m_Name + "_Static" + "_VertexBuffer";
		vbDesc.byteWidth = sizeof(StandardVertex) * (uint32_t)m_Vertices.size();
		vbDesc.stride = sizeof(StandardVertex);
		vbDesc.offset = 0;
		vbDesc.pInitData = &(m_Vertices[0]);
		vbDesc.isDynamic = false;
		m_pRenderData->pVB = (IVertexBuffer*)g_pIBufferManager->GetVertexBuffer(vbDesc);

		BufferDesc ibDesc = {};
		ibDesc.name = m_Name + "_Static" + "_IndexBuffer";
		ibDesc.byteWidth = sizeof(uint32_t) * (uint32_t)m_Indices.size();
		ibDesc.pInitData = &(m_Indices[0]);
		ibDesc.isDynamic = false;
		m_pRenderData->pIB = (IIndexBuffer*)g_pIBufferManager->GetIndexBuffer(ibDesc);

		ShaderCompileDesc vsDesc(g_StaticModelVertexShaderFilePath, nullptr, "Main", "vs_5_0", 0);
		m_pRenderData->pVS = g_pIShaderManager->GetModelVertexShader(VERTEX_TYPE::STANDARD, vsDesc);

		vsDesc = ShaderCompileDesc(g_ShadowDepthStaticVertexShaderFilePath, nullptr, "Main", "vs_5_0", 0);
		m_pRenderData->pShadowVS = g_pIShaderManager->GetModelVertexShader(VERTEX_TYPE::STANDARD, vsDesc);

		BufferDesc cbDesc = {};
		cbDesc.name = m_Name + "_ComVar_ConstantBuffer_PerObject";
		cbDesc.byteWidth = sizeof(Matrix4x4);
		cbDesc.isDynamic = false;
		m_pRenderData->pLocalToWorldCB = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);
	}

	void StaticMesh::ReleaseRenderData()
	{
		if (m_pRenderData)
		{
			m_pRenderData->pVB->Release();
			m_pRenderData->pIB->Release();
			m_pRenderData->pVS->Release();
			m_pRenderData->pShadowVS->Release();
			m_pRenderData->pLocalToWorldCB->Release();

			delete m_pRenderData;
			m_pRenderData = nullptr;
		}

		if (m_pMtlIns)
		{
			m_pMtlIns->Release();
			m_pMtlIns = nullptr;
		}
	}

	void StaticMesh::SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale)
	{
		Matrix4x4 transMatrix = GetTranslationMatrix(pos.x, pos.y, pos.z);
		Matrix4x4 rotMatrix = GetRotationMatrix(euler.x, euler.y, euler.z);
		Matrix4x4 scaleMatrix = GetScaleMatrix(scale.x, scale.y, scale.z);

		Matrix4x4 localToWorldMatrix = scaleMatrix * rotMatrix * transMatrix;

		UpdateAABB(localToWorldMatrix);

		m_pRenderData->pLocalToWorldCB->Update(&localToWorldMatrix, sizeof(Matrix4x4));
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
			//注意，对于静态模型来说，顶点位置加载进来之后即转换到左手系了，因此这里使用左手定则计算面法线
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

		if (rawVertices.size() < 3)
		{
			return;
		}

		struct VertexValue
		{
			StandardVertex*		pVert;
			uint32_t			index;
		};

		std::unordered_multimap<VertexKey, VertexValue> vertMap;
		//加入第一个三角形
		for (uint32_t i = 0; i < 3; ++i)
		{
			m_Vertices.push_back(rawVertices[i]);
			m_Indices.push_back(i);

			vertMap.insert({ {m_Vertices[i].position, m_Vertices[i].texcoord, smoothGroup[i]}, {&m_Vertices[i], i} });
		}

		for (uint32_t i = 3; i < rawVertices.size(); ++i)
		{
			StandardVertex& rawVert = rawVertices[i];
			uint32_t rawSmooth = smoothGroup[i];

			Vector3 weightedAverNormal = rawVert.normal;

			size_t sameVertsNum = vertMap.count({ rawVert.position, rawVert.texcoord, rawSmooth });
			auto findedVert = vertMap.find({ rawVert.position, rawVert.texcoord, rawSmooth });
			if (sameVertsNum == 1)
			{
				//HashMap中只找到一个相同点，则有可能手相性不同，若相同则合并，否则加入到新的顶点数组以及HashMap中
				StandardVertex* pTheSameVert = findedVert->second.pVert;
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

					StandardVertex* pNewVert = &m_Vertices[newIndex];
					vertMap.insert({ { pNewVert->position, pNewVert->texcoord, rawSmooth }, { pNewVert, newIndex} });
				}
			}
			else if (sameVertsNum > 1)
			{
				//相同点的数量大于1，则其中一定有一个手相性相同的可以合并
				for (size_t n = 0; n < sameVertsNum; ++n)
				{
					StandardVertex* pTheSameVert = findedVert->second.pVert;
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

				StandardVertex* pNewVert = &m_Vertices[newIndex];
				vertMap.insert({ { pNewVert->position, pNewVert->texcoord, rawSmooth }, { pNewVert, newIndex} });
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
			InitRenderData();
		}

		if (mesh.m_pMtlIns)
		{
			IMaterial* pMtl = mesh.m_pMtlIns->GetMaterial();
			m_pMtlIns = g_pMaterialManager->GetMaterialInstance(pMtl);
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