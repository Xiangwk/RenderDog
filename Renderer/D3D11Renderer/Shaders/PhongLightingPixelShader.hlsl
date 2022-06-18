////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: PhongLightingPixelShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#include "PhongLightingCommon.hlsl"

Texture2D		NormalTexture		: register(t0);
SamplerState	LinearSampler		: register(s0);

Texture2D		ShadowDepthTexture	: register(t1);


cbuffer LightingParam : register(b0)
{
	float4	lightColor;
	float3	lightDirection;
	float	luminance;
};

struct VSOutput
{
	float4 Pos			: SV_POSITION;
	float4 Color		: COLOR;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 BiTangent	: BITANGENT;
	float2 TexCoord		: TEXCOORD0;
	float4 ShadowPos	: TEXCOORD1;
};

float4 Main(VSOutput vsOutput) : SV_Target
{
	float3 BaseColor = float3(1.0f, 1.0f, 1.0f);
	BaseColor *= vsOutput.Color.rgb;

	float2 TangentNormalXY = NormalTexture.Sample(LinearSampler, vsOutput.TexCoord).xy;
	TangentNormalXY = TangentNormalXY * 2.0f - 1.0f;
	float TangentNormalZ = sqrt(1.0f - TangentNormalXY.x * TangentNormalXY.x - TangentNormalXY.y * TangentNormalXY.y);
	float3 TangentNormal = normalize(float3(TangentNormalXY, TangentNormalZ));

	float3x3 TBN = float3x3(vsOutput.Tangent, vsOutput.BiTangent, vsOutput.Normal);

	float3 WorldNormal = normalize(mul(TangentNormal, TBN));
	
	float NoL = saturate(dot(WorldNormal, -lightDirection));
	float3 Diffuse = ComFunc_Phong_Diffuse(lightColor.rgb, luminance, BaseColor, NoL);

	//Shadow
	float2 ShadowTex = float2(vsOutput.ShadowPos.x * 0.5f + 0.5f, 0.5f - vsOutput.ShadowPos.y * 0.5f);
	float ShadowDepth = ShadowDepthTexture.Sample(LinearSampler, ShadowTex).r;
	ShadowDepth += 0.01f;
	float ShadowFactor = vsOutput.ShadowPos.z <= ShadowDepth ? 1.0f : 0.0f;
	Diffuse *= ShadowFactor;

	float3 Ambient = float3(0.2f, 0.2f, 0.2f);

	float3 finalColor = saturate(Diffuse + Ambient);

	return float4(finalColor, 1.0f);
}