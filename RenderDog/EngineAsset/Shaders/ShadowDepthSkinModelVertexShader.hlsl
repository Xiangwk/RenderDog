////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: ShadowDepthSkinModelVertexShader.hlsl
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
	float3 BoneWeights	: WEIGHTS;
	uint4  BoneIndices	: BONEINDICES;
};

struct VSOutput
{
	float4 PosH			: SV_POSITION;
};

VSOutput Main(VSInput VsInput)
{
	VSOutput VsOutput = (VSOutput)0;

	float Weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	Weights[0] = VsInput.BoneWeights.x;
	Weights[1] = VsInput.BoneWeights.y;
	Weights[2] = VsInput.BoneWeights.z;
	Weights[3] = 1.0f - VsInput.BoneWeights.x - VsInput.BoneWeights.y - VsInput.BoneWeights.z;

	float3 PosL = float3(0.0f, 0.0f, 0.0f);
	//Skinned
	for (int i = 0; i < 4; ++i)
	{
		PosL += (Weights[i] * mul(float4(VsInput.PosL, 1.0f), ComVar_Matrix_BoneTransforms[VsInput.BoneIndices[i]])).xyz;
	}

	VsOutput.PosH = mul(float4(PosL, 1.0f), ComVar_Matrix_LocalToWorld);
	VsOutput.PosH = mul(VsOutput.PosH, ComVar_Matrix_ShadowView);
	VsOutput.PosH = mul(VsOutput.PosH, ComVar_Matrix_ShadowProjection);

	return VsOutput;
}