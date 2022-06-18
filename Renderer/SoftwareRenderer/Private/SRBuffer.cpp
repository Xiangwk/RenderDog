////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SRBuffer.cpp
//Written by Xiang Weikang
//Desc: Software Renderer Buffer
////////////////////////////////////////

#include "Buffer.h"
#include "RefCntObject.h"
#include "SoftwareRenderer.h"
#include "SoftwareRender3D.h"

#include <unordered_map>

namespace RenderDog
{
	//================================================================
	//       VertexBuffer
	//================================================================
	class SRVertexBuffer : public IVertexBuffer, public RefCntObject
	{
	public:
		SRVertexBuffer(const BufferDesc& desc);

		virtual ~SRVertexBuffer();

		virtual void				Release() override;

		virtual void				Update(void* srcData, uint32_t srcSize) override;

		virtual const std::string&	GetName() const { return m_Name; }
		virtual void*				GetResource() override { return (void*)m_pVB; }

		virtual uint32_t			GetStride() const override { return m_Stride; }
		virtual uint32_t			GetOffset() const override { return m_Offset; }

	private:
		std::string					m_Name;

		ISRBuffer*					m_pVB;
		uint32_t					m_Stride;
		uint32_t					m_Offset;
	};

	//================================================================
	//       IndexBuffer
	//================================================================
	class SRIndexBuffer : public IIndexBuffer, public RefCntObject
	{
	public:
		SRIndexBuffer(const BufferDesc& desc);
		virtual ~SRIndexBuffer();

		virtual void				Release() override;

		virtual void				Update(void* srcData, uint32_t srcSize) override;

		virtual const std::string&	GetName() const { return m_Name; }
		virtual void*				GetResource() override { return (void*)m_pIB; }

		virtual uint32_t			GetIndexNum() const override { return m_indexNum; }

	private:
		std::string					m_Name;

		ISRBuffer*					m_pIB;
		uint32_t					m_indexNum;
	};

	//================================================================
	//       ConstantBuffer
	//================================================================
	class SRConstantBuffer : public IConstantBuffer, public RefCntObject
	{
	public:
		SRConstantBuffer(const BufferDesc& desc);
		virtual ~SRConstantBuffer();

		virtual void				Release() override;

		virtual void				Update(void* srcData, uint32_t srcSize) override;

		virtual const std::string&	GetName() const { return m_Name; }
		virtual void*				GetResource() override { return (void*)m_pCB; }

	private:
		std::string					m_Name;

		ISRBuffer*					m_pCB;
		bool						m_IsDynamic;
	};

	//================================================================
	//       BufferManager
	//================================================================
	class SRBufferManager : public IBufferManager
	{
	private:
		typedef std::unordered_map<std::string, IBuffer*> BufferMap;

	public:
		SRBufferManager() = default;
		virtual ~SRBufferManager() = default;

		virtual IVertexBuffer*		GetVertexBuffer(const BufferDesc& desc) override;
		virtual IIndexBuffer*		GetIndexBuffer(const BufferDesc& desc) override;
		virtual IConstantBuffer*	GetConstantBuffer(const BufferDesc& desc) override;

		void						ReleaseVertexBuffer(SRVertexBuffer* pBuffer);
		void						ReleaseIndexBuffer(SRIndexBuffer* pBuffer);
		void						ReleaseConstantBuffer(SRConstantBuffer* pBuffer);

	private:
		BufferMap					m_BufferMap;
	};

	SRBufferManager g_SRBufferManager;
	IBufferManager* g_pIBufferManager = &g_SRBufferManager;


	//================================================================
	//       Function Implementation
	//================================================================
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
		g_SRBufferManager.ReleaseVertexBuffer(this);
	}

	void SRVertexBuffer::Update(void* srcData, uint32_t srcSize)
	{

	}

	

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
		g_SRBufferManager.ReleaseIndexBuffer(this);
	}

	void SRIndexBuffer::Update(void* srcData, uint32_t srcSize)
	{

	}
	

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
		g_SRBufferManager.ReleaseConstantBuffer(this);
	}

	void SRConstantBuffer::Update(void* srcData, uint32_t srcSize)
	{
		g_pSRImmediateContext->UpdateSubresource(m_pCB, srcData, 0, 0);
	}


	IVertexBuffer* SRBufferManager::GetVertexBuffer(const BufferDesc& desc)
	{
		SRVertexBuffer* pBuffer = nullptr;

		auto buffer = m_BufferMap.find(desc.name);
		if (buffer != m_BufferMap.end())
		{
			pBuffer = (SRVertexBuffer*)(buffer->second);
			pBuffer->AddRef();
		}
		else
		{
			pBuffer = new SRVertexBuffer(desc);
		}

		return pBuffer;
	}

	IIndexBuffer* SRBufferManager::GetIndexBuffer(const BufferDesc& desc)
	{
		SRIndexBuffer* pBuffer = nullptr;

		auto buffer = m_BufferMap.find(desc.name);
		if (buffer != m_BufferMap.end())
		{
			pBuffer = (SRIndexBuffer*)(buffer->second);
			pBuffer->AddRef();
		}
		else
		{
			pBuffer = new SRIndexBuffer(desc);
		}

		return pBuffer;
	}

	IConstantBuffer* SRBufferManager::GetConstantBuffer(const BufferDesc& desc)
	{
		SRConstantBuffer* pBuffer = nullptr;

		auto buffer = m_BufferMap.find(desc.name);
		if (buffer != m_BufferMap.end())
		{
			pBuffer = (SRConstantBuffer*)(buffer->second);
			pBuffer->AddRef();
		}
		else
		{
			pBuffer = new SRConstantBuffer(desc);
		}

		return pBuffer;
	}

	void SRBufferManager::ReleaseVertexBuffer(SRVertexBuffer* pBuffer)
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

	void SRBufferManager::ReleaseIndexBuffer(SRIndexBuffer* pBuffer)
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

	void SRBufferManager::ReleaseConstantBuffer(SRConstantBuffer* pBuffer)
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