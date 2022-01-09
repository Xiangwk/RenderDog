////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Buffer.h
//Written by Xiang Weikang
//Desc: Buffer Interface
////////////////////////////////////////

#pragma once

#include <cstdint>

namespace RenderDog
{
	class IBuffer
	{
	public:
		virtual ~IBuffer() = default;

		virtual bool Init() = 0;
		virtual void Release() = 0;

		virtual void Update() = 0;
	};

	class IVertexBuffer : public IBuffer
	{
	public:
		virtual ~IVertexBuffer() = default;

		virtual uint32_t	GetVertexNum() const = 0;
		virtual void*		GetVertexBuffer() = 0;
	};

	class IIndexBuffer : public IBuffer
	{
	public:
		virtual ~IIndexBuffer() = default;

		virtual uint32_t	GetIndexNum() const = 0;
		virtual void*		GetIndexBuffer() = 0;
	};
	
}// namespace RenderDog