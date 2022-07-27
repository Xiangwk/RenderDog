////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: PhongLightingPixelShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#include "PhongLightingCommon.hlsl"
#include "ShadowTestCommon.hlsl"

Texture2D		DiffuseTexture				: register(t1);
SamplerState	DiffuseTextureSampler		: register(s1);

Texture2D		NormalTexture				: register(t2);
SamplerState	NormalTextureSampler		: register(s2);


struct VSOutput
{
	float4 PosH			: SV_POSITION;
	float4 PosW			: POSITION;
	float4 Color		: COLOR;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 BiTangent	: BITANGENT;
	float2 TexCoord		: TEXCOORD0;
	float4 ShadowPos	: TEXCOORD1;
	float4 EyePosW		: TEXCOORD2;
};

float4 Main(VSOutput VsOutput) : SV_Target
{
	float3 BaseColor = DiffuseTexture.Sample(DiffuseTextureSampler, VsOutput.TexCoord).rgb;
	BaseColor *= VsOutput.Color.rgb;

	float2 TangentNormalXY = NormalTexture.Sample(NormalTextureSampler, VsOutput.TexCoord).xy;
	TangentNormalXY = TangentNormalXY * 2.0f - 1.0f;
	float TangentNormalZ = sqrt(1.0f - TangentNormalXY.x * TangentNormalXY.x - TangentNormalXY.y * TangentNormalXY.y);
	float3 TangentNormal = normalize(float3(TangentNormalXY, TangentNormalZ));

	float3x3 TBN = float3x3(VsOutput.Tangent, VsOutput.BiTangent, VsOutput.Normal);

	float3 WorldNormal = normalize(mul(TangentNormal, TBN));
	
	float NoL = saturate(dot(WorldNormal, -ComVar_Vector_DirLightDirection));
	float3 Diffuse = ComFunc_Phong_Diffuse(ComVar_Vector_DirLightColor.rgb, ComVar_Vector_DirLightLuminance, BaseColor, NoL);

	float3 EyeDir = normalize(VsOutput.EyePosW.xyz - VsOutput.PosW.xyz);
	float3 ReflectionColor = ComFunc_Phong_Specular(EyeDir, WorldNormal);

	float3 LightResult = Diffuse;
	//Shadow
	float3 ShadowPos = VsOutput.ShadowPos.xyz / VsOutput.ShadowPos.w;
	float ShadowFactor = ComFunc_ShadowDepth_GetShadowFactor(ShadowPos);
	LightResult *= ShadowFactor;

	float3 Ambient = ReflectionColor * 0.4f;

	float3 FinalColor = saturate(LightResult + Ambient);

	return float4(FinalColor, 1.0f);
}