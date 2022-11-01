////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Material.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

Texture2D		DiffuseTexture;
SamplerState	DiffuseTextureSampler;

Texture2D		NormalTexture;
SamplerState	NormalTextureSampler;

Texture2D		MetallicRoughnessTexture;
SamplerState	MetallicRoughnessTextureSampler;

float3 ComFunc_Material_GetBaseColorRaw(float2 inTexCoord)
{
	float3 BaseColor = abs(DiffuseTexture.Sample(DiffuseTextureSampler, inTexCoord).rgb);
	BaseColor = pow(BaseColor, 2.2f);

	return BaseColor;
}

float3 ComFunc_Material_GetNormalRaw(float2 inTexCoord)
{
	float2 TangentNormalXY = NormalTexture.Sample(NormalTextureSampler, inTexCoord).xy;
	TangentNormalXY = TangentNormalXY * 2.0f - 1.0f;
	float TangentNormalZ = sqrt(1.0f - TangentNormalXY.x * TangentNormalXY.x - TangentNormalXY.y * TangentNormalXY.y);
	float3 TangentNormal = normalize(float3(TangentNormalXY, TangentNormalZ));

	return TangentNormal;
}

float ComFunc_Material_GetMetallicRaw(float2 inTexCoord)
{
	float Metallic = MetallicRoughnessTexture.SampleLevel(MetallicRoughnessTextureSampler, inTexCoord, 0).r;

	return saturate(Metallic);
}

float ComFunc_Material_GetRoughnessRaw(float2 inTexCoord)
{
	float Roughness = MetallicRoughnessTexture.SampleLevel(MetallicRoughnessTextureSampler, inTexCoord, 0).g;

	return saturate(Roughness);
}