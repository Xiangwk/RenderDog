///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Shader.cpp
//Written by Xiang Weikang
///////////////////////////////////

#include "Shader.h"
#include "Matrix.h"
#include "ShaderResourceView.h"
#include "Utility.h"

#include <cstdint>

namespace RenderDog
{
	VSOutputVertex VertexShader::VSMain(const Vertex& inVertex, const Matrix4x4& matWorld, const Matrix4x4& matView, const Matrix4x4& matProj) const
	{
		VSOutputVertex Output = {};
		Vector4 vInPos = Vector4(inVertex.vPosition, 1.0f);
		vInPos = vInPos * matWorld;
		vInPos = vInPos * matView;
		vInPos = vInPos * matProj;

		Output.SVPosition.x = vInPos.x;
		Output.SVPosition.y = vInPos.y;
		Output.SVPosition.z = vInPos.z;
		Output.SVPosition.w = vInPos.w;

		Output.Color = Vector4(inVertex.vColor, 1.0f);

		Output.UV = inVertex.vUV;

		return Output;
	}

	uint32_t PixelShader::PSMain(const VSOutputVertex& PSInput, const ShaderResourceView* pSRV) const
	{
		Vector2 vUV = PSInput.UV;

		uint32_t TextureColor = Sample(pSRV, vUV);

		return TextureColor;
	}

	uint32_t PixelShader::Sample(const ShaderResourceView* pSRV, const Vector2& vUV) const
	{
		const uint32_t* pData = pSRV->GetView();

		uint32_t nWidth = pSRV->GetWidth();
		uint32_t nHeight = pSRV->GetHeight();

		uint32_t nRow = (uint32_t)(vUV.y * (nHeight - 1));
		uint32_t nCol = (uint32_t)(vUV.x * (nWidth - 1));

		uint32_t color = pData[nRow * nWidth + nCol];

		return color;
	}

}

