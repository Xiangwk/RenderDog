///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Model.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include "Vertex.h"

#include <vector>

namespace RenderDog
{
	class IDevice;
	class IDeviceContext;
	class VertexBuffer;
	class IndexBuffer;

	class StaticMesh
	{
	public:
		StaticMesh();
		~StaticMesh();

		StaticMesh(const std::vector<Vertex>& Vertices, const std::vector<uint32_t>& Indices);

		bool Init(IDevice* pDevice);
		void Release();

		void Draw(IDeviceContext* pDeviceContext);

		void CalculateTangents();

	private:
		std::vector<Vertex>		m_RawVertices;
		std::vector<uint32_t>	m_RawIndices;

		std::vector<Vertex>		m_Vertices;
		std::vector<uint32_t>	m_Indices;

		VertexBuffer*			m_pVB;
		IndexBuffer*			m_pIB;
	};
}