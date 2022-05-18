////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: D3D11Buffer.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Buffer.h"
#include "D3D11Renderer.h"

#include <d3d11.h>

namespace RenderDog
{
	//================================================================
	//       VertexBuffer
	//================================================================

	class D3D11VertexBuffer : public IVertexBuffer
	{
	public:
		D3D11VertexBuffer(const BufferDesc& desc);

		virtual ~D3D11VertexBuffer();

		virtual void		Release() override;

		virtual void		Update(void* srcData, uint32_t srcSize) override;

		virtual void*		GetVertexBuffer() override;

		virtual uint32_t	GetStride() const override { return m_Stride; }
		virtual uint32_t	GetOffset() const override { return m_Offset; }

	private:
		ID3D11Buffer*		m_pVB;
		uint32_t			m_Stride;
		uint32_t			m_Offset;
	};

	D3D11VertexBuffer::D3D11VertexBuffer(const BufferDesc& desc) :
		m_pVB(nullptr),
		m_Stride(desc.stride),
		m_Offset(desc.offset)
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.Usage			= desc.isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth		= desc.byteWidth;
		bufferDesc.BindFlags		= D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags	= desc.isDynamic ? D3D11_CPU_ACCESS_WRITE : 0;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = desc.pInitData;

		g_pD3D11Device->CreateBuffer(&bufferDesc, &initData, &m_pVB);
	}

	D3D11VertexBuffer::~D3D11VertexBuffer()
	{
		if (m_pVB)
		{
			m_pVB->Release();
			m_pVB = nullptr;
		}
	}

	void D3D11VertexBuffer::Release()
	{
		g_pIBufferManager->ReleaseBuffer(this);
	}

	void D3D11VertexBuffer::Update(void* srcData, uint32_t srcSize)
	{

	}

	void* D3D11VertexBuffer::GetVertexBuffer()
	{
		return (void*)m_pVB;
	}


	//================================================================
	//       IndexBuffer
	//================================================================

	class D3D11IndexBuffer : public IIndexBuffer
	{
	public:
		D3D11IndexBuffer(const BufferDesc& desc);
		virtual ~D3D11IndexBuffer();

		virtual void		Release() override;

		virtual void		Update(void* srcData, uint32_t srcSize) override;

		virtual void*		GetIndexBuffer() override;

		virtual uint32_t	GetIndexNum() const override { return m_indexNum; }

	private:
		ID3D11Buffer*		m_pIB;
		uint32_t			m_indexNum;
	};

	D3D11IndexBuffer::D3D11IndexBuffer(const BufferDesc& desc) :
		m_pIB(nullptr),
		m_indexNum(desc.byteWidth / sizeof(uint32_t))
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.Usage			= desc.isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth		= desc.byteWidth;
		bufferDesc.BindFlags		= D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags	= desc.isDynamic ? D3D11_CPU_ACCESS_WRITE : 0;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = desc.pInitData;

		g_pD3D11Device->CreateBuffer(&bufferDesc, &initData, &m_pIB);
	}

	D3D11IndexBuffer::~D3D11IndexBuffer()
	{
		if (m_pIB)
		{
			m_pIB->Release();
			m_pIB = nullptr;
		}
	}

	void D3D11IndexBuffer::Release()
	{
		g_pIBufferManager->ReleaseBuffer(this);
	}

	void D3D11IndexBuffer::Update(void* srcData, uint32_t srcSize)
	{

	}

	void* D3D11IndexBuffer::GetIndexBuffer()
	{
		return (void*)m_pIB;
	}


	//================================================================
	//       ConstantBuffer
	//================================================================

	class D3D11ConstantBuffer : public IConstantBuffer
	{
	public:
		D3D11ConstantBuffer(const BufferDesc& desc);
		virtual ~D3D11ConstantBuffer();

		virtual void		Release() override;

		virtual void		Update(void* srcData, uint32_t srcSize) override;

		virtual void*		GetConstantBuffer() override;

	private:
		ID3D11Buffer*		m_pCB;
		bool				m_IsDynamic;
	};

	D3D11ConstantBuffer::D3D11ConstantBuffer(const BufferDesc& desc) :
		m_pCB(nullptr),
		m_IsDynamic(desc.isDynamic)
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.Usage			= desc.isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth		= desc.byteWidth;
		bufferDesc.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags	= desc.isDynamic ? D3D11_CPU_ACCESS_WRITE : 0;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = desc.pInitData;

		g_pD3D11Device->CreateBuffer(&bufferDesc, nullptr, &m_pCB);
	}

	D3D11ConstantBuffer::~D3D11ConstantBuffer()
	{
		if (m_pCB)
		{
			m_pCB->Release();
			m_pCB = nullptr;
		}
	}

	void D3D11ConstantBuffer::Release()
	{
		g_pIBufferManager->ReleaseBuffer(this);
	}

	void D3D11ConstantBuffer::Update(void* srcData, uint32_t srcSize)
	{
		if (!m_IsDynamic)
		{
			g_pD3D11ImmediateContext->UpdateSubresource(m_pCB, 0, nullptr, srcData, 0, 0);
		}
		else
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource = {};
			g_pD3D11ImmediateContext->Map(m_pCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			memcpy(mappedResource.pData, srcData, srcSize);
			g_pD3D11ImmediateContext->Unmap(m_pCB, 0);
		}
	}

	void* D3D11ConstantBuffer::GetConstantBuffer()
	{
		return (void*)m_pCB;
	}

	//================================================================
	//       BufferManager
	//================================================================

	class D3D11BufferManager : public IBufferManager
	{
	public:
		D3D11BufferManager() = default;
		virtual ~D3D11BufferManager() = default;

		virtual IBuffer*	CreateBuffer(const BufferDesc& desc) override;
		virtual void		ReleaseBuffer(IBuffer* pBuffer) override;
	};

	D3D11BufferManager	g_D3D11BufferManager;
	IBufferManager*		g_pIBufferManager = &g_D3D11BufferManager;


	IBuffer* D3D11BufferManager::CreateBuffer(const BufferDesc& desc)
	{
		IBuffer* pBuffer = nullptr;
		switch (desc.bufferBind)
		{
		case BUFFER_BIND::NONE:
			break;

		case BUFFER_BIND::VERTEX:
			pBuffer = new D3D11VertexBuffer(desc);
			pBuffer->AddRef();
			break;

		case BUFFER_BIND::INDEX:
			pBuffer = new D3D11IndexBuffer(desc);
			pBuffer->AddRef();
			break;

		case BUFFER_BIND::CONSTANT:
			pBuffer = new D3D11ConstantBuffer(desc);
			pBuffer->AddRef();
			break;

		default:
			break;
		}

		return pBuffer;
	}

	void D3D11BufferManager::ReleaseBuffer(IBuffer* pBuffer)
	{
		pBuffer->SubRef();
	}

}// namespace RenderDog