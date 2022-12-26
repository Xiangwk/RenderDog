////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Material.hlsl
//Written by Xiang Weikang
////////////////////////////////////////

Texture2D		LocVar_Material_DiffuseTexture;
SamplerState	LocVar_Material_DiffuseTextureSampler;

Texture2D		LocVar_Material_NormalTexture;
SamplerState	LocVar_Material_NormalTextureSampler;

Texture2D		LocVar_Material_MetallicRoughnessTexture;
SamplerState	LocVar_Material_MetallicRoughnessTextureSampler;

float3 ComFunc_Material_GetBaseColorRaw(float2 inTexCoord)
{
	float3 BaseColor = abs(LocVar_Material_DiffuseTexture.Sample(LocVar_Material_DiffuseTextureSampler, inTexCoord).rgb);
	BaseColor = pow(BaseColor, 2.2f);

	return BaseColor;
}

float3 ComFunc_Material_GetNormalRaw(float2 inTexCoord)
{
	float2 TangentNormalXY = LocVar_Material_NormalTexture.Sample(LocVar_Material_NormalTextureSampler, inTexCoord).xy;
	TangentNormalXY = TangentNormalXY * 2.0f - 1.0f;
	float TangentNormalZ = sqrt(1.0f - TangentNormalXY.x * TangentNormalXY.x - TangentNormalXY.y * TangentNormalXY.y);
	float3 TangentNormal = normalize(float3(TangentNormalXY, TangentNormalZ));

	return TangentNormal;
}

float ComFunc_Material_GetMetallicRaw(float2 inTexCoord)
{
	float Metallic = LocVar_Material_MetallicRoughnessTexture.SampleLevel(LocVar_Material_MetallicRoughnessTextureSampler, inTexCoord, 0).r;

	return saturate(Metallic);
}

float ComFunc_Material_GetRoughnessRaw(float2 inTexCoord)
{
	float Roughness = LocVar_Material_MetallicRoughnessTexture.SampleLevel(LocVar_Material_MetallicRoughnessTextureSampler, inTexCoord, 0).g;

	return saturate(Roughness);
}