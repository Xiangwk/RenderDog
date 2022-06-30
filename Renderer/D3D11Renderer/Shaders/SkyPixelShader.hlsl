////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SingleColorPixelShader.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

TextureCube		SkyCubeTexture			: register(t0);
SamplerState	SkyCubeTextureSampler	: register(s0);

struct VSOutput
{
	float4 Pos   : SV_POSITION;
	float3 PosL	 : POSITION;
};

float4 Main(VSOutput vsOutput) : SV_Target
{
	return SkyCubeTexture.Sample(SkyCubeTextureSampler, vsOutput.PosL);
}