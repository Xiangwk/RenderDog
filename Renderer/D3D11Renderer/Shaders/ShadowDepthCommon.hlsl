////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: ShadowDepthCommon.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

cbuffer ShadowCB : register(b2)
{
	row_major matrix ComVar_Matrix_ShadowView;
	row_major matrix ComVar_Matrix_ShadowProjection;
}