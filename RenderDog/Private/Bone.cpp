////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Bone.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Bone.h"

namespace RenderDog
{
	Bone::Bone() :
		m_ParentIndex(-1),
		m_OffsetMatrix(),
		m_UpToParentMatrix(),
		m_UpToRootMatrix()
	{}

	Bone::~Bone()
	{}



}// namespace RenderDog