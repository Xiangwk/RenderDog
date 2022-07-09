////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: D3D11InputLayout.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "D3D11InputLayout.h"
#include "D3D11Renderer.h"

#include <vector>

namespace RenderDog
{
	static D3D11_INPUT_ELEMENT_DESC SimpleInputLayout[] =
	{
		{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",			0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	static D3D11_INPUT_ELEMENT_DESC StandardInputLayout[] =
	{
		{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",			0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	static D3D11_INPUT_ELEMENT_DESC SkinInputLayout[] = 
	{
		{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",			0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES",	0, DXGI_FORMAT_R8G8B8A8_UINT,		0, 76, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D11InputLayout::D3D11InputLayout() :
		m_InputLayout(nullptr)
	{}

	D3D11InputLayout::~D3D11InputLayout()
	{}

	bool D3D11InputLayout::Init(VERTEX_TYPE vertexType, void* compiledCode, uint32_t codeSize)
	{
		D3D11_INPUT_ELEMENT_DESC* inputElemDesc = nullptr;
		uint32_t elemNum = 0;
		switch (vertexType)
		{
		case RenderDog::VERTEX_TYPE::SIMPLE:
			inputElemDesc = SimpleInputLayout;
			elemNum = 2;
			break;
		case RenderDog::VERTEX_TYPE::STANDARD:
			inputElemDesc = StandardInputLayout;
			elemNum = 5;
			break;
		case RenderDog::VERTEX_TYPE::SKIN:
			inputElemDesc = SkinInputLayout;
			elemNum = 7;
			break;
		default:
			break;
		}

		if (FAILED(g_pD3D11Device->CreateInputLayout(inputElemDesc, elemNum, compiledCode, codeSize, &m_InputLayout)))
		{
			return false;
		}

		return true;
	}

	void D3D11InputLayout::Release()
	{
		if (m_InputLayout)
		{
			m_InputLayout->Release();
			m_InputLayout = nullptr;
		}
	}

	void D3D11InputLayout::SetToContext()
	{
		g_pD3D11ImmediateContext->IASetInputLayout(m_InputLayout);
	}

}// namespace RenderDog