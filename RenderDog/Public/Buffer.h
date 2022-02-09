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
	enum class BufferBind
	{
		NONE,
		VERTEX,
		INDEX,
		CONSTANT
	};

	struct BufferDesc
	{
		BufferBind	bufferBind;
		uint32_t	byteWidth;
		uint32_t	stride;
		uint32_t	offset;
		void*		pInitData;
		bool		isDynamic;

		BufferDesc() :
			bufferBind(BufferBind::NONE),
			byteWidth(0),
			stride(0),
			offset(0),
			pInitData(nullptr),
			isDynamic(false)
		{}
	};

	class IBuffer : public RefCntObject
	{
	public:
		virtual ~IBuffer() = default;

		virtual void		Release() = 0;

		virtual void		Update(void* srcData, uint32_t srcSize) = 0;
	};

	class IVertexBuffer : public IBuffer
	{
	public:
		virtual ~IVertexBuffer() = default;

		virtual void*		GetVertexBuffer() = 0;

		virtual uint32_t	GetStride() const = 0;
		virtual uint32_t	GetOffset() const = 0;
	};

	class IIndexBuffer : public IBuffer
	{
	public:
		virtual ~IIndexBuffer() = default;

		virtual void*		GetIndexBuffer() = 0;

		virtual uint32_t	GetIndexNum() const = 0;
	};

	class IConstantBuffer : public IBuffer
	{
	public:
		virtual ~IConstantBuffer() = default;

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