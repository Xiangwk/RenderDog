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
		virtual ~RefCntObject();

		void		AddRef();
		void		SubRef();

	protected:
		uint32_t	m_RefCnt;
	};


}// namespace RenderDog
