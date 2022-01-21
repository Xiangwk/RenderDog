////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Buffer.h
//Written by Xiang Weikang
//Desc: Buffer Interface
////////////////////////////////////////

#pragma once

#include "RenderDog.h"

#include <cstdint>

namespace RenderDog
{
	struct BufferDesc
	{
		uint32_t	byteWidth;
		void*		pInitData;
		bool		isDynamic;

		BufferDesc() :
			byteWidth(0),
			pInitData(nullptr),
			isDynamic(false)
		{}
	};

	class IBuffer
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

		virtual bool		Init(const BufferDesc& desc, uint32_t stride, uint32_t offset) = 0;

		virtual uint32_t	GetStride() const = 0;
		virtual uint32_t	GetOffset() const = 0;
	};

	class IIndexBuffer : public IBuffer
	{
	public:
		virtual ~IIndexBuffer() = default;

		virtual bool		Init(const BufferDesc& desc, uint32_t indexNum) = 0;

		virtual void*		GetIndexBuffer() = 0;

		virtual uint32_t	GetIndexNum() const = 0;
	};

	class IConstantBuffer : public IBuffer
	{
	public:
		virtual ~IConstantBuffer() = default;

		virtual bool		Init(const BufferDesc& desc) = 0;

		virtual void*		GetConstantBuffer() = 0;
	};

	class IBufferManager
	{
	public:
		virtual ~IBufferManager() = default;

		virtual IVertexBuffer*		CreateVertexBuffer() = 0;
		virtual IIndexBuffer*		CreateIndexBuffer() = 0;
		virtual IConstantBuffer*	CreateConstantBuffer() = 0;
	};

	extern IBufferManager* g_pIBufferManager;
	
}// namespace RenderDog