////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SingleColorPixelShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

struct VSOutput
{
	float4 PosH   : SV_POSITION;
	float4 Color : COLOR;
};

float4 Main(VSOutput VsOutput) : SV_Target
{
	return VsOutput.Color;
}