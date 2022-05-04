////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SRBuffer.cpp
//Written by Xiang Weikang
//Desc: Software Renderer Buffer
////////////////////////////////////////

#include "Buffer.h"
#include "SoftwareRenderer.h"
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
		bufferDesc.byteWidth	= desc.byteWidth;
		bufferDesc.bindFlag		= SR_BIND_FLAG::BIND_VERTEX_BUFFER;

		SRSubResourceData initData = {};
		initData.pSysMem		= desc.pInitData;
		initData.sysMemPitch	= bufferDesc.byteWidth;

		g_pSRDevice->CreateBuffer(&bufferDesc, &initData, &m_pVB);
	}

	SRVertexBuffer::~SRVertexBuffer()
	{
		if (m_pVB)
		{
			m_pVB->Release();
			m_pVB = nullptr;
		}
	}

	void SRVertexBuffer::Release()
	{
		g_pIBufferManager->ReleaseBuffer(this);
	}

	void SRVertexBuffer::Update(void* srcData, uint32_t srcSize)
	{

	}

	void* SRVertexBuffer::GetVertexBuffer()
	{
		return (void*)m_pVB;
	}

	//================================================================
	//       IndexBuffer
	//================================================================

	class SRIndexBuffer : public IIndexBuffer
	{
	public:
		SRIndexBuffer(const BufferDesc& desc);
		virtual ~SRIndexBuffer();

		virtual void		Release() override;

		virtual void		Update(void* srcData, uint32_t srcSize) override;

		virtual void*		GetIndexBuffer() override;

		virtual uint32_t	GetIndexNum() const override { return m_indexNum; }

	private:
		ISRBuffer*			m_pIB;
		uint32_t			m_indexNum;
	};

	SRIndexBuffer::SRIndexBuffer(const BufferDesc& desc) :
		m_pIB(nullptr),
		m_indexNum(desc.byteWidth / sizeof(uint32_t))
	{
		SRBufferDesc bufferDesc = {};
		bufferDesc.byteWidth	= desc.byteWidth;
		bufferDesc.bindFlag		= SR_BIND_FLAG::BIND_INDEX_BUFFER;

		SRSubResourceData initData = {};
		initData.pSysMem		= desc.pInitData;
		initData.sysMemPitch	= desc.byteWidth;
		g_pSRDevice->CreateBuffer(&bufferDesc, &initData, &m_pIB);
	}

	SRIndexBuffer::~SRIndexBuffer()
	{
		if (m_pIB)
		{
			m_pIB->Release();
			m_pIB = nullptr;
		}
	}

	void SRIndexBuffer::Release()
	{
		g_pIBufferManager->ReleaseBuffer(this);
	}

	void SRIndexBuffer::Update(void* srcData, uint32_t srcSize)
	{

	}

	void* SRIndexBuffer::GetIndexBuffer()
	{
		return (void*)m_pIB;
	}

	//================================================================
	//       ConstantBuffer
	//================================================================

	class SRConstantBuffer : public IConstantBuffer
	{
	public:
		SRConstantBuffer(const BufferDesc& desc);
		virtual ~SRConstantBuffer();

		virtual void		Release() override;

		virtual void		Update(void* srcData, uint32_t srcSize) override;

		virtual void*		GetConstantBuffer() override;

	private:
		ISRBuffer*			m_pCB;
		bool				m_IsDynamic;
	};

	SRConstantBuffer::SRConstantBuffer(const BufferDesc& desc) :
		m_pCB(nullptr),
		m_IsDynamic(desc.isDynamic)
	{
		SRBufferDesc bufferDesc = {};
		bufferDesc.byteWidth	= desc.byteWidth;
		bufferDesc.bindFlag		= SR_BIND_FLAG::BIND_CONSTANT_BUFFER;

		SRSubResourceData initData = {};
		initData.pSysMem		= desc.pInitData;

		g_pSRDevice->CreateBuffer(&bufferDesc, nullptr, &m_pCB);
	}

	SRConstantBuffer::~SRConstantBuffer()
	{
		if (m_pCB)
		{
			m_pCB->Release();
			m_pCB = nullptr;
		}
	}

	void SRConstantBuffer::Release()
	{
		g_pIBufferManager->ReleaseBuffer(this);
	}

	void SRConstantBuffer::Update(void* srcData, uint32_t srcSize)
	{
		g_pSRImmediateContext->UpdateSubresource(m_pCB, srcData, 0, 0);
	}

	void* SRConstantBuffer::GetConstantBuffer()
	{
		return (void*)m_pCB;
	}

	//================================================================
	//       BufferManager
	//================================================================

	class SRBufferManager : public IBufferManager
	{
	public:
		SRBufferManager() = default;
		virtual ~SRBufferManager() = default;

		virtual IBuffer*	CreateBuffer(const BufferDesc& desc) override;
		virtual void		ReleaseBuffer(IBuffer* pBuffer) override;
	};

	SRBufferManager g_SRBufferManager;
	IBufferManager* g_pIBufferManager = &g_SRBufferManager;


	IBuffer* SRBufferManager::CreateBuffer(const BufferDesc& desc)
	{
		IBuffer* pBuffer = nullptr;
		switch (desc.bufferBind)
		{
		case BufferBind::NONE:
			break;

		case BufferBind::VERTEX:
			pBuffer = new SRVertexBuffer(desc);
			pBuffer->AddRef();
			break;

		case BufferBind::INDEX:
			pBuffer = new SRIndexBuffer(desc);
			pBuffer->AddRef();
			break;

		case BufferBind::CONSTANT:
			pBuffer = new SRConstantBuffer(desc);
			pBuffer->AddRef();
			break;

		default:
			break;
		}

		return pBuffer;
	}

	void SRBufferManager::ReleaseBuffer(IBuffer* pBuffer)
	{
		pBuffer->SubRef();
	}

}// namespace RenderDog