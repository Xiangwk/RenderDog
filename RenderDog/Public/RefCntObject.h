////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: RefCntObject.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include <cstdint>

namespace RenderDog
{
	class RefCntObject
	{
	public:
		RefCntObject();
		~RefCntObject();

		void		AddRef();
		uint32_t	SubRef();

	protected:
		uint32_t	m_RefCnt;
	};


}// namespace RenderDog
