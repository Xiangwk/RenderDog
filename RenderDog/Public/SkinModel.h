////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SkinModel.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "SkinMesh.h"

#include <vector>

namespace RenderDog
{
	class SkinModel
	{
	public:
		SkinModel();
		~SkinModel();



	private:
		std::vector<SkinMesh*>		m_Meshes;

		AABB						m_AABB;
		BoundingSphere				m_BoundingSphere;
	};

}// namespace RenderDog
