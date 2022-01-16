//cbuffer cbPerObject : register(b0)
//{
//	matrix WorldMat;
//	matrix ViewMat;
//	matrix ProjMat;
//};

struct VS_Input
{
	float3 Pos		: POSITION;
	float4 Color	: COLOR;
	float3 Normal	: NORMAL;
	float4 Tangent	: TANGENT;
	float2 Texcoord : TEXCOORD;
};

struct VS_Output
{
	float4 Pos   : SV_POSITION;
	float4 Color : COLOR;
};

VS_Output VS(VS_Input vsInput)
{
	VS_Output vsOutput = (VS_Output)0;
	/*float4 PosL = float4(vs_input.Pos, 1.0f);
	vs_output.Pos = mul(PosL, WorldMat);
	vs_output.Pos = mul(vs_output.Pos, ViewMat);
	vs_output.Pos = mul(vs_output.Pos, ProjMat);*/

	vsOutput.Pos = float4(vsInput.Pos, 1.0f);
	vsOutput.Pos.z = 0.0f;
	vsOutput.Color = vsInput.Color;

	return vsOutput;
}

float4 PS(VS_Output vs_output) : SV_Target
{
	return vs_output.Color;
}