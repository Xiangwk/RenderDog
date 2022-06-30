////////////////////////////////////////
//RenderDog <·,·>
//FileName: SkyVertexShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

cbuffer GlobleCB : register(b0)
{
	row_major matrix	ViewMat;
	row_major matrix	ProjMat;
	float4				EyePosW;
};

cbuffer PerObjCB : register(b1)
{
	row_major matrix WorldMat;
};

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
	vsOutput.Pos = mul(PosL, WorldMat);
	//天空盒相对于相机静止
	vsOutput.Pos.xyz += EyePosW.xyz;
	vsOutput.Pos = mul(vsOutput.Pos, ViewMat);
	vsOutput.Pos = mul(vsOutput.Pos, ProjMat);

	vsOutput.Pos = vsOutput.Pos.xyww;

	vsOutput.PosL = vsInput.Pos;

	return vsOutput;
}