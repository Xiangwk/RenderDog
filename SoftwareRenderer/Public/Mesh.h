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
	class Device;
	class DeviceContext;
	class VertexBuffer;
	class IndexBuffer;

	class StaticMesh
	{
	public:
		StaticMesh();
		~StaticMesh();

		StaticMesh(const std::vector<Vertex>& Vertices, const std::vector<uint32_t>& Indices);

		bool Init(Device* pDevice);
		void Release();

		void Draw(DeviceContext* pDeviceContext);

	private:
		std::vector<Vertex>		m_Vertices;
		std::vector<uint32_t>	m_Indices;

		VertexBuffer*			m_pVB;
		IndexBuffer*			m_pIB;
	};
}