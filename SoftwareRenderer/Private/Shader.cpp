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

		Vector4 vNormal = Vector4(inVertex.vNormal, 0.0f);
		vNormal = vNormal * matWorld;
		Output.Normal.x = vNormal.x;
		Output.Normal.y = vNormal.y;
		Output.Normal.z = vNormal.z;

		Vector4 vTangent = Vector4(inVertex.vTangent.x, inVertex.vTangent.y, inVertex.vTangent.z, 0.0f);
		vTangent = vTangent * matWorld;
		Output.Tangent.x = vTangent.x;
		Output.Tangent.y = vTangent.y;
		Output.Tangent.z = vTangent.z;
		Output.Tangent.w = inVertex.vTangent.w;

		Output.Texcoord = inVertex.vTexcoord;

		return Output;
	}

	uint32_t PixelShader::PSMain(const VSOutputVertex& PSInput, const ShaderResourceView* pSRV) const
	{
		Vector2 vUV = PSInput.Texcoord;

		uint32_t TextureColor = Sample(pSRV, vUV);
		
		//Debug Normal Map
		float fTangentNormalX = (TextureColor >> 16 & 0x000000ff) / 255.0f;
		float fTangentNormalY = (TextureColor >> 8 & 0x000000ff) / 255.0f;
		fTangentNormalX = fTangentNormalX * 2.0f - 1.0f;
		fTangentNormalY = fTangentNormalY * 2.0f - 1.0f;

		float fTangentNormalZ = sqrt(fTangentNormalX * fTangentNormalX + fTangentNormalY * fTangentNormalY);

		Vector3 vTangentNormal = Normalize(Vector3(fTangentNormalX, fTangentNormalY, fTangentNormalZ));
		
		Vector3 vTangent = Vector3(PSInput.Tangent.x, PSInput.Tangent.y, PSInput.Tangent.z);
		Vector3 vBiTangent = CrossProduct(PSInput.Normal, vTangent) * PSInput.Tangent.w;

		Vector4 vT = Vector4(vTangent, 0.0f);
		Vector4 vB = Vector4(vBiTangent, 0.0f);
		Vector4 vN = Vector4(PSInput.Normal, 0.0f);
		
		Matrix4x4 matTBN(vT, vB, vN, Vector4(0, 0, 0, 1));

		Vector4 vNormal = Vector4(vTangentNormal, 0.0f) * matTBN;

		Vector3 vWorldNormal = Vector3(vNormal.x, vNormal.y, vNormal.z);
		vWorldNormal.x = vWorldNormal.x * 0.5f + 0.5f;
		vWorldNormal.y = vWorldNormal.y * 0.5f + 0.5f;
		vWorldNormal.z = vWorldNormal.z * 0.5f + 0.5f;

		float NormalColor[4] = { vWorldNormal.x, vWorldNormal.y, vWorldNormal.z, 1.0f };

		return ConvertFloatColorToUInt32(NormalColor);
	}

	uint32_t PixelShader::Sample(const ShaderResourceView* pSRV, const Vector2& vUV) const
	{
		const uint32_t* pData = pSRV->GetView();

		uint32_t nWidth = pSRV->GetWidth();
		uint32_t nHeight = pSRV->GetHeight();

		float fU = vUV.x - std::floor(vUV.x);
		float fV = vUV.y - std::floor(vUV.y);

		uint32_t nRow = (uint32_t)(fV * (nHeight - 1));
		uint32_t nCol = (uint32_t)(fU * (nWidth - 1));

		uint32_t color = pData[nRow * nWidth + nCol];

		return color;
	}

}

