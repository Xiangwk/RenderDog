////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SingleColorPixelShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#include "PhongLightingCommon.hlsl"

struct VSOutput
{
	float4 PosH  : SV_POSITION;
	float3 PosL	 : POSITION;
};

float4 Main(VSOutput VsOutput) : SV_Target
{
	return ComVar_Texture_SkyCubeTexture.SampleLevel(ComVar_Texture_SkyCubeTextureSampler, VsOutput.PosL, 0);
}