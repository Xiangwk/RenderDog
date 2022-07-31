////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Common.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

cbuffer ComVar_ConstantBuffer_Global : register(b0)
{
	row_major matrix	ComVar_Matrix_WorldToView;
	row_major matrix	ComVar_Matrix_ViewToClip;
	float4				ComVar_Vector_WorldEyePosition;
};

cbuffer ComVar_ConstantBuffer_PerObject : register(b1)
{
	row_major matrix	ComVar_Matrix_LocalToWorld;
	row_major matrix	ComVar_Matrix_BoneTransforms[256];
};