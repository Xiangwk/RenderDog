///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Buffer.cpp
//Written by Xiang Weikang
///////////////////////////////////

#include "Buffer.h"
#include "Vertex.h"

namespace RenderDog
{
	VertexBuffer::VertexBuffer(const VertexBufferDesc& vbDesc)
	{
		m_nVertsNum = vbDesc.nVertexNum;

		m_pData = new Vertex[m_nVertsNum];
		for (uint32_t i = 0; i < m_nVertsNum; ++i)
		{
			m_pData[i] = vbDesc.pInitData[i];
		}
	}
	VertexBuffer::~VertexBuffer()
	{
		Release();
	}

	void VertexBuffer::Release()
	{
		if (m_pData)
		{
			delete[] m_pData;
			m_pData = nullptr;
		}
	}

	IndexBuffer::IndexBuffer(const IndexBufferDesc& ibDesc)
	{
		m_pData = new uint32_t[ibDesc.nIndexNum];
		for (uint32_t i = 0; i < ibDesc.nIndexNum; ++i)
		{
			m_pData[i] = ibDesc.pInitData[i];
		}
	}
	IndexBuffer::~IndexBuffer()
	{
		Release();
	}

	void IndexBuffer::Release()
	{
		if (m_pData)
		{
			delete[] m_pData;
			m_pData = nullptr;
		}
	}
}