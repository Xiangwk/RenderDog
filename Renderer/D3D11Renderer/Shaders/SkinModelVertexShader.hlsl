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

	float3 PosL = float3(0.0f, 0.0f, 0.0f);
	float3 normal = float3(0.0f, 0.0f, 0.0f);
	float3 tangent = float3(0.0f, 0.0f, 0.0f);
	//Skinned
	for (int i = 0; i < 4; ++i)
	{
		PosL += (Weights[i] * mul(float4(vsInput.Pos, 1.0f), ComVar_Matrix_BoneTransforms[vsInput.BoneIndices[i]])).xyz;
		normal += (Weights[i] * mul(float4(vsInput.Normal, 0.0f), ComVar_Matrix_BoneTransforms[vsInput.BoneIndices[i]])).xyz;
		tangent += (Weights[i] * mul(float4(vsInput.Tangent.xyz, 0.0f), ComVar_Matrix_BoneTransforms[vsInput.BoneIndices[i]])).xyz;
	}

	vsOutput.Pos = mul(float4(PosL, 1.0f), ComVar_Matrix_LocalToWorld);
	vsOutput.PosW = vsOutput.Pos;

	vsOutput.Pos = mul(vsOutput.Pos, ComVar_Matrix_WorldToView);
	vsOutput.Pos = mul(vsOutput.Pos, ComVar_Matrix_ViewToClip);

	float4 WorldNormal = mul(float4(normal, 0.0f), ComVar_Matrix_LocalToWorld);
	vsOutput.Normal = normalize(normal.xyz);

	float4 WorldTangent = mul(float4(tangent, 1.0f), ComVar_Matrix_LocalToWorld);
	vsOutput.Tangent = normalize(tangent.xyz);

	float3 WorldBiTangent = cross(WorldNormal.xyz, WorldTangent.xyz) * vsInput.Tangent.w;
	vsOutput.BiTangent = normalize(WorldBiTangent);

	vsOutput.Color = vsInput.Color;

	vsOutput.Texcoord = vsInput.Texcoord;

	vsOutput.ShadowPos = vsOutput.PosW;
	vsOutput.ShadowPos = mul(vsOutput.ShadowPos, ShadowViewMat);
	vsOutput.ShadowPos = mul(vsOutput.ShadowPos, ShadowProjMat);

	vsOutput.EyePosW = ComVar_Vector_WorldEyePosition;

	return vsOutput;
}