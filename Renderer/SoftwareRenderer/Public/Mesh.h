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
	class ISRDevice;
	class ISRDeviceContext;
	class ISRBuffer;
	class IndexBuffer;

	class StaticMesh
	{
	public:
		StaticMesh();
		~StaticMesh();

		StaticMesh(const std::vector<LocalVertex>& vertices, const std::vector<uint32_t>& indices);

		bool Init(ISRDevice* pDevice);
		void Release();

		void Draw(ISRDeviceContext* pDeviceContext);

		void CalculateTangents();

	private:
		std::vector<LocalVertex>		m_RawVertices;
		std::vector<uint32_t>			m_RawIndices;

		std::vector<LocalVertex>		m_Vertices;
		std::vector<uint32_t>			m_Indices;

		ISRBuffer*						m_pVB;
		ISRBuffer*						m_pIB;
	};

}// namespace RenderDog