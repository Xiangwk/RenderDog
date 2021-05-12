///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Shader.cpp
//Written by Xiang Weikang
///////////////////////////////////

#include "Shader.h"
#include "Matrix.h"
#include "ShaderResourceView.h"

#include <cstdint>

namespace RenderDog
{
	VSOutput VertexShader::VSMain(const Vertex& inVertex, const Matrix4x4& matWorld, const Matrix4x4& matView, const Matrix4x4& matProj) const
	{
		VSOutput Output = {};
		Vector4 vInPos = Vector4(inVertex.vPostion, 1.0f);
		vInPos = vInPos * matWorld;
		vInPos = vInPos * matView;
		vInPos = vInPos * matProj;

		Output.fInvZ = 1.0f / vInPos.w;

		vInPos = vInPos / vInPos.w;

		Output.vert.vPostion = Vector3(vInPos.x, vInPos.y, vInPos.z);
		Output.vert.vColor = inVertex.vColor;
		Output.vert.vUV = inVertex.vUV;

		return Output;
	}

	uint32_t PixelShader::PSMain(const VSOutput& PSInput, const ShaderResourceView* pSRV) const
	{
		Vector2 vUV = PSInput.vert.vUV;

		uint32_t TextureColor = Sample(pSRV, vUV);

		return TextureColor;
	}

	uint32_t PixelShader::Sample(const ShaderResourceView* pSRV, const Vector2& vUV) const
	{
		const uint32_t* pData = pSRV->GetView();

		uint32_t nRow = (uint32_t)(vUV.y * 256);
		uint32_t nCol = (uint32_t)(vUV.x * 256);

		uint32_t color = pData[nRow * 256 + nCol];

		return color;
	}

}

