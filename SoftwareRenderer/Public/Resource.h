///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Resource.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

namespace RenderDog
{
	class IResource
	{
	public:
		virtual void AddRef() = 0;
		virtual void Release() = 0;
	};
}