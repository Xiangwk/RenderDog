////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SRBuffer.cpp
//Written by Xiang Weikang
//Desc: Software Renderer Buffer
////////////////////////////////////////

#include "Buffer.h"
#include "SoftwareRender3D.h"

namespace RenderDog
{
	//================================================================
	//       VertexBuffer
	//================================================================

	class SRVertexBuffer : public IVertexBuffer
	{
	public:
		SRVertexBuffer(const BufferDesc& desc);

		virtual ~SRVertexBuffer();

		virtual void		Release() override;

		virtual void		Update(void* srcData, uint32_t srcSize) override;

		virtual void*		GetVertexBuffer() override;

		virtual uint32_t	GetStride() const override { return m_Stride; }
		virtual uint32_t	GetOffset() const override { return m_Offset; }

	private:
		ISRBuffer*			m_pVB;
		uint32_t			m_Stride;
		uint32_t			m_Offset;
	};

	SRVertexBuffer::SRVertexBuffer(const BufferDesc& desc) :
		m_pVB(nullptr),
		m_Stride(desc.stride),
		m_Offset(desc.offset)
	{
		SRBufferDesc bufferDesc = {};
		bufferDesc.byteWidth = desc.byteWidth;
		bufferDesc.bindFlag = SR_BIND_FLAG::BIND_VERTEX_BUFFER;
		SubResourceData initVBData;
		initVBData.pSysMem = desc.pInitData;
		initVBData.sysMemPitch = bufferDesc.byteWidth;
		//g_pD3D11Device->CreateBuffer(&bufferDesc, &initVBData, &m_pVB);
	}


}// namespace RenderDog