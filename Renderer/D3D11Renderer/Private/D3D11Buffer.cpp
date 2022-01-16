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

		~D3D11VertexBuffer();

		virtual bool		Init(const BufferDesc& desc, uint32_t stride, uint32_t offset) override;
		virtual void		Release() override;

		virtual void		Update() override;

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

	void D3D11VertexBuffer::Update()
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
		~D3D11IndexBuffer();

		virtual bool		Init(const BufferDesc& desc, uint32_t indexNum) override;
		virtual void		Release() override;

		virtual void		Update() override;

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

	void D3D11IndexBuffer::Update()
	{
	}

	void* D3D11IndexBuffer::GetIndexBuffer()
	{
		return (void*)m_pIB;
	}

	class D3D11BufferManager : public IBufferManager
	{
	public:
		D3D11BufferManager() = default;
		virtual ~D3D11BufferManager() = default;

		virtual IVertexBuffer*	CreateVertexBuffer() override;
		virtual IIndexBuffer*	CreateIndexBuffer() override;
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


}// namespace RenderDog