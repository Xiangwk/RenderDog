////////////////////////////////////////
// RenderDog <��,��>
// FileName: StaticMesh.cpp
// Written by Xiang Weikang
////////////////////////////////////////

#include "StaticMesh.h"
#include "Matrix.h"
#include "Transform.h"
#include "Utility.h"
#include "Material.h"

#include <unordered_map>
#include <fstream>
#include <locale>
#include <codecvt>

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

	bool StaticMesh::CreateMaterialInstance(IMaterial* pMtl, const std::vector<MaterialParam>* pMtlParams)
	{
		m_pMtlIns = g_pMaterialManager->GetMaterialInstance(pMtl, pMtlParams);

		return true;
	}

	bool StaticMesh::LoadMaterialInstance(const std::string& mtlinsFile)
	{
		std::ifstream fin(mtlinsFile);
		if (fin.is_open())
		{
			std::string line;
			std::getline(fin, line);

			std::string mtlName;
			size_t strStart = line.find("=") + 1;
			size_t strEnd = line.size();
			mtlName = line.substr(strStart, strEnd - strStart);
			mtlName.erase(std::remove(mtlName.begin(), mtlName.end(), ' '), mtlName.end());

			std::string mtlDirName = "UserAsset/Materials/";
			IMaterial* pMtl = g_pMaterialManager->GetMaterial(mtlDirName + mtlName);

			std::vector<MaterialParam> mtlParams;
			while (std::getline(fin, line))
			{
				if (line.find(MTL_PROPS_FLOAT4) != std::string::npos)
				{
					strStart = line.find(MTL_PROPS_FLOAT4) + MTL_PROPS_FLOAT4.size();
					strEnd = line.find("=");
					std::string mtlParamName = line.substr(strStart, strEnd - strStart);
					mtlParamName.erase(std::remove(mtlParamName.begin(), mtlParamName.end(), ' '), mtlParamName.end());

					strStart = line.find("(") + 1;
					strEnd = line.rfind(")");
					std::string mtlParamValue = line.substr(strStart, strEnd - strStart);
					mtlParamValue.erase(std::remove(mtlParamValue.begin(), mtlParamValue.end(), ' '), mtlParamValue.end());

					Vector4 vec4Value;
					strStart = 0;
					strEnd = 0;
					float tempVec4[4] = {};
					int i = 0;
					while (strEnd < mtlParamValue.size())
					{
						while (mtlParamValue[strEnd] != ',' && strEnd < mtlParamValue.size())
						{
							strEnd++;
						}
						std::string value = mtlParamValue.substr(strStart, strEnd - strStart);
						tempVec4[i++] = std::stof(value);

						strStart = strEnd + 1;
						strEnd++;
					}

					vec4Value.x = tempVec4[0];
					vec4Value.y = tempVec4[1];
					vec4Value.z = tempVec4[2];
					vec4Value.w = tempVec4[3];

					MaterialParam vec4Param(mtlParamName, MATERIAL_PARAM_TYPE::VECTOR4);
					vec4Param.SetVector4(vec4Value);

					mtlParams.push_back(vec4Param);
				}
				else if (line.find(MTL_PARAMS_TEXTURE2D) != std::string::npos)
				{
					strStart = line.find(MTL_PARAMS_TEXTURE2D) + MTL_PARAMS_TEXTURE2D.size();
					strEnd = line.find("=");
					std::string mtlParamName = line.substr(strStart, strEnd - strStart);
					mtlParamName.erase(std::remove(mtlParamName.begin(), mtlParamName.end(), ' '), mtlParamName.end());

					strStart = line.find("\"") + 1;
					strEnd = line.rfind("\"");
					std::string textureFileName = line.substr(strStart, strEnd - strStart);
					textureFileName.erase(std::remove(textureFileName.begin(), textureFileName.end(), ' '), textureFileName.end());

					std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
					RenderDog::ITexture2D* pTexture = RenderDog::g_pITextureManager->CreateTexture2D(converter.from_bytes(textureFileName));
					if (!pTexture)
					{
						return false;
					}
					RenderDog::MaterialParam textureParam(mtlParamName, RenderDog::MATERIAL_PARAM_TYPE::TEXTURE2D);
					textureParam.SetTexture2D(pTexture);
					mtlParams.push_back(textureParam);

					RenderDog::SamplerDesc samplerDesc = {};
					samplerDesc.name = mtlParamName + "Sampler";
					samplerDesc.filterMode = RenderDog::SAMPLER_FILTER::LINEAR;
					samplerDesc.addressMode = RenderDog::SAMPLER_ADDRESS::WRAP;
					RenderDog::ISamplerState* pTextureSampler = RenderDog::g_pISamplerStateManager->CreateSamplerState(samplerDesc);
					if (!pTextureSampler)
					{
						return false;
					}
					RenderDog::MaterialParam textureSamplerParam(samplerDesc.name, RenderDog::MATERIAL_PARAM_TYPE::SAMPLER);
					textureSamplerParam.SetSamplerState(pTextureSampler);
					mtlParams.push_back(textureSamplerParam);
				}
			}

			CreateMaterialInstance(pMtl, &mtlParams);
			pMtl->Release();
		}
		else
		{
			return false;
		}

		fin.close();

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
		//1. �������μ���TBN
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

			//���¼��㷨��
			//ע�⣬���ھ�̬ģ����˵������λ�ü��ؽ���֮��ת��������ϵ�ˣ��������ʹ�����ֶ�������淨��
			Vector3 faceNormal0 = CrossProduct(pos1 - pos0, pos2 - pos0);
			Vector3 faceNormal1 = CrossProduct(pos2 - pos1, pos0 - pos1);
			Vector3 faceNormal2 = CrossProduct(pos0 - pos2, pos1 - pos2);

			float rawHandParty = DotProduct(CrossProduct(faceTangent, faceBiTangent), faceNormal0) > 0.0f ? 1.0f : -1.0f;

			v0.tangent = v1.tangent = v2.tangent = Vector4(faceTangent, rawHandParty);

			v0.normal = faceNormal0;
			v1.normal = faceNormal1;
			v2.normal = faceNormal2;
		}

		//2. �ϲ���ͬ�Ķ���
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
		//�����һ��������
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
				//HashMap��ֻ�ҵ�һ����ͬ�㣬���п��������Բ�ͬ������ͬ��ϲ���������뵽�µĶ��������Լ�HashMap��
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

					//�����Բ�ͬʱ�������㷨�ߵļ�Ȩƽ��
					rawVert.normal = weightedAverNormal;
					m_Vertices.push_back(rawVert);

					StandardVertex* pNewVert = &m_Vertices[newIndex];
					vertMap.insert({ { pNewVert->position, pNewVert->texcoord, rawSmooth }, { pNewVert, newIndex} });
				}
			}
			else if (sameVertsNum > 1)
			{
				//��ͬ�����������1��������һ����һ����������ͬ�Ŀ��Ժϲ�
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
				//û���ҵ���ͬ�㣬ֱ�Ӽ��뵽�µĶ��������Լ�HashMap��
				uint32_t newIndex = (uint32_t)m_Vertices.size();
				m_Indices.push_back(newIndex);

				m_Vertices.push_back(rawVert);

				StandardVertex* pNewVert = &m_Vertices[newIndex];
				vertMap.insert({ { pNewVert->position, pNewVert->texcoord, rawSmooth }, { pNewVert, newIndex} });
			}
		}

		//3. �����ж����T��N�淶������
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
			std::vector<MaterialParam> mtlParams;
			for (uint32_t i = 0; i < m_pMtlIns->GetMaterialParamNum(); ++i)
			{
				mtlParams.push_back(m_pMtlIns->GetMaterialParamByIndex(i));
			}

			IMaterial* pMtl = mesh.m_pMtlIns->GetMaterial();
			m_pMtlIns = g_pMaterialManager->GetMaterialInstance(pMtl);

			for (uint32_t i = 0; i < m_pMtlIns->GetMaterialParamNum(); ++i)
			{
				MaterialParam& param = m_pMtlIns->GetMaterialParamByIndex(i);
				param = mtlParams[i];
			}
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