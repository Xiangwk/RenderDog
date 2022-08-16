////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: ShadowDepthStaticModelVertexShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#include "Common.hlsl"
#include "ShadowTestCommon.hlsl"

struct VSInput
{
	float3 PosL			: POSITION;
	float4 Color		: COLOR;
	float3 Normal		: NORMAL;
	float4 Tangent		: TANGENT;
	float2 Texcoord		: TEXCOORD;
};

struct VSOutput
{
	float4 PosH			: SV_POSITION;
};

VSOutput Main(VSInput VsInput)
{
	VSOutput VsOutput = (VSOutput)0;
	float4 PosL = float4(VsInput.PosL, 1.0f);
	VsOutput.PosH = mul(PosL, ComVar_Matrix_LocalToWorld);
	VsOutput.PosH = mul(VsOutput.PosH, ComVar_Matrix_ShadowView);
	VsOutput.PosH = mul(VsOutput.PosH, ComVar_Matrix_ShadowProjection);

	return VsOutput;
}