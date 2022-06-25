////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: ShadowTestCommon.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#define PCF_SAMPLE_COUNT 9

Texture2D				ShadowDepthTexture			: register(t2);
SamplerComparisonState	ShadowDepthTextureSampler	: register(s2);

cbuffer ShadowParam : register(b1)
{
	float4 ShadowParam0;
};

#define ComVar_ShadowDepth_Offset	ShadowParam0.x
#define ComVar_ShadowDepth_RTSize	ShadowParam0.y

float ComFunc_ShadowDepth_GetShadowFactor(float3 ShadowPos)
{
	//Shadow
	float2 ShadowTex = float2(ShadowPos.x * 0.5f + 0.5f, 0.5f - ShadowPos.y * 0.5f);

	float sceneDepthInLightSpace = ShadowPos.z - ComVar_ShadowDepth_Offset;
	
	const float dx = 1.0f / ComVar_ShadowDepth_RTSize;
	const float2 sampleOffset[PCF_SAMPLE_COUNT] =
	{
		float2(-dx, -dx), float2(0.0f, -dx), float2(+dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(+dx, 0.0f),
		float2(-dx, +dx), float2(0.0f, +dx), float2(+dx, +dx)
	};

	float ShadowFactor = 0.0f;
	[unroll]
	for (int i = 0; i < PCF_SAMPLE_COUNT; ++i)
	{
		// no need to do this manually, to DXGI_FORMAT_R24_UNORM_X8_TYPELESS srv, SampleCmpLevelZero can clamp sceneDepthInLightSpace to 0~1;
		// clamp sceneDepthInLightSpace to 0~1 can make the pixel that depth is larger than the light camera's viewport's far plane get a correct shadow factor
		//sceneDepthInLightSpace = clamp(sceneDepthInLightSpace, 0.0f, 1.0f);
		ShadowFactor += ShadowDepthTexture.SampleCmpLevelZero(ShadowDepthTextureSampler, ShadowTex + sampleOffset[i], sceneDepthInLightSpace).r;
	}
	
	ShadowFactor /= PCF_SAMPLE_COUNT;

	return ShadowFactor;
}