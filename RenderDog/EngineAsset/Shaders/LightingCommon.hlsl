////////////////////////////////////////
//RenderDog <·,·>
//FileName: LightingCommon.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

TextureCube		ComVar_Texture_SkyCubeTexture;
SamplerState	ComVar_Texture_SkyCubeTextureSampler;

Texture2D		ComVar_Texture_IblBrdfLutTexture;
SamplerState	ComVar_Texture_IblBrdfLutTextureSampler;

#define REFLECTION_CAPTURE_ROUGHEST_MIP			1
#define REFLECTION_CAPTURE_ROUGHNESS_MIP_SCALE	1.2
#define REFLECTION_CAPTURE_DIFFUSE_MIP			7.0

cbuffer ComVar_ConstantBuffer_LightingParam
{
	float4	ComVar_Vector_DirLightDirection;
	float4	ComVar_Vector_DirLightColor;
};

static const float PI = 3.1415927f;

float LocFunc_Lighting_ComputeReflectionCaptureMip(float Roughness, float CubemapMaxMip)
{
	float LevelFrom1x1 = REFLECTION_CAPTURE_ROUGHEST_MIP - REFLECTION_CAPTURE_ROUGHNESS_MIP_SCALE * log2(max(Roughness, 0.001));
	
	return CubemapMaxMip - 1 - LevelFrom1x1;
}

float3 LocFunc_Lighting_GetOffSpecularPeakReflectionDir(float3 Normal, float3 ReflectionVector, float Roughness)
{
	float a = Roughness * Roughness;

	return lerp(Normal, ReflectionVector, (1 - a) * (sqrt(1 - a) + a));
}

float3 LocFunc_Lighting_DiffuseLambert(float3 diffuseColor)
{
	return diffuseColor / PI;
}

float3 LocFunc_Lighting_GetSpecularColor(float3 BaseColor, float Specular, float Metallic)
{
	float3 F0 = 0.08f * Specular.xxx;
	float3 SpecularColor = lerp(F0, BaseColor, Metallic);

	return SpecularColor;
}

float3 LocFunc_Lighting_FresnelSchlick(float HoV, float3 SpecularColor)
{
	float Fc = pow(1.0f - HoV, 5.0f);

	float3 Ks = saturate(50.0f * SpecularColor.g) * Fc + (1.0f - Fc) * SpecularColor;

	return Ks;
}

float LocFunc_Lighting_DistributionGGX(float NoH, float Roughness)
{
	float A = Roughness * Roughness;
	float A2 = A * A;
	
	float DeNom = (NoH * A2 - NoH) * NoH + 1.0f;
	DeNom = PI * DeNom * DeNom;

	return A2 / DeNom;
}

float LocFunc_Lighting_VisSmithJointApprox(float NoV, float NoL, float Roughness)
{
	float visSmithV = NoL * (NoV * (1.0f - Roughness) + Roughness);
	float visSmithL = NoV * (NoL * (1.0f - Roughness) + Roughness);

	return 0.5f * rcp(visSmithV + visSmithL);
}

float3 LocFunc_Lighting_SpecularGGX(float NoH, float NoV, float NoL, float HoV, float3 SpecularColor, float Roughness)
{
	float D = LocFunc_Lighting_DistributionGGX(NoH, Roughness);
	float Vis = LocFunc_Lighting_VisSmithJointApprox(NoV, NoL, Roughness);

	float3 F = LocFunc_Lighting_FresnelSchlick(HoV, SpecularColor);

	return (D * Vis) * F;
}

float3 LocFunc_Lighting_EnvReflectDiffuse(float3 DiffuseColor, float3 N)
{
	float3 Irradiance = ComVar_Texture_SkyCubeTexture.SampleLevel(ComVar_Texture_SkyCubeTextureSampler, N, REFLECTION_CAPTURE_DIFFUSE_MIP).rgb;

	float3 Diffuse = Irradiance * DiffuseColor;

	return Diffuse;
}

float3 LocFunc_Lighting_EnvBRDF(float3 SpecularColor, float Roughness, float NoV)
{
	float2 AB = ComVar_Texture_IblBrdfLutTexture.SampleLevel(ComVar_Texture_IblBrdfLutTextureSampler, float2(NoV, Roughness), 0).rg;

	float3 GF = SpecularColor * AB.x + saturate(50.0f * SpecularColor.g) * AB.y;

	return GF;
}

float3 LocFunc_Lighting_EnvReflectSpecular(float3 R, float Roughness, float NoV, float3 SpecularColor)
{
	//FIXME!!! 这里应该根据环境贴图的尺寸来计算，环境图为1024*1024，所以MaxMipMap为10；
	const float MaxMipMap = 10.0f;
	float CubemapMipLevel = LocFunc_Lighting_ComputeReflectionCaptureMip(Roughness, MaxMipMap);
	float3 PrefilteredColor = ComVar_Texture_SkyCubeTexture.SampleLevel(ComVar_Texture_SkyCubeTextureSampler, R, CubemapMipLevel).rgb;

	float3 Specular = PrefilteredColor * LocFunc_Lighting_EnvBRDF(SpecularColor, Roughness, NoV);

	return Specular;
}

float3 ComFunc_Lighting_DirectionalLighting(float NoH, float NoV, float NoL, float HoV, float3 BaseColor, float3 SpecularColor, float Metallic, float Roughness)
{
	float3 Radiance = ComVar_Vector_DirLightColor.rgb * ComVar_Vector_DirLightColor.a;

	float3 DiffuseColor = BaseColor * (1.0f - Metallic);
	float3 LightingDiffuse = LocFunc_Lighting_DiffuseLambert(DiffuseColor);

	float3 LightingSpecular = LocFunc_Lighting_SpecularGGX(NoH, NoV, NoL, HoV, SpecularColor, Roughness);

	return (LightingDiffuse + LightingSpecular) * Radiance * NoL;
}

float3 ComFunc_Lighting_EnvReflection(float NoV, float3 N, float3 V, float3 BaseColor, float3 SpecularColor, float Metallic, float Roughness)
{
	float3 DiffuseColor = BaseColor * (1.0f - Metallic);
	float3 AmbientDiffuse = LocFunc_Lighting_EnvReflectDiffuse(DiffuseColor, N);

	float3 R = reflect(-V, N);
	R = LocFunc_Lighting_GetOffSpecularPeakReflectionDir(N, R, Roughness);
	float3 AmbientSpecular = LocFunc_Lighting_EnvReflectSpecular(R, Roughness, NoV, SpecularColor);

	return AmbientDiffuse + AmbientSpecular;
}