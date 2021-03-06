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
	struct SimpleMeshRenderData
	{
		IVertexBuffer* pVB;
		IIndexBuffer* pIB;
		IConstantBuffer* pCB;

		IShader* pVS;
		IShader* pPS;

		SimpleMeshRenderData() :
			pVB(nullptr),
			pIB(nullptr),
			pCB(nullptr),
			pVS(nullptr),
			pPS(nullptr)
		{}
	};

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

	SimpleMesh::SimpleMesh(const SimpleMesh& mesh) :
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
}// namespace RenderDog