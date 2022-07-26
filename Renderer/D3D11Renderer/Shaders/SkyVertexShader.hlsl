////////////////////////////////////////
//RenderDog <·,·>
//FileName: SkyVertexShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#include "Common.hlsl"

struct VSInput
{
	float3 Pos			: POSITION;
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

VSOutput Main(VSInput vsInput)
{
	VSOutput vsOutput = (VSOutput)0;
	float4 PosL = float4(vsInput.Pos, 1.0f);
	vsOutput.Pos = mul(PosL, ComVar_Matrix_LocalToWorld);
	//天空盒相对于相机静止
	vsOutput.Pos.xyz += ComVar_Vector_WorldEyePosition.xyz;
	vsOutput.Pos = mul(vsOutput.Pos, ComVar_Matrix_WorldToView);
	vsOutput.Pos = mul(vsOutput.Pos, ComVar_Matrix_ViewToClip);

	vsOutput.Pos = vsOutput.Pos.xyww;

	vsOutput.PosL = vsInput.Pos;

	return vsOutput;
}