////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: PhongLightingPixelShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#include "PhongLightingCommon.hlsl"
#include "ShadowTestCommon.hlsl"

Texture2D		DiffuseTexture				: register(t0);
SamplerState	DiffuseTextureSampler		: register(s0);

Texture2D		NormalTexture				: register(t1);
SamplerState	NormalTextureSampler		: register(s1);


cbuffer LightingParam : register(b0)
{
	float4	LightColor;
	float3	LightDirection;
	float	Luminance;
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
	float3 BaseColor = DiffuseTexture.Sample(DiffuseTextureSampler, vsOutput.TexCoord).rgb;
	BaseColor *= vsOutput.Color.rgb;

	float2 TangentNormalXY = NormalTexture.Sample(NormalTextureSampler, vsOutput.TexCoord).xy;
	TangentNormalXY = TangentNormalXY * 2.0f - 1.0f;
	float TangentNormalZ = sqrt(1.0f - TangentNormalXY.x * TangentNormalXY.x - TangentNormalXY.y * TangentNormalXY.y);
	float3 TangentNormal = normalize(float3(TangentNormalXY, TangentNormalZ));

	float3x3 TBN = float3x3(vsOutput.Tangent, vsOutput.BiTangent, vsOutput.Normal);

	float3 WorldNormal = normalize(mul(TangentNormal, TBN));
	
	float NoL = saturate(dot(WorldNormal, -LightDirection));
	float3 Diffuse = ComFunc_Phong_Diffuse(LightColor.rgb, Luminance, BaseColor, NoL);

	//Shadow
	float3 ShadowPos = vsOutput.ShadowPos.xyz / vsOutput.ShadowPos.w;
	float ShadowFactor = ComFunc_ShadowDepth_GetShadowFactor(ShadowPos);
	Diffuse *= ShadowFactor;

	float3 Ambient = float3(0.2f, 0.2f, 0.2f);

	float3 finalColor = saturate(Diffuse + Ambient);

	return float4(finalColor, 1.0f);
}