////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: StaticModelVertexShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

cbuffer GlobleCB : register(b0)
{
	row_major matrix ViewMat;
	row_major matrix ProjMat;
};

cbuffer PerObjCB : register(b1)
{
	row_major matrix WorldMat;
};

struct VSInput
{
	float3 Pos			: POSITION;
	float4 Color		: COLOR;
};

struct VSOutput
{
	float4 Pos			: SV_POSITION;
	float4 Color		: COLOR;
};

VSOutput Main(VSInput vsInput)
{
	VSOutput vsOutput = (VSOutput)0;
	float4 PosL = float4(vsInput.Pos, 1.0f);
	vsOutput.Pos = mul(PosL, WorldMat);
	vsOutput.Pos = mul(vsOutput.Pos, ViewMat);
	vsOutput.Pos = mul(vsOutput.Pos, ProjMat);

	vsOutput.Color = vsInput.Color;

	return vsOutput;
}