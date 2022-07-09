////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SkinMesh.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "Vertex.h"

#include <vector>
#include <string>

namespace RenderDog
{
	class SkinMesh
	{
	public:
		SkinMesh();
		~SkinMesh();


	private:
		std::string						m_Name;

		std::vector<SkinVertex>			m_Vertices;
		std::vector<uint32_t>			m_Indices;
	};

}// namespace RenderDog
