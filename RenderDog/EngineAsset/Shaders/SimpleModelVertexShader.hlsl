////////////////////////////////////////
//RenderDog <��,��>
//FileName: StaticModelVertexShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#include "Common.hlsl"

struct VSInput
{
	float3 PosL			: POSITION;
	float4 Color		: COLOR;
};

struct VSOutput
{
	float4 PosH			: SV_POSITION;
	float4 Color		: COLOR;
};

VSOutput Main(VSInput VsInput)
{
	VSOutput VsOutput = (VSOutput)0;
	float4 PosL = float4(VsInput.PosL, 1.0f);
	VsOutput.PosH = mul(PosL, ComVar_Matrix_LocalToWorld);
	VsOutput.PosH = mul(VsOutput.PosH, ComVar_Matrix_WorldToView);
	VsOutput.PosH = mul(VsOutput.PosH, ComVar_Matrix_ViewToClip);

	VsOutput.Color = VsInput.Color;

	return VsOutput;
}