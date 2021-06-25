///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Shader.cpp
//Written by Xiang Weikang
///////////////////////////////////

#include "Shader.h"
#include "Matrix.h"
#include "ShaderResourceView.h"
#include "Utility.h"
#include "Light.h"

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

	Vector4 PixelShader::PSMain(const VSOutputVertex& PSInput, const ShaderResourceView* pSRV, DirectionalLight* pDirLight) const
	{
		Vector2 vUV = PSInput.Texcoord;

		Vector4 TextureColor = Sample(pSRV, vUV);
		
		//Debug Normal Map
		float fTangentNormalX = TextureColor.x;
		float fTangentNormalY = TextureColor.y;
		float fTangentNormalZ = TextureColor.z;
		fTangentNormalX = fTangentNormalX * 2.0f - 1.0f;
		fTangentNormalY = fTangentNormalY * 2.0f - 1.0f;
		fTangentNormalZ = fTangentNormalZ * 2.0f - 1.0f;

		Vector3 TangentNormal = Normalize(Vector3(fTangentNormalX, fTangentNormalY, fTangentNormalZ));
		
		Vector3 Tangent = Normalize(Vector3(PSInput.Tangent.x, PSInput.Tangent.y, PSInput.Tangent.z));
		Vector3 BiTangent = Normalize(CrossProduct(PSInput.Normal, Tangent) * PSInput.Tangent.w);
		Vector3 VertexNormal = Normalize(PSInput.Normal);

		Vector4 vT = Vector4(Tangent, 0.0f);
		Vector4 vB = Vector4(BiTangent, 0.0f);
		Vector4 vN = Vector4(VertexNormal, 0.0f);
		
		Matrix4x4 matTBN(vT, vB, vN, Vector4(0, 0, 0, 1));

		Vector4 WorldNormal = Vector4(TangentNormal, 0.0f) * matTBN;

		Vector3 DiffuseColor = CalcPhongLighing(*pDirLight, Vector3(WorldNormal.x, WorldNormal.y, WorldNormal.z), Vector3(1.0f, 1.0f, 1.0f));
		
		return Vector4(DiffuseColor, 1.0f);
	}

	Vector4 PixelShader::Sample(const ShaderResourceView* pSRV, const Vector2& vUV) const
	{
		const Vector4* pData = pSRV->GetView();

		uint32_t nWidth = pSRV->GetWidth();
		uint32_t nHeight = pSRV->GetHeight();

		float fU = vUV.x - std::floor(vUV.x);
		float fV = vUV.y - std::floor(vUV.y);

		uint32_t nRow = (uint32_t)(fV * (nHeight - 1));
		uint32_t nCol = (uint32_t)(fU * (nWidth - 1));

		Vector4 color = pData[nRow * nWidth + nCol];

		return color;
	}

	Vector3 PixelShader::CalcPhongLighing(const DirectionalLight& light, const Vector3& normal, const Vector3& faceColor) const
	{
		Vector3 worldLightDir = -Normalize(light.GetDirection());
		worldLightDir = Normalize(worldLightDir);
		Vector3 worldNormal = Normalize(normal);

		float fDiffuse = DotProduct(worldLightDir, worldNormal);

		fDiffuse = Clamp(fDiffuse, 0.0f, 1.0f);

		//return fDiffuse * light.GetColor() * light.GetLuminance() * faceColor;
		return Vector3(fDiffuse);
	}
}

