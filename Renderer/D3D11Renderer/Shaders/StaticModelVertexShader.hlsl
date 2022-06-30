////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: StaticModelVertexShader.hlsl
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

cbuffer ShadowCB : register(b2)
{
	row_major matrix ShadowViewMat;
	row_major matrix ShadowProjMat;
}

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
	float4 Color		: COLOR;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 BiTangent	: BITANGENT;
	float2 Texcoord		: TEXCOORD0;
	float4 ShadowPos	: TEXCOORD1;
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

	float4 tangent = float4(vsInput.Tangent.xyz, 0.0f);
	tangent = mul(tangent, WorldMat);
	vsOutput.Tangent = normalize(tangent.xyz);

	float3 biTangent = normalize(cross(normal.xyz, tangent.xyz) * vsInput.Tangent.w);
	vsOutput.BiTangent = biTangent;

	vsOutput.Color = vsInput.Color;

	vsOutput.Texcoord = vsInput.Texcoord;

	vsOutput.ShadowPos = mul(PosL, WorldMat);
	vsOutput.ShadowPos = mul(vsOutput.ShadowPos, ShadowViewMat);
	vsOutput.ShadowPos = mul(vsOutput.ShadowPos, ShadowProjMat);

	return vsOutput;
}