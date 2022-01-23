////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: PhongLightingCommon.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

float3 ComFunc_Phong_Diffuse(float3 lightColor, float luminance, float3 surfaceColor, float NoL)
{
	float3 diffuseColor = lightColor * luminance * surfaceColor * NoL;

	return diffuseColor;
}