////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Buffer.h
//Written by Xiang Weikang
//Desc: Buffer Interface
////////////////////////////////////////

#pragma once

#include "RenderDog.h"

#include <cstdint>
#include <string>

namespace RenderDog
{
	struct BufferDesc
	{
		std::string			name;
		uint32_t			byteWidth;
		uint32_t			stride;
		uint32_t			offset;
		void*				pInitData;
		bool				isDynamic;

		BufferDesc() :
			name(""),
			byteWidth(0),
			stride(0),
			offset(0),
			pInitData(nullptr),
			isDynamic(false)
		{}
	};

	class IBuffer
	{
	protected:
		virtual ~IBuffer() = default;

	public:
		virtual void				Release() = 0;

		virtual void				Update(void* srcData, uint32_t srcSize) = 0;

		virtual const std::string&	GetName() const = 0;
		virtual void*				GetResource() = 0;
	};

	class IVertexBuffer : public IBuffer
	{
	protected:
		virtual ~IVertexBuffer() = default;

	public:
		virtual uint32_t			GetStride() const = 0;
		virtual uint32_t			GetOffset() const = 0;
	};

	class IIndexBuffer : public IBuffer
	{
	protected:
		virtual ~IIndexBuffer() = default;

	public:
		virtual uint32_t			GetIndexNum() const = 0;
	};

	class IConstantBuffer : public IBuffer
	{
	protected:
		virtual ~IConstantBuffer() = default;
	};

	class IBufferManager
	{
	public:
		virtual ~IBufferManager() = default;

		virtual IVertexBuffer*			GetVertexBuffer(const BufferDesc& desc) = 0;
		virtual IIndexBuffer*			GetIndexBuffer(const BufferDesc& desc) = 0;
		virtual IConstantBuffer*		GetConstantBuffer(const BufferDesc& desc) = 0;
		virtual IConstantBuffer*		GetConstantBufferByName(const std::string& name) = 0;
	};

	extern IBufferManager* g_pIBufferManager;
	
}// namespace RenderDog