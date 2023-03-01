////////////////////////////////////////
//RenderDog <��,��>
//FileName: LightingCommon.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

TextureCube		ComVar_Texture_SkyCubeTexture;
SamplerState	ComVar_Texture_SkyCubeTextureSampler;

Texture2D		ComVar_Texture_IblBrdfLutTexture;
SamplerState	ComVar_Texture_IblBrdfLutTextureSampler;

cbuffer ComVar_ConstantBuffer_LightingParam
{
	float4	ComVar_Vector_DirLightDirection;
	float4	ComVar_Vector_DirLightColor;
};

static const float PI = 3.1415927f;

float3 LocFunc_Lighting_DiffuseLambert(float3 diffuseColor)
{
	return diffuseColor / PI;
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
	float3 Irradiance = ComVar_Texture_SkyCubeTexture.SampleLevel(ComVar_Texture_SkyCubeTextureSampler, N, 7.0f).rgb;

	float3 Diffuse = Irradiance * DiffuseColor;

	return Diffuse;
}

float3 LocFunc_Lighting_EnvReflectSpecular(float3 R, float Roughness, float NoV, float3 SpecularColor)
{
	float3 Ks = LocFunc_Lighting_FresnelSchlick(NoV, SpecularColor);

	const float MaxMipMap = 11.0f;
	float3 PrefilteredColor = ComVar_Texture_SkyCubeTexture.SampleLevel(ComVar_Texture_SkyCubeTextureSampler, R, Roughness * MaxMipMap).rgb;

	float2 EnvBRDF = ComVar_Texture_IblBrdfLutTexture.SampleLevel(ComVar_Texture_IblBrdfLutTextureSampler, float2(NoV, Roughness), 0).rg;

	float3 Specular = PrefilteredColor * (Ks * EnvBRDF.x + EnvBRDF.y);

	return Specular;
}

float3 ComFunc_Lighting_DirectionalLighting(float NoH, float NoV, float NoL, float HoV, float3 BaseColor, float Metallic, float Roughness, float Specular)
{
	float3 Radiance = ComVar_Vector_DirLightColor.rgb * ComVar_Vector_DirLightColor.a;

	float3 DiffuseColor = BaseColor * (1.0f - Metallic);
	float3 LightingDiffuse = LocFunc_Lighting_DiffuseLambert(DiffuseColor);

	float3 F0 = 0.08f * Specular.xxx;
	float3 SpecularColor = lerp(F0, BaseColor, Metallic);
	float3 LightingSpecular = LocFunc_Lighting_SpecularGGX(NoH, NoV, NoL, HoV, SpecularColor, Roughness);

	return (LightingDiffuse + LightingSpecular) * Radiance * NoL;
}

float3 ComFunc_Lighting_EnvReflection(float NoV, float3 N, float3 V, float3 BaseColor, float Metallic, float Roughness, float Specular)
{
	float3 DiffuseColor = BaseColor * (1.0f - Metallic);
	float3 AmbientDiffuse = LocFunc_Lighting_EnvReflectDiffuse(DiffuseColor, N);

	float3 R = reflect(-V, N);
	float3 F0 = 0.08f * Specular.xxx;
	float3 SpecularColor = lerp(F0, BaseColor, Metallic);
	float3 AmbientSpecular = LocFunc_Lighting_EnvReflectSpecular(R, Roughness, NoV, SpecularColor);

	return AmbientDiffuse + AmbientSpecular;
}