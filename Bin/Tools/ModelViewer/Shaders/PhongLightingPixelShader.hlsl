////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: PhongLightingPixelShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#include "PhongLightingCommon.hlsl"

Texture2D		DiffuseTexture	: register(t0);
SamplerState	LinearSampler	: register(s0);

cbuffer LightingParam : register(b0)
{
	float4	lightColor;
	float3	lightDirection;
	float	luminance;
};

struct VSOutput
{
	float4 Pos		: SV_POSITION;
	float4 Color	: COLOR;
	float3 Normal	: NORMAL;
	float2 TexCoord : TEXCOORD0;
};

float4 Main(VSOutput vsOutput) : SV_Target
{
	float3 BaseColor = DiffuseTexture.Sample(LinearSampler, vsOutput.TexCoord).rgb;
	BaseColor *= vsOutput.Color.rgb;

	float NoL = saturate(dot(vsOutput.Normal, -lightDirection));
	float3 diffuse = ComFunc_Phong_Diffuse(lightColor.rgb, luminance, BaseColor, NoL);

	return float4(diffuse, 1.0f);
}