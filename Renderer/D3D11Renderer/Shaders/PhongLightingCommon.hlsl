////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: PhongLightingCommon.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

TextureCube		ComVar_Texture_SkyCubeTexture;
SamplerState	ComVar_Texture_SkyCubeTextureSampler;

cbuffer LightingParam : register(b0)
{
	float4	ComVar_Vector_DirLightColor;
	float3	ComVar_Vector_DirLightDirection;
	float	ComVar_Vector_DirLightLuminance;
};

float3 ComFunc_Phong_Diffuse(float3 lightColor, float luminance, float3 surfaceColor, float NoL)
{
	float3 diffuseColor = lightColor * luminance * surfaceColor * NoL;

	return diffuseColor;
}

float3 ComFunc_Phong_Specular(float3 EyeDir, float3 Normal)
{
	float3 ReflectDir = reflect(-EyeDir, Normal);

	float3 SpecularColor = ComVar_Texture_SkyCubeTexture.Sample(ComVar_Texture_SkyCubeTextureSampler, ReflectDir).rgb;

	return SpecularColor;
}