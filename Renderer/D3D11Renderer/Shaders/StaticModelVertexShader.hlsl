////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: StaticModelVertexShader.hlsl
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
	float4 Pos			: SV_POSITION;
	float4 PosW			: POSITION;
	float4 Color		: COLOR;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 BiTangent	: BITANGENT;
	float2 Texcoord		: TEXCOORD0;
	float4 ShadowPos	: TEXCOORD1;
	float4 EyePosW		: TEXCOORD2;
};

VSOutput Main(VSInput VsInput)
{
	VSOutput VsOutput = (VSOutput)0;
	float4 PosL = float4(VsInput.PosL, 1.0f);
	VsOutput.Pos = mul(PosL, ComVar_Matrix_LocalToWorld);
	VsOutput.PosW = VsOutput.Pos;

	VsOutput.Pos = mul(VsOutput.Pos, ComVar_Matrix_WorldToView);
	VsOutput.Pos = mul(VsOutput.Pos, ComVar_Matrix_ViewToClip);

	float4 normal = float4(VsInput.Normal, 0.0f);
	normal = mul(normal, ComVar_Matrix_LocalToWorld);
	VsOutput.Normal = normalize(normal.xyz);

	float4 tangent = float4(VsInput.Tangent.xyz, 0.0f);
	tangent = mul(tangent, ComVar_Matrix_LocalToWorld);
	VsOutput.Tangent = normalize(tangent.xyz);

	float3 biTangent = normalize(cross(normal.xyz, tangent.xyz) * VsInput.Tangent.w);
	VsOutput.BiTangent = biTangent;

	VsOutput.Color = VsInput.Color;

	VsOutput.Texcoord = VsInput.Texcoord;

	VsOutput.ShadowPos = VsOutput.PosW;
	VsOutput.ShadowPos = mul(VsOutput.ShadowPos, ComVar_Matrix_ShadowView);
	VsOutput.ShadowPos = mul(VsOutput.ShadowPos, ComVar_Matrix_ShadowProjection);

	VsOutput.EyePosW = ComVar_Vector_WorldEyePosition;

	return VsOutput;
}