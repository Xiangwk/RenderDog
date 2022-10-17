////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: PhongLightingPixelShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#include "LightingCommon.hlsl"
//#include "PhongLightingCommon.hlsl"
#include "ShadowTestCommon.hlsl"
#include "MaterialShaders/Material.hlsl"

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
	float3 BaseColor = ComFunc_Material_GetBaseColorRaw(VsOutput.TexCoord);
	BaseColor *= VsOutput.Color.rgb;

	float3 TangentNormal = ComFunc_Material_GetNormalRaw(VsOutput.TexCoord);
	float3x3 TBN = float3x3(VsOutput.Tangent, VsOutput.BiTangent, VsOutput.Normal);
	float3 WorldNormal = normalize(mul(TangentNormal, TBN));
	
	float LightLuminance = ComVar_Vector_DirLightColor.a;
	float NoL = saturate(dot(WorldNormal, -ComVar_Vector_DirLightDirection.xyz));

	float3 EyeDir = normalize(VsOutput.EyePosW.xyz - VsOutput.PosW.xyz);
	float NoV = saturate(dot(WorldNormal, EyeDir));

	float3 H = normalize(-ComVar_Vector_DirLightDirection.xyz + EyeDir);
	float NoH = saturate(dot(WorldNormal, H));

	float HoV = saturate(dot(H, EyeDir));

	float Metallic = 0.0f;
	float Roughness = 0.25f;

	float3 LightResult = ComFunc_Lighting_DirectionalLighting(NoH, NoV, NoL, HoV, BaseColor, Metallic, Roughness);
	//Shadow
	float3 ShadowPos = VsOutput.ShadowPos.xyz / VsOutput.ShadowPos.w;
	float ShadowFactor = ComFunc_ShadowDepth_GetShadowFactor(ShadowPos);
	LightResult *= ShadowFactor;

	float3 Ambient = ComFunc_Lighting_EnvReflection(NoV, WorldNormal, BaseColor, Metallic, Roughness);

	float3 FinalColor = LightResult + Ambient * 0.3f;

	//ColorTone
	FinalColor = abs(FinalColor / (FinalColor + 1.0f));
	FinalColor = pow(FinalColor, 1.0f / 2.2f);

	return float4(FinalColor, 1.0f);
}