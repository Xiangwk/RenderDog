////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: StaticMesh.h
//Written by Xiang Weikang
////////////////////////////////////////

#include "Vertex.h"
#include "Primitive.h"

#include <vector>

#pragma once

namespace RenderDog
{
	class StaticMesh : public IPrimitive
	{
	public:
		StaticMesh();
		~StaticMesh();

		StaticMesh(const StaticMesh&) = default;
		StaticMesh& operator=(const StaticMesh&) = default;

		virtual void Render(IPrimitiveRenderer* pPrimitiveRenderer) override;

		void LoadFromData(const std::vector<LocalVertex>& vertices, const std::vector<uint32_t>& indices);

	private:
		std::vector<LocalVertex>	m_Vertices;
		std::vector<uint32_t>		m_Indices;
	};

}// namespace RenderDog
