////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Common.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

#define MAX_BONE_NUM 256

cbuffer ComVar_ConstantBuffer_Global
{
	row_major matrix	ComVar_Matrix_WorldToView;
	row_major matrix	ComVar_Matrix_ViewToClip;
	float4				ComVar_Vector_WorldEyePosition;
};

cbuffer ComVar_ConstantBuffer_PerObject
{
	row_major matrix	ComVar_Matrix_LocalToWorld;
};

cbuffer ComVar_ConstantBuffer_BoneTransforms
{
	row_major matrix	ComVar_Matrix_BoneTransforms[MAX_BONE_NUM];
};