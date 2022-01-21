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
		D3D11VertexBuffer();

		virtual ~D3D11VertexBuffer();

		virtual bool		Init(const BufferDesc& desc, uint32_t stride, uint32_t offset) override;
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

	D3D11VertexBuffer::D3D11VertexBuffer() :
		m_pVB(nullptr),
		m_Stride(0),
		m_Offset(0)
	{}

	D3D11VertexBuffer::~D3D11VertexBuffer()
	{}

	bool D3D11VertexBuffer::Init(const BufferDesc& desc, uint32_t stride, uint32_t offset)
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.Usage = desc.isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = desc.byteWidth;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = desc.isDynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = desc.pInitData;
		if (FAILED(g_pD3D11Device->CreateBuffer(&bufferDesc, &initData, &m_pVB)))
		{
			return false;
		}

		m_Stride = stride;
		m_Offset = offset;

		return true;
	}

	void D3D11VertexBuffer::Release()
	{
		if (m_pVB)
		{
			m_pVB->Release();
			m_pVB = nullptr;
		}
		
		m_Stride = 0;
		m_Offset = 0;
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
		D3D11IndexBuffer();
		virtual ~D3D11IndexBuffer();

		virtual bool		Init(const BufferDesc& desc, uint32_t indexNum) override;
		virtual void		Release() override;

		virtual void		Update(void* srcData, uint32_t srcSize) override;

		virtual void*		GetIndexBuffer() override;

		virtual uint32_t	GetIndexNum() const override { return m_indexNum; }

	private:
		ID3D11Buffer*		m_pIB;
		uint32_t			m_indexNum;
	};

	D3D11IndexBuffer::D3D11IndexBuffer() :
		m_pIB(nullptr),
		m_indexNum(0)
	{}

	D3D11IndexBuffer::~D3D11IndexBuffer()
	{}

	bool D3D11IndexBuffer::Init(const BufferDesc& desc, uint32_t indexNum)
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.Usage = desc.isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = desc.byteWidth;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = desc.isDynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = desc.pInitData;
		if (FAILED(g_pD3D11Device->CreateBuffer(&bufferDesc, &initData, &m_pIB)))
		{
			return false;
		}

		m_indexNum = indexNum;

		return true;
	}

	void D3D11IndexBuffer::Release()
	{
		if (m_pIB)
		{
			m_pIB->Release();
			m_pIB = nullptr;
		}

		m_indexNum = 0;
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
		D3D11ConstantBuffer();
		virtual ~D3D11ConstantBuffer();

		virtual bool		Init(const BufferDesc& desc) override;
		virtual void		Release() override;

		virtual void		Update(void* srcData, uint32_t srcSize) override;

		virtual void*		GetConstantBuffer() override;

	private:
		ID3D11Buffer*		m_pCB;
		bool				m_IsDynamic;
	};

	D3D11ConstantBuffer::D3D11ConstantBuffer() :
		m_pCB(nullptr),
		m_IsDynamic(false)
	{}

	D3D11ConstantBuffer::~D3D11ConstantBuffer()
	{}

	bool D3D11ConstantBuffer::Init(const BufferDesc& desc)
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.Usage = desc.isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = desc.byteWidth;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = desc.isDynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = desc.pInitData;
		if (FAILED(g_pD3D11Device->CreateBuffer(&bufferDesc, nullptr, &m_pCB)))
		{
			return false;
		}

		m_IsDynamic = desc.isDynamic;

		return true;
	}

	void D3D11ConstantBuffer::Release()
	{
		if (m_pCB)
		{
			m_pCB->Release();
			m_pCB = nullptr;
		}
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

		virtual IVertexBuffer*		CreateVertexBuffer() override;
		virtual IIndexBuffer*		CreateIndexBuffer() override;
		virtual IConstantBuffer*	CreateConstantBuffer() override;
	};

	D3D11BufferManager g_D3D11BufferManager;
	IBufferManager* g_pIBufferManager = &g_D3D11BufferManager;


	IVertexBuffer* D3D11BufferManager::CreateVertexBuffer()
	{
		D3D11VertexBuffer* pVertexBuffer = new D3D11VertexBuffer();

		return pVertexBuffer;
	}

	IIndexBuffer* D3D11BufferManager::CreateIndexBuffer()
	{
		D3D11IndexBuffer* pIndexBuffer = new D3D11IndexBuffer();

		return pIndexBuffer;
	}

	IConstantBuffer* D3D11BufferManager::CreateConstantBuffer()
	{
		D3D11ConstantBuffer* pConstantBuffer = new D3D11ConstantBuffer();

		return pConstantBuffer;
	}


}// namespace RenderDog