////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: RefCntObject.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "RefCntObject.h"

namespace RenderDog
{
	RefCntObject::RefCntObject() :
		m_RefCnt(0)
	{}

	RefCntObject::~RefCntObject()
	{}

	void RefCntObject::AddRef()
	{
		++m_RefCnt;
	}

	void RefCntObject::SubRef()
	{
		--m_RefCnt;
		if (m_RefCnt == 0)
		{
			delete this;
		}
	}


}// namespace RenderDog