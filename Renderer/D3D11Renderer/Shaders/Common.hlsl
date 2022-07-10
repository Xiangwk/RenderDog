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
	row_major matrix BoneTransforms[256];
};