////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: PhongLightingPixelShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#include "PhongLightingCommon.hlsl"

cbuffer LightingParam : register(b0)
{
	float3	lightDirection;
	float3	lightColor;
	float	luminance;
	float	padding;
};

struct VSOutput
{
	float4 Pos		: SV_POSITION;
	float4 Color	: COLOR;
	float3 Normal	: NORMAL;
};

float4 Main(VSOutput vsOutput) : SV_Target
{
	float NoL = saturate(dot(vsOutput.Normal, -lightDirection));
	float3 diffuse = ComFunc_Phong_Diffuse(lightColor, luminance, vsOutput.Color, NoL);

	return float4(diffuse, 1.0f);
}