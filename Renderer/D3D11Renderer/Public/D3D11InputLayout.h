////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: D3D11InputLayout.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "Vertex.h"

#include <cstdint>
#include <d3d11.h>

namespace RenderDog
{
	class D3D11InputLayout
	{
	public:
		D3D11InputLayout();
		~D3D11InputLayout();

		bool Init(VertexType vertexType, void* compiledCode, uint32_t codeSize);
		void Release();

		void SetToContext();

	private:
		ID3D11InputLayout*	m_InputLayout;
	};

}// namespace RenderDog
