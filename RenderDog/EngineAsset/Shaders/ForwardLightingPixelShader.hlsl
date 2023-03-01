////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: ForwardLightingPixelShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#include "LightingCommon.hlsl"
//#include "PhongLightingCommon.hlsl"
#include "ShadowTestCommon.hlsl"
#include "Material.hlsl"

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

	float Metallic = ComFunc_Material_GetMetallicRaw(VsOutput.TexCoord);
	float Roughness = ComFunc_Material_GetRoughnessRaw(VsOutput.TexCoord);
	float Specular = ComFunc_Material_GetSpecularRaw(VsOutput.TexCoord);

	float3 DirectionalLighting = ComFunc_Lighting_DirectionalLighting(NoH, NoV, NoL, HoV, BaseColor, Metallic, Roughness, Specular);
	//Shadow
	float3 ShadowPos = VsOutput.ShadowPos.xyz / VsOutput.ShadowPos.w;
	float ShadowFactor = ComFunc_ShadowDepth_GetShadowFactor(ShadowPos);
	DirectionalLighting *= ShadowFactor;

	float3 EnvironmentLighting = ComFunc_Lighting_EnvReflection(NoV, WorldNormal, EyeDir, BaseColor, Metallic, Roughness, Specular);

	float3 FinalColor = DirectionalLighting + EnvironmentLighting;

	//ColorTone
	FinalColor = abs(FinalColor / (FinalColor + 1.0f));
	FinalColor = pow(FinalColor, 1.0f / 2.2f);

	return float4(FinalColor, 1.0f);
}