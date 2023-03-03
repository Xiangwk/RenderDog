////////////////////////////////////////
//RenderDog <·,·>
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
	float4 skyTextureColor = ComVar_Texture_SkyCubeTexture.SampleLevel(ComVar_Texture_SkyCubeTextureSampler, VsOutput.PosL, 0);

	////FIXME!!! 导入的CubeMap不是HDR，亮度不够，临时乘上一个倍数
	return skyTextureColor * 1.3f;
}