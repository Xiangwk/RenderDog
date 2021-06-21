///////////////////////////////////
//RenderDog <·,·>
//FileName: Mesh.cpp
//Written by Xiang Weikang
///////////////////////////////////

#include "Mesh.h"
#include "Device.h"
#include "DeviceContext.h"
#include "Buffer.h"

#include <unordered_map>

namespace RenderDog
{
	StaticMesh::StaticMesh():
		m_pVB(nullptr),
		m_pIB(nullptr)
	{}

	StaticMesh::~StaticMesh()
	{
		if (m_pVB)
		{
			delete m_pVB;
			m_pVB = nullptr;
		}

		if (m_pIB)
		{
			delete m_pIB;
			m_pIB = nullptr;
		}
	}

	StaticMesh::StaticMesh(const std::vector<Vertex>& Vertices, const std::vector<uint32_t>& Indices):
		m_pVB(nullptr),
		m_pIB(nullptr)
	{
		m_RawVertices.reserve(Vertices.size());
		m_RawIndices.reserve(Indices.size());

		for (uint32_t i = 0; i < Vertices.size(); ++i)
		{
			m_RawVertices.push_back(Vertices[i]);
		}

		for (uint32_t i = 0; i < Indices.size(); ++i)
		{
			m_RawIndices.push_back(Indices[i]);
		}
	}

	bool StaticMesh::Init(Device* pDevice)
	{
		VertexBufferDesc vbDesc;
		vbDesc.nVertexNum = (uint32_t)m_Vertices.size();
		vbDesc.pInitData = &m_Vertices[0];
		if (!pDevice->CreateVertexBuffer(vbDesc, &m_pVB))
		{
			return false;
		}

		IndexBufferDesc ibDesc;
		ibDesc.nIndexNum = (uint32_t)m_Indices.size();
		ibDesc.pInitData = &m_Indices[0];
		if (!pDevice->CreateIndexBuffer(ibDesc, &m_pIB))
		{
			return false;
		}

		return true;
	}

	void StaticMesh::Release()
	{
		if (m_pVB)
		{
			m_pVB->Release();

			delete m_pVB;
			m_pVB = nullptr;
		}

		if (m_pIB)
		{
			m_pIB->Release();

			delete m_pIB;
			m_pIB = nullptr;
		}
	}

	void StaticMesh::Draw(DeviceContext* pDeviceContext)
	{
		pDeviceContext->IASetVertexBuffer(m_pVB);
		pDeviceContext->IASetIndexBuffer(m_pIB);

		pDeviceContext->DrawIndex((uint32_t)m_Indices.size());
	}

	void StaticMesh::DrawTangentSpace(DeviceContext* pDeviceContext)
	{

	}

	void StaticMesh::CalculateTangents()
	{
		m_Vertices.resize(m_RawVertices.size());
		m_Indices.resize(m_RawIndices.size());

		std::vector<Vector3> vPositions(m_RawVertices.size(), Vector3());
		std::vector<Vector3> vColors(m_RawVertices.size(), Vector3());
		std::vector<Vector2> vTexcoords(m_RawVertices.size(), Vector2());
		std::vector<Vector3> vTangents(m_RawVertices.size(), Vector3());
		std::vector<Vector3> vBiTangents(m_RawVertices.size(), Vector3());
		std::vector<Vector3> vNormals(m_RawVertices.size(), Vector3());
		std::vector<float> vHandPartys(m_RawVertices.size(), 0.0f);

		std::unordered_map<uint32_t, uint32_t> IndexMap; //用于拆点时做索引映射，key：旧索引，value：新索引

		for (uint32_t i = 0; i < m_RawIndices.size(); i += 3)
		{
			uint32_t nIndex0 = m_RawIndices[i];
			uint32_t nIndex1 = m_RawIndices[i + 1];
			uint32_t nIndex2 = m_RawIndices[i + 2];

			const Vertex& v0 = m_RawVertices[nIndex0];
			const Vertex& v1 = m_RawVertices[nIndex1];
			const Vertex& v2 = m_RawVertices[nIndex2];

			const Vector3& vPos0 = v0.vPosition;
			const Vector3& vPos1 = v1.vPosition;
			const Vector3& vPos2 = v2.vPosition;

			const Vector2& vTex0 = v0.vTexcoord;
			const Vector2& vTex1 = v1.vTexcoord;
			const Vector2& vTex2 = v2.vTexcoord;

			float x1 = vPos1.x - vPos0.x;
			float x2 = vPos2.x - vPos0.x;
			float y1 = vPos1.y - vPos0.y;
			float y2 = vPos2.y - vPos0.y;
			float z1 = vPos1.z - vPos0.z;
			float z2 = vPos2.z - vPos0.z;

			float s1 = vTex1.x - vTex0.x;
			float s2 = vTex2.x - vTex0.x;
			float t1 = vTex1.y - vTex0.y;
			float t2 = vTex2.y - vTex0.y;

			float fInv = 1.0f / (s1 * t2 - s2 * t1);

			float tx = fInv * (t2 * x1 - t1 * x2);
			float ty = fInv * (t2 * y1 - t1 * y2);
			float tz = fInv * (t2 * z1 - t1 * z2);

			float bx = fInv * (s1 * x2 - s2 * x1);
			float by = fInv * (s1 * y2 - s2 * y1);
			float bz = fInv * (s1 * z2 - s2 * z1);

			Vector3 vRawTangent = Vector3(tx, ty, tz);
			Vector3 vRawBiTangent = Vector3(bx, by, bz);

			const Vector3& vRawNormal0 = v0.vNormal;
			const Vector3& vRawNormal1 = v1.vNormal;
			const Vector3& vRawNormal2 = v2.vNormal;

			float fRawHandParty0 = vHandPartys[nIndex0];
			float fRawHandParty1 = vHandPartys[nIndex1];
			float fRawHandParty2 = vHandPartys[nIndex2];

			float fHandParty0 = DotProduct(CrossProduct(vRawTangent, vRawBiTangent), vRawNormal0) > 0.0f ? 1.0f : -1.0f;
			float fHandParty1 = DotProduct(CrossProduct(vRawTangent, vRawBiTangent), vRawNormal1) > 0.0f ? 1.0f : -1.0f;
			float fHandParty2 = DotProduct(CrossProduct(vRawTangent, vRawBiTangent), vRawNormal2) > 0.0f ? 1.0f : -1.0f;

			if (fRawHandParty0 != 0 && fHandParty0 != fRawHandParty0)
			{
				if (IndexMap.find(nIndex0) != IndexMap.end())
				{
					//与已经拆分过的顶点合并
					uint32_t nNewIndex = IndexMap[nIndex0];
					vNormals[nNewIndex] = vRawNormal0;
					vTangents[nNewIndex] += vRawTangent;
					vBiTangents[nNewIndex] += vRawBiTangent;

					m_Indices[i] = nNewIndex;
				}
				else
				{
					//拆分新的顶点
					vPositions.push_back(vPos0);
					vColors.push_back(v0.vColor);
					vTexcoords.push_back(vTex0);
					vNormals.push_back(vRawNormal0);
					vTangents.push_back(vRawTangent);
					vBiTangents.push_back(vRawBiTangent);
					vHandPartys.push_back(fHandParty0);

					m_Indices[i] = ((uint32_t)m_Vertices.size());
					m_Vertices.push_back(Vertex());

					IndexMap.insert({ nIndex0, m_Indices[i] });
				}
			}
			else
			{
				vPositions[nIndex0] = vPos0;
				vColors[nIndex0] = v0.vColor;
				vTexcoords[nIndex0] = vTex0;
				vNormals[nIndex0] = vRawNormal0;
				vTangents[nIndex0] += vRawTangent;
				vBiTangents[nIndex0] += vRawBiTangent;
				vHandPartys[nIndex0] = fHandParty0;

				m_Indices[i] = nIndex0;
			}

			if (fRawHandParty1 != 0 && fHandParty1 != fRawHandParty1)
			{	
				if (IndexMap.find(nIndex1) != IndexMap.end())
				{
					//与已经拆分过的顶点合并
					uint32_t nNewIndex = IndexMap[nIndex1];
					vNormals[nNewIndex] = vRawNormal1;
					vTangents[nNewIndex] += vRawTangent;
					vBiTangents[nNewIndex] += vRawBiTangent;

					m_Indices[i + 1] = nNewIndex;
				}
				else
				{
					//拆分新的顶点
					vPositions.push_back(vPos1);
					vColors.push_back(v1.vColor);
					vTexcoords.push_back(vTex1);
					vNormals.push_back(vRawNormal1);
					vTangents.push_back(vRawTangent);
					vBiTangents.push_back(vRawBiTangent);
					vHandPartys.push_back(fHandParty1);

					m_Indices[i + 1] = ((uint32_t)m_Vertices.size());
					m_Vertices.push_back(Vertex());
					

					IndexMap.insert({ nIndex1, m_Indices[i + 1] });
				}
			}
			else
			{
				vPositions[nIndex1] = vPos1;
				vColors[nIndex1] = v1.vColor;
				vTexcoords[nIndex1] = vTex1;
				vNormals[nIndex1] = vRawNormal1;
				vTangents[nIndex1] += vRawTangent;
				vBiTangents[nIndex1] += vRawBiTangent;
				vHandPartys[nIndex1] = fHandParty1;

				m_Indices[i + 1] = nIndex1;
			}

			if (fRawHandParty2 != 0 && fHandParty2 != fRawHandParty2)
			{
				if (IndexMap.find(nIndex2) != IndexMap.end())
				{
					//与已经拆分过的顶点合并
					uint32_t nNewIndex = IndexMap[nIndex2];
					vNormals[nNewIndex] = vRawNormal2;
					vTangents[nNewIndex] += vRawTangent;
					vBiTangents[nNewIndex] += vRawBiTangent;

					m_Indices[i + 2] = nNewIndex;
				}
				else
				{
					//拆分新的顶点
					vPositions.push_back(vPos2);
					vColors.push_back(v2.vColor);
					vTexcoords.push_back(vTex2);
					vNormals.push_back(vRawNormal2);
					vTangents.push_back(vRawTangent);
					vBiTangents.push_back(vRawBiTangent);
					vHandPartys.push_back(fHandParty2);

					m_Indices[i + 2] = ((uint32_t)m_Vertices.size());
					m_Vertices.push_back(Vertex());
					
					IndexMap.insert({ nIndex2, m_Indices[i + 2] });
				}
			}
			else
			{
				vPositions[nIndex2] = vPos2;
				vColors[nIndex2] = v2.vColor;
				vTexcoords[nIndex2] = vTex2;
				vNormals[nIndex2] = vRawNormal2;
				vTangents[nIndex2] += vRawTangent;
				vBiTangents[nIndex2] += vRawBiTangent;
				vHandPartys[nIndex2] = fHandParty2;

				m_Indices[i + 2] = nIndex2;
			}
		}

		for (uint32_t i = 0; i < m_Vertices.size(); ++i)
		{
			Vector3 vPos = vPositions[i];
			Vector3 vColor = vColors[i];
			Vector2 vTexcoord = vTexcoords[i];
			float fHandParty = vHandPartys[i];
			//Vector3 vTangent = Normalize(vTangents[i]);
			//Vector3 vBiTangent = Normalize(vBiTangents[i]);
			//Vector3 vNormal = CrossProduct(vTangent, vBiTangent) * fHandParty;
			Vector3 vNormal = Normalize(vNormals[i]);
			//Vector3 vTangent = Normalize((vTangents[i] - vNormal * DotProduct(vNormal, vTangents[i])));
			Vector3 vTangent = Normalize(vTangents[i]);

			m_Vertices[i] = { vPos, vColor, vNormal, Vector4(vTangent.x, vTangent.y, vTangent.z, fHandParty), vTexcoord };
		}
	}
}