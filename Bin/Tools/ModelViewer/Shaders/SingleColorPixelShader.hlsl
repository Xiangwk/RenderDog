////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SingleColorPixelShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

struct VSOutput
{
	float4 Pos   : SV_POSITION;
	float4 Color : COLOR;
};

float4 Main(VSOutput vsOutput) : SV_Target
{
	return vsOutput.Color;
}