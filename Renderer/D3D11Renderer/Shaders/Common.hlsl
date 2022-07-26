////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: StaticModelVertexShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

cbuffer GlobleCB : register(b0)
{
	row_major matrix	ComVar_Matrix_WorldToView;
	row_major matrix	ComVar_Matrix_ViewToClip;
	float4				ComVar_Vector_WorldEyePosition;
};

cbuffer PerObjCB : register(b1)
{
	row_major matrix ComVar_Matrix_LocalToWorld;
	row_major matrix ComVar_Matrix_BoneTransforms[256];
};