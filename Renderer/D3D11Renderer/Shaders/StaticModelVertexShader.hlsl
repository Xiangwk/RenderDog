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
	float3 Pos		: POSITION;
	float4 Color	: COLOR;
	float3 Normal	: NORMAL;
	float4 Tangent	: TANGENT;
	float2 Texcoord : TEXCOORD;
};

struct VSOutput
{
	float4 Pos		: SV_POSITION;
	float4 Color	: COLOR;
	float3 Normal	: NORMAL;
	float2 Texcoord : TEXCOORD0;
};

VSOutput Main(VSInput vsInput)
{
	VSOutput vsOutput = (VSOutput)0;
	float4 PosL = float4(vsInput.Pos, 1.0f);
	vsOutput.Pos = mul(PosL, WorldMat);
	vsOutput.Pos = mul(vsOutput.Pos, ViewMat);
	vsOutput.Pos = mul(vsOutput.Pos, ProjMat);

	float4 normal = float4(vsInput.Normal, 0.0f);
	normal = mul(normal, WorldMat);
	vsOutput.Normal = normalize(normal.xyz);

	vsOutput.Color = vsInput.Color;

	vsOutput.Texcoord = vsInput.Texcoord;

	return vsOutput;
}