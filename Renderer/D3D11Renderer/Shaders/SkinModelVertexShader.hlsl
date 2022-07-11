////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: StaticModelVertexShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#include "Common.hlsl"

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
	float3 BoneWeights	: WEIGHTS;
	uint4  BoneIndices	: BONEINDICES;
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

VSOutput Main(VSInput vsInput)
{
	VSOutput vsOutput = (VSOutput)0;

	float Weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	Weights[0] = vsInput.BoneWeights.x;
	Weights[1] = vsInput.BoneWeights.y;
	Weights[2] = vsInput.BoneWeights.z;
	Weights[3] = 1.0f - vsInput.BoneWeights.x - vsInput.BoneWeights.y - vsInput.BoneWeights.z;

	float4 PosL = float4(vsInput.Pos, 1.0f);
	float4 normal = float4(vsInput.Normal, 0.0f);
	float4 tangent = float4(vsInput.Tangent.xyz, 0.0f);
	//Skinned
	for (int i = 0; i < 4; ++i)
	{
		PosL += Weights[i] * mul(PosL, BoneTransforms[vsInput.BoneIndices[i]]);
		normal += Weights[i] * mul(normal, BoneTransforms[vsInput.BoneIndices[i]]);
		tangent += Weights[i] * mul(tangent, BoneTransforms[vsInput.BoneIndices[i]]);
	}

	vsOutput.Pos = mul(PosL, WorldMat);
	vsOutput.PosW = vsOutput.Pos;

	vsOutput.Pos = mul(vsOutput.Pos, ViewMat);
	vsOutput.Pos = mul(vsOutput.Pos, ProjMat);

	normal = mul(normal, WorldMat);
	vsOutput.Normal = normalize(normal.xyz);

	tangent = mul(tangent, WorldMat);
	vsOutput.Tangent = normalize(tangent.xyz);

	float3 biTangent = normalize(cross(normal.xyz, tangent.xyz) * vsInput.Tangent.w);
	vsOutput.BiTangent = biTangent;

	vsOutput.Color = vsInput.Color;

	vsOutput.Texcoord = vsInput.Texcoord;

	vsOutput.ShadowPos = vsOutput.PosW;
	vsOutput.ShadowPos = mul(vsOutput.ShadowPos, ShadowViewMat);
	vsOutput.ShadowPos = mul(vsOutput.ShadowPos, ShadowProjMat);

	vsOutput.EyePosW = EyePosW;

	return vsOutput;
}