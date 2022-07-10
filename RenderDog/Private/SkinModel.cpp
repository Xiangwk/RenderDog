////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SkinModel.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "SkinModel.h"

namespace RenderDog
{
	SkinModel::SkinModel():
		m_Meshes(0),
		m_AABB(),
		m_BoundingSphere()
	{}

	SkinModel::~SkinModel()
	{
		m_Meshes.clear();
	}



}// namespace RenderDog