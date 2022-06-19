////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: ShadowTestCommon.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

Texture2D		ShadowDepthTexture	: register(t1);
SamplerState	ShadowMapSampler	: register(s1);

cbuffer ShadowParam : register(b1)
{
	float4 ShadowParam0;
};

#define ComVar_ShadowDepth_Offset ShadowParam0.x;

float ComFunc_ShadowDepth_GetShadowFactor(float3 ShadowPos)
{
	//Shadow
	float2 ShadowTex = float2(ShadowPos.x * 0.5f + 0.5f, 0.5f - ShadowPos.y * 0.5f);
	float ShadowDepth = ShadowDepthTexture.Sample(ShadowMapSampler, ShadowTex).r;
	ShadowDepth += ComVar_ShadowDepth_Offset;

	float ShadowFactor = ShadowPos.z <= ShadowDepth ? 0.0f : 1.0f;
	
	return ShadowFactor;
}