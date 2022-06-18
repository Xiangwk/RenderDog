////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: D3D11Buffer.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Buffer.h"
#include "RefCntObject.h"
#include "D3D11Renderer.h"

#include <d3d11.h>
#include <unordered_map>

namespace RenderDog
{
	//================================================================
	//       VertexBuffer
	//================================================================
	class D3D11VertexBuffer : public IVertexBuffer, public RefCntObject
	{
		friend class D3D11BufferManager;

	public:
		D3D11VertexBuffer(const BufferDesc& desc);

		virtual ~D3D11VertexBuffer();

		virtual void				Release() override;

		virtual void				Update(void* srcData, uint32_t srcSize) override;

		virtual const std::string&	GetName() const { return m_Name; }
		virtual void*				GetResource() override { return (void*)m_pVB; }

		virtual uint32_t			GetStride() const override { return m_Stride; }
		virtual uint32_t			GetOffset() const override { return m_Offset; }

	private:
		std::string					m_Name;

		ID3D11Buffer*				m_pVB;
		uint32_t					m_Stride;
		uint32_t					m_Offset;
	};

	//================================================================
	//       IndexBuffer
	//================================================================
	class D3D11IndexBuffer : public IIndexBuffer, public RefCntObject
	{
	public:
		D3D11IndexBuffer(const BufferDesc& desc);
		virtual ~D3D11IndexBuffer();

		virtual void				Release() override;

		virtual void				Update(void* srcData, uint32_t srcSize) override;

		virtual const std::string&	GetName() const { return m_Name; }
		virtual void*				GetResource() override { return (void*)m_pIB; }

		virtual uint32_t			GetIndexNum() const override { return m_indexNum; }

	private:
		std::string					m_Name;

		ID3D11Buffer*				m_pIB;
		uint32_t					m_indexNum;
	};

	//================================================================
	//       ConstantBuffer
	//================================================================
	class D3D11ConstantBuffer : public IConstantBuffer, public RefCntObject
	{
	public:
		D3D11ConstantBuffer(const BufferDesc& desc);
		virtual ~D3D11ConstantBuffer();

		virtual void				Release() override;

		virtual void				Update(void* srcData, uint32_t srcSize) override;

		virtual const std::string&	GetName() const { return m_Name; }
		virtual void*				GetResource() override { return (void*)m_pCB; }

	private:
		std::string					m_Name;

		ID3D11Buffer*				m_pCB;
		bool						m_IsDynamic;
	};

	//================================================================
	//       BufferManager
	//================================================================
	class D3D11BufferManager : public IBufferManager
	{
	private:
		typedef std::unordered_map<std::string, IBuffer*> BufferMap;

	public:
		D3D11BufferManager() = default;
		virtual ~D3D11BufferManager() = default;

		virtual IVertexBuffer*		GetVertexBuffer(const BufferDesc& desc) override;
		virtual IIndexBuffer*		GetIndexBuffer(const BufferDesc& desc) override;
		virtual IConstantBuffer*	GetConstantBuffer(const BufferDesc& desc) override;

		void						ReleaseVertexBuffer(D3D11VertexBuffer* pBuffer);
		void						ReleaseIndexBuffer(D3D11IndexBuffer* pBuffer);
		void						ReleaseConstantBuffer(D3D11ConstantBuffer* pBuffer);

	private:
		BufferMap					m_BufferMap;
	};

	D3D11BufferManager	g_D3D11BufferManager;
	IBufferManager*		g_pIBufferManager = &g_D3D11BufferManager;
	

	//================================================================
	//       Function Implementation
	//================================================================
	D3D11VertexBuffer::D3D11VertexBuffer(const BufferDesc& desc) :
		RefCntObject(),
		m_Name(desc.name),
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
		g_D3D11BufferManager.ReleaseVertexBuffer(this);
	}

	void D3D11VertexBuffer::Update(void* srcData, uint32_t srcSize)
	{

	}

	D3D11IndexBuffer::D3D11IndexBuffer(const BufferDesc& desc) :
		RefCntObject(),
		m_Name(desc.name),
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
		g_D3D11BufferManager.ReleaseIndexBuffer(this);
	}

	void D3D11IndexBuffer::Update(void* srcData, uint32_t srcSize)
	{

	}
	
	D3D11ConstantBuffer::D3D11ConstantBuffer(const BufferDesc& desc) :
		RefCntObject(),
		m_Name(desc.name),
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
		g_D3D11BufferManager.ReleaseConstantBuffer(this);
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


	IVertexBuffer* D3D11BufferManager::GetVertexBuffer(const BufferDesc& desc)
	{
		D3D11VertexBuffer* pBuffer = nullptr;

		auto buffer = m_BufferMap.find(desc.name);
		if (buffer != m_BufferMap.end())
		{
			pBuffer = (D3D11VertexBuffer*)(buffer->second);
			pBuffer->AddRef();
		}
		else
		{
			pBuffer = new D3D11VertexBuffer(desc);
		}

		return pBuffer;
	}

	IIndexBuffer* D3D11BufferManager::GetIndexBuffer(const BufferDesc& desc)
	{
		D3D11IndexBuffer* pBuffer = nullptr;

		auto buffer = m_BufferMap.find(desc.name);
		if (buffer != m_BufferMap.end())
		{
			pBuffer = (D3D11IndexBuffer*)(buffer->second);
			pBuffer->AddRef();
		}
		else
		{
			pBuffer = new D3D11IndexBuffer(desc);
		}

		return pBuffer;
	}

	IConstantBuffer* D3D11BufferManager::GetConstantBuffer(const BufferDesc& desc)
	{
		D3D11ConstantBuffer* pBuffer = nullptr;

		auto buffer = m_BufferMap.find(desc.name);
		if (buffer != m_BufferMap.end())
		{
			pBuffer = (D3D11ConstantBuffer*)(buffer->second);
			pBuffer->AddRef();
		}
		else
		{
			pBuffer = new D3D11ConstantBuffer(desc);
		}

		return pBuffer;
	}

	void D3D11BufferManager::ReleaseVertexBuffer(D3D11VertexBuffer* pBuffer)
	{
		if (pBuffer)
		{
			std::string name = pBuffer->GetName();
			if (pBuffer->SubRef() == 0)
			{
				m_BufferMap.erase(name);
			}
		}
	}

	void D3D11BufferManager::ReleaseIndexBuffer(D3D11IndexBuffer* pBuffer)
	{
		if (pBuffer)
		{
			std::string name = pBuffer->GetName();
			if (pBuffer->SubRef() == 0)
			{
				m_BufferMap.erase(name);
			}
		}
	}

	void D3D11BufferManager::ReleaseConstantBuffer(D3D11ConstantBuffer* pBuffer)
	{
		if (pBuffer)
		{
			std::string name = pBuffer->GetName();
			if (pBuffer->SubRef() == 0)
			{
				m_BufferMap.erase(name);
			}
		}
	}

}// namespace RenderDog