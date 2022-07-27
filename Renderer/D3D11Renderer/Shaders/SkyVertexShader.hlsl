////////////////////////////////////////
//RenderDog <·,·>
//FileName: SkyVertexShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#include "Common.hlsl"

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
	float4 Pos			: SV_POSITION;
	float3 PosL			: POSITION;
};

VSOutput Main(VSInput VsInput)
{
	VSOutput VsOutput = (VSOutput)0;
	float4 PosL = float4(VsInput.PosL, 1.0f);
	VsOutput.Pos = mul(PosL, ComVar_Matrix_LocalToWorld);
	//天空盒相对于相机静止
	VsOutput.Pos.xyz += ComVar_Vector_WorldEyePosition.xyz;
	VsOutput.Pos = mul(VsOutput.Pos, ComVar_Matrix_WorldToView);
	VsOutput.Pos = mul(VsOutput.Pos, ComVar_Matrix_ViewToClip);

	VsOutput.Pos = VsOutput.Pos.xyww;

	VsOutput.PosL = VsInput.PosL;

	return VsOutput;
}