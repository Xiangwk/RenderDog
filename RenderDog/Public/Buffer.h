////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Buffer.h
//Written by Xiang Weikang
//Desc: Buffer Interface
////////////////////////////////////////

#pragma once

#include "RenderDog.h"
#include "RefCntObject.h"

#include <cstdint>

namespace RenderDog
{
	enum class BUFFER_BIND
	{
		NONE,
		VERTEX,
		INDEX,
		CONSTANT
	};

	struct BufferDesc
	{
		BUFFER_BIND			bufferBind;
		uint32_t			byteWidth;
		uint32_t			stride;
		uint32_t			offset;
		void*				pInitData;
		bool				isDynamic;

		BufferDesc() :
			bufferBind(BUFFER_BIND::NONE),
			byteWidth(0),
			stride(0),
			offset(0),
			pInitData(nullptr),
			isDynamic(false)
		{}
	};

	class IBuffer : public RefCntObject
	{
	protected:
		virtual ~IBuffer() = default;

	public:
		virtual void		Release() = 0;

		virtual void		Update(void* srcData, uint32_t srcSize) = 0;
	};

	class IVertexBuffer : public IBuffer
	{
	protected:
		virtual ~IVertexBuffer() = default;

	public:
		virtual void*		GetVertexBuffer() = 0;

		virtual uint32_t	GetStride() const = 0;
		virtual uint32_t	GetOffset() const = 0;
	};

	class IIndexBuffer : public IBuffer
	{
	protected:
		virtual ~IIndexBuffer() = default;

	public:
		virtual void*		GetIndexBuffer() = 0;
		virtual uint32_t	GetIndexNum() const = 0;
	};

	class IConstantBuffer : public IBuffer
	{
	protected:
		virtual ~IConstantBuffer() = default;

	public:
		virtual void*		GetConstantBuffer() = 0;
	};

	class IBufferManager
	{
	public:
		virtual ~IBufferManager() = default;

		virtual IBuffer*	CreateBuffer(const BufferDesc& desc) = 0;
		virtual void		ReleaseBuffer(IBuffer* pBuffer) = 0;
	};

	extern IBufferManager* g_pIBufferManager;
	
}// namespace RenderDog