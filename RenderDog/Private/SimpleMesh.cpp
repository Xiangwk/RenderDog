////////////////////////////////////////////////
// RenderDog <·,·>
// FileName: SimpleMesh.cpp
// Written by Xiang Weikang
// Desc: 用于绘制标识物体，如地面网格，坐标系箭头等
////////////////////////////////////////////////

#include "SimpleMesh.h"
#include "Transform.h"

#include <unordered_map>

namespace RenderDog
{
	struct VertexKey
	{
		Vector3		pos;

		bool operator==(const VertexKey& rhs) const
		{
			return pos == rhs.pos;
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
				^ hash<float>()(vk.pos.z);
		}
	};
}// namespace std

namespace RenderDog
{
	SimpleMesh::SimpleMesh() :
		m_Name(""),
		m_Vertices(0),
		m_Indices(0),
		m_pRenderData(nullptr),
		m_AABB(),
		m_bIsLine(false)
	{}

	SimpleMesh::SimpleMesh(const std::string& name) :
		m_Name(name),
		m_Vertices(0),
		m_Indices(0),
		m_pRenderData(nullptr),
		m_AABB(),
		m_bIsLine(false)
	{}

	SimpleMesh::SimpleMesh(const SimpleMesh& mesh) :
		m_Name(mesh.m_Name),
		m_Vertices(mesh.m_Vertices),
		m_Indices(mesh.m_Indices),
		m_pRenderData(nullptr),
		m_AABB(mesh.m_AABB),
		m_bIsLine(mesh.m_bIsLine)
	{
		CloneRenderData(mesh);
	}

	SimpleMesh::~SimpleMesh()
	{
		ReleaseRenderData();

		m_Vertices.clear();
		m_Indices.clear();
	}

	SimpleMesh& SimpleMesh::operator=(const SimpleMesh& mesh)
	{
		if (&mesh == this)
		{
			return *this;
		}

		m_Name					= mesh.m_Name;
		m_Vertices				= mesh.m_Vertices;
		m_Indices				= mesh.m_Indices;
		m_AABB					= mesh.m_AABB;
		m_bIsLine				= mesh.m_bIsLine;
		
		ReleaseRenderData();

		CloneRenderData(mesh);

		return *this;
	}

	void SimpleMesh::Render(IPrimitiveRenderer* pPrimitiveRenderer)
	{
		PrimitiveRenderParam renderParam = {};
		renderParam.pVB							= m_pRenderData->pVB;
		renderParam.pIB							= m_pRenderData->pIB;
		renderParam.pMtlIns						= nullptr;
		renderParam.pVS							= m_pRenderData->pVS;
		renderParam.PerObjParam.pPerObjectCB	= m_pRenderData->pLocalToWorldCB;
		renderParam.bRenderLine					= m_bIsLine;

		pPrimitiveRenderer->Render(renderParam);
	}

	void SimpleMesh::LoadFromSimpleData(const std::vector<SimpleVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name)
	{
		m_Vertices.assign(vertices.begin(), vertices.end());

		m_Indices.assign(indices.begin(), indices.end());

		m_Name = name;
	}

	void SimpleMesh::GenVerticesAndIndices(std::vector<SimpleVertex>& rawVertices)
	{
		//2. 合并相同的顶点
		m_Vertices.reserve(rawVertices.size());
		m_Indices.reserve(rawVertices.size());

		if (rawVertices.size() < 3)
		{
			return;
		}

		struct VertexValue
		{
			SimpleVertex*	pVert;
			uint32_t		index;
		};

		std::unordered_multimap<VertexKey, VertexValue> vertMap;
		//加入第一个三角形
		for (uint32_t i = 0; i < 3; ++i)
		{
			m_Vertices.push_back(rawVertices[i]);
			m_Indices.push_back(i);

			vertMap.insert({ {m_Vertices[i].position}, {&m_Vertices[i], i} });
		}

		for (uint32_t i = 3; i < rawVertices.size(); ++i)
		{
			SimpleVertex& rawVert = rawVertices[i];

			size_t sameVertsNum = vertMap.count({ rawVert.position });
			auto findedVert = vertMap.find({ rawVert.position });
			if (findedVert != vertMap.end())
			{
				for (size_t n = 0; n < sameVertsNum; ++n)
				{
					m_Indices.push_back(findedVert->second.index);
					findedVert++;
				}
			}
			else
			{
				//没有找到相同点，直接加入到新的顶点数组以及HashMap中
				uint32_t newIndex = (uint32_t)m_Vertices.size();
				m_Indices.push_back(newIndex);

				m_Vertices.push_back(rawVert);

				SimpleVertex* pNewVert = &m_Vertices[newIndex];
				vertMap.insert({ { pNewVert->position }, { pNewVert, newIndex} });
			}
		}
	}

	void SimpleMesh::InitRenderData()
	{
		m_pRenderData = new SimpleMeshRenderData();

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

		ShaderCompileDesc vsDesc(g_SimpleModelVertexShadreFilePath, nullptr, "Main", "vs_5_0", 0);
		m_pRenderData->pVS = g_pIShaderManager->GetModelVertexShader(VERTEX_TYPE::SIMPLE, vsDesc);

		BufferDesc cbDesc = {};
		cbDesc.name = m_Name + "_ComVar_ConstantBuffer_PerObject";
		cbDesc.byteWidth = sizeof(Matrix4x4);
		cbDesc.isDynamic = false;
		m_pRenderData->pLocalToWorldCB = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);
	}

	void SimpleMesh::ReleaseRenderData()
	{
		if (m_pRenderData)
		{
			m_pRenderData->pVB->Release();
			m_pRenderData->pIB->Release();
			m_pRenderData->pVS->Release();
			m_pRenderData->pLocalToWorldCB->Release();

			delete m_pRenderData;
			m_pRenderData = nullptr;
		}
	}

	void SimpleMesh::SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale)
	{
		Matrix4x4 transMatrix = GetTranslationMatrix(pos.x, pos.y, pos.z);
		Matrix4x4 rotMatrix = GetRotationMatrix(euler.x, euler.y, euler.z);
		Matrix4x4 scaleMatrix = GetScaleMatrix(scale.x, scale.y, scale.z);

		Matrix4x4 localToWorldMatrix = scaleMatrix * rotMatrix * transMatrix;

		m_pRenderData->pLocalToWorldCB->Update(&localToWorldMatrix, sizeof(Matrix4x4));
	}

	void SimpleMesh::CloneRenderData(const SimpleMesh& mesh)
	{
		if (mesh.m_pRenderData)
		{
			const std::string& vsFile = mesh.m_pRenderData->pVS->GetFileName();

			InitRenderData();
		}
	}
}// namespace RenderDog