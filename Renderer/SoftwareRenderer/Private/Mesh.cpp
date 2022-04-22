///////////////////////////////////
//RenderDog <·,·>
//FileName: Mesh.cpp
//Written by Xiang Weikang
///////////////////////////////////

#include "Mesh.h"
#include "SoftwareRender3D.h"
#include "Utility.h"

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
			m_pVB->Release();
			m_pVB = nullptr;
		}

		if (m_pIB)
		{
			m_pIB->Release();
			m_pIB = nullptr;
		}
	}

	StaticMesh::StaticMesh(const std::vector<LocalVertex>& vertices, const std::vector<uint32_t>& indices):
		m_pVB(nullptr),
		m_pIB(nullptr)
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

	bool StaticMesh::Init(IDevice* pDevice)
	{
		SRBufferDesc vbDesc;
		vbDesc.byteWidth = (uint32_t)m_Vertices.size() * sizeof(LocalVertex);
		vbDesc.bindFlag = SR_BIND_FLAG::BIND_VERTEX_BUFFER;
		SubResourceData initVBData;
		initVBData.pSysMem = &m_Vertices[0];
		initVBData.sysMemPitch = vbDesc.byteWidth;
		if (!pDevice->CreateBuffer(&vbDesc, &initVBData, &m_pVB))
		{
			return false;
		}

		SRBufferDesc ibDesc;
		ibDesc.byteWidth = (uint32_t)m_Indices.size() * sizeof(uint32_t);
		ibDesc.bindFlag = SR_BIND_FLAG::BIND_INDEX_BUFFER;
		SubResourceData initIBData;
		initIBData.pSysMem = &m_Indices[0];
		initIBData.sysMemPitch = ibDesc.byteWidth;
		if (!pDevice->CreateBuffer(&ibDesc, &initIBData, &m_pIB))
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
			m_pVB = nullptr;
		}

		if (m_pIB)
		{
			m_pIB->Release();
			m_pIB = nullptr;
		}
	}

	void StaticMesh::Draw(IDeviceContext* pDeviceContext)
	{
		pDeviceContext->IASetVertexBuffer(m_pVB);
		pDeviceContext->IASetIndexBuffer(m_pIB);

		pDeviceContext->DrawIndex((uint32_t)m_Indices.size());
	}

	void StaticMesh::CalculateTangents()
	{
		//1. 按索引拆分顶点
		std::vector<LocalVertex> tempVertices;
		tempVertices.reserve(m_RawIndices.size());

		for (uint32_t i = 0; i < m_RawIndices.size(); ++i)
		{
			tempVertices.push_back(m_RawVertices[m_RawIndices[i]]);
		}

		//2. 按三角形计算TBN
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
			LocalVertex& rawVert = tempVertices[i];
			Vector3 weightedAverNormal = rawVert.normal;

			bool bHasSameVertex = false;
			uint32_t theSameIndex = 0;
			for (uint32_t j = 0; j < (uint32_t)m_Vertices.size(); ++j)
			{
				LocalVertex& vert = m_Vertices[j];
				//TEMP!!! 暂时没有光滑组信息，临时增加一个夹角大于45度则不做法线的加权平均的条件
				if (rawVert.position == vert.position && 
					rawVert.texcoord == vert.texcoord && 
					abs(DotProduct(Normalize(rawVert.normal), Normalize(vert.normal))) > 0.7f )
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
			LocalVertex& vert = m_Vertices[i];
			vert.normal.Normalize();
			Vector3 tangent = Vector3(vert.tangent.x, vert.tangent.y, vert.tangent.z);
			tangent = Normalize(tangent - vert.normal * DotProduct(vert.normal, tangent));
			vert.tangent = Vector4(tangent, vert.tangent.w);
		}
	}
}// namespace RenderDog