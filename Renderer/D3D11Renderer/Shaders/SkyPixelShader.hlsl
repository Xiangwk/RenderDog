////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SingleColorPixelShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#include "PhongLightingCommon.hlsl"

struct VSOutput
{
	float4 Pos   : SV_POSITION;
	float3 PosL	 : POSITION;
};

float4 Main(VSOutput vsOutput) : SV_Target
{
	return ComVar_Texture_SkyCubeTexture.Sample(ComVar_Texture_SkyCubeTextureSampler, vsOutput.PosL);
}