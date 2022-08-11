////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: ShadowTestCommon.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#define PCF_SAMPLE_COUNT 9

Texture2D				ComVar_Texture_ShadowDepthTexture;
SamplerComparisonState	ComVar_Texture_ShadowDepthTextureSampler;

cbuffer ComVar_ConstantBuffer_ShadowParam : register(b1)
{
	float4 ComVar_Vector_ShadowParam0;
};

cbuffer ComVar_ConstantBuffer_ShadowMatrixs : register(b2)
{
	row_major matrix ComVar_Matrix_ShadowView;
	row_major matrix ComVar_Matrix_ShadowProjection;
}

#define ComVar_ShadowDepth_Offset	ComVar_Vector_ShadowParam0.x
#define ComVar_ShadowDepth_RTSize	ComVar_Vector_ShadowParam0.y

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
		ShadowFactor += ComVar_Texture_ShadowDepthTexture.SampleCmpLevelZero(ComVar_Texture_ShadowDepthTextureSampler, ShadowTex + sampleOffset[i], sceneDepthInLightSpace).r;
	}
	
	ShadowFactor /= PCF_SAMPLE_COUNT;

	return ShadowFactor;
}