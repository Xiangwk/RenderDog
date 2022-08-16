////////////////////////////////////////////////
// RenderDog <·,·>
// FileName: SimpleMesh.cpp
// Written by Xiang Weikang
// Desc: 用于绘制标识物体，如地面网格，坐标系箭头等
////////////////////////////////////////////////

#include "SimpleMesh.h"
#include "Transform.h"

namespace RenderDog
{
	SimpleMesh::SimpleMesh() :
		m_Name(""),
		m_Vertices(0),
		m_Indices(0),
		m_pRenderData(nullptr),
		m_AABB()
	{}

	SimpleMesh::SimpleMesh(const SimpleMesh& mesh) :
		m_Name(mesh.m_Name),
		m_Vertices(mesh.m_Vertices),
		m_Indices(mesh.m_Indices),
		m_pRenderData(nullptr),
		m_AABB(mesh.m_AABB)
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
		
		ReleaseRenderData();

		CloneRenderData(mesh);

		return *this;
	}

	void SimpleMesh::Render(IPrimitiveRenderer* pPrimitiveRenderer)
	{
		PrimitiveRenderParam renderParam = {};
		renderParam.pVB						= m_pRenderData->pVB;
		renderParam.pIB						= m_pRenderData->pIB;
		renderParam.pDiffuseTexture			= nullptr;
		renderParam.pDiffuseTextureSampler	= nullptr;
		renderParam.pVS						= m_pRenderData->pVS;
		renderParam.pPerObjectCB			= m_pRenderData->pPerObjectCB;

		pPrimitiveRenderer->Render(renderParam);
	}

	void SimpleMesh::LoadFromSimpleData(const std::vector<SimpleVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name)
	{
		m_Vertices.assign(vertices.begin(), vertices.end());

		m_Indices.assign(indices.begin(), indices.end());

		m_Name = name;
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
		m_pRenderData->pPerObjectCB = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);
	}

	void SimpleMesh::ReleaseRenderData()
	{
		if (m_pRenderData)
		{
			m_pRenderData->pVB->Release();
			m_pRenderData->pIB->Release();
			m_pRenderData->pVS->Release();
			m_pRenderData->pPerObjectCB->Release();

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

		m_pRenderData->pPerObjectCB->Update(&localToWorldMatrix, sizeof(Matrix4x4));
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