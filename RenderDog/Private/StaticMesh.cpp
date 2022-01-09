////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: StaticMesh.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "StaticMesh.h"

namespace RenderDog
{
	StaticMesh::StaticMesh() :
		m_Vertices(0),
		m_Indices(0)
	{}

	StaticMesh::~StaticMesh()
	{
		m_Vertices.clear();
		m_Indices.clear();
	}

	void StaticMesh::Render(IPrimitiveRenderer* pPrimitiveRenderer)
	{
		return;
	}

	void StaticMesh::LoadFromData(const std::vector<LocalVertex>& vertices, const std::vector<uint32_t>& indices)
	{
		m_Vertices.assign(vertices.begin(), vertices.end());

		m_Indices.assign(indices.begin(), indices.end());
	}


}// namespace RenderDog