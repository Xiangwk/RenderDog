////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: ShadowDepthSkinModelVertexShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

cbuffer ShadowLightCB : register(b0)
{
	row_major matrix ViewMat;
	row_major matrix ProjMat;
};

cbuffer PerObjCB : register(b1)
{
	row_major matrix WorldMat;
	row_major matrix BoneTransforms[256];
};

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
	//Skinned
	for (int i = 0; i < 4; ++i)
	{
		PosL += (Weights[i] * mul(float4(vsInput.Pos, 1.0f), BoneTransforms[vsInput.BoneIndices[i]])).xyz;
	}

	vsOutput.Pos = mul(float4(PosL, 1.0f), WorldMat);
	vsOutput.Pos = mul(vsOutput.Pos, ViewMat);
	vsOutput.Pos = mul(vsOutput.Pos, ProjMat);

	return vsOutput;
}