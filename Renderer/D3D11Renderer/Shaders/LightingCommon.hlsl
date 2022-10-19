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

float3 LocFunc_Lighting_FresnelSchlick(float HoV, float3 BaseColor, float Metallic)
{
	float3 F0 = 0.04f;
	F0 = lerp(F0, BaseColor, Metallic);

	float3 Ks = F0 + (1.0f - F0) * pow(1.0f - HoV, 5.0f);

	return Ks;
}

float3 LocFunc_Lighting_FresnelSchlickRoughness(float NoV, float3 BaseColor, float Metallic, float Roughness)
{
	float3 F0 = 0.04f;
	F0 = lerp(F0, BaseColor, Metallic);

	float3 Ks = F0 + (max(1.0f - Roughness, F0) - F0) * pow(1.0f - NoV, 5.0f);

	return Ks;
}

float LocFunc_Lighting_DistributionGGX(float NoH, float Roughness)
{
	float A = Roughness * Roughness;
	float Nom = A * A;

	float NoH2 = NoH * NoH;
	
	float DeNom = (NoH2 * (Nom - 1.0f) + 1.0f);
	DeNom = PI * DeNom * DeNom;

	return Nom / DeNom;
}

float LocFunc_Lighting_GeometrySchlickGGX(float NoV, float Roughness)
{
	float R = (Roughness + 1.0f);
	float K = (R * R) / 8.0f;

	float Nom = NoV;
	float DeNom = NoV * (1.0f - K) + K;

	return Nom / DeNom;
}

float LocFunc_Lighting_GeometrySmith(float NoV, float NoL, float Roughness)
{
	float GGX1 = LocFunc_Lighting_GeometrySchlickGGX(NoL, Roughness);
	float GGX2 = LocFunc_Lighting_GeometrySchlickGGX(NoV, Roughness);

	return GGX1 * GGX2;
}

float3 LocFunc_Lighting_Specular(float NoH, float NoV, float NoL, float3 BaseColor, float Roughness, float3 Fs)
{
	float D = LocFunc_Lighting_DistributionGGX(NoH, Roughness);
	float G = LocFunc_Lighting_GeometrySmith(NoV, NoL, Roughness);

	float3 Nom = Fs * D * G;
	float DeNom = 4.0f * NoV * NoL + 0.01f;

	return Nom / DeNom;
}

float3 LocFunc_Lighting_EnvReflectDiffuse(float3 BaseColor, float3 N, float3 Kd)
{
	float3 Irradiance = ComVar_Texture_SkyCubeTexture.SampleLevel(ComVar_Texture_SkyCubeTextureSampler, N, 10).rgb;

	float3 Diffuse = Irradiance * BaseColor;

	return Kd * Diffuse;
}

float3 LocFunc_Lighting_EnvReflectSpecular(float3 R, float Roughness, float NoV, float3 Ks)
{
	const float MaxMipMap = 11.0f;
	float3 PrefilteredColor = ComVar_Texture_SkyCubeTexture.SampleLevel(ComVar_Texture_SkyCubeTextureSampler, R, Roughness * MaxMipMap).rgb;

	float2 EnvBRDF = ComVar_Texture_IblBrdfLutTexture.SampleLevel(ComVar_Texture_IblBrdfLutTextureSampler, float2(NoV, Roughness), 0).rg;

	float3 Specular = PrefilteredColor * (Ks * EnvBRDF.x + EnvBRDF.y);

	return Specular;
}

float3 ComFunc_Lighting_DirectionalLighting(float NoH, float NoV, float NoL, float HoV, float3 BaseColor, float Metallic, float Roughness)
{
	float3 Ks = LocFunc_Lighting_FresnelSchlick(HoV, BaseColor, Metallic);

	float3 Kd = 1.0f - Ks;

	Kd *= (1.0f - Metallic);

	float3 Radiance = ComVar_Vector_DirLightColor.rgb * ComVar_Vector_DirLightColor.a;

	float3 Specular = LocFunc_Lighting_Specular(NoH, NoV, NoL, BaseColor, Roughness, Ks);

	return (Kd * BaseColor / PI + Specular) * Radiance * NoL;
}

float3 ComFunc_Lighting_EnvReflection(float NoV, float3 N, float3 V, float3 BaseColor, float Metallic, float Roughness)
{
	float3 Ks = LocFunc_Lighting_FresnelSchlickRoughness(NoV, BaseColor, Metallic, Roughness);

	float3 Kd = 1.0f - Ks;
	Kd *= 1.0f - Metallic;
	float3 AmbientDiffuse = LocFunc_Lighting_EnvReflectDiffuse(BaseColor, N, Kd);

	float3 R = reflect(-V, N);
	float3 AmbientSpecular = LocFunc_Lighting_EnvReflectSpecular(R, Roughness, NoV, Ks);

	return AmbientDiffuse + AmbientSpecular;
}