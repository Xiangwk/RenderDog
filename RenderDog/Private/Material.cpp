////////////////////////////////////////
// RenderDog <¡¤,¡¤>
// FileName: Material.cpp
// Written by Xiang Weikang
////////////////////////////////////////

#include "Material.h"
#include "RefCntObject.h"
#include "Shader.h"
#include "Texture.h"
#include "Buffer.h"

#include <vector>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <locale>
#include <codecvt>

namespace RenderDog
{
	MaterialParam INVALID_MATERIAL_PARAM = MaterialParam("InvalidParam", MATERIAL_PARAM_TYPE::UNKNOWN);

	MaterialParam::MaterialParam(const MaterialParam& param):
		m_Name(param.m_Name),
		m_ParamType(param.m_ParamType)
	{
		CloneParamValue(param);
	}

	MaterialParam& MaterialParam::operator=(const MaterialParam& param)
	{
		m_Name = param.m_Name;
		m_ParamType = param.m_ParamType;

		CloneParamValue(param);

		return *this;
	}

	void MaterialParam::CloneParamValue(const MaterialParam& param)
	{
		switch (param.m_ParamType)
		{
		case MATERIAL_PARAM_TYPE::VECTOR4:
		{
			m_Vector4Value = param.m_Vector4Value;
			break;
		}
		case MATERIAL_PARAM_TYPE::TEXTURE2D:
		{
			m_pTexture2DValue = param.m_pTexture2DValue;
			break;
		}
		case MATERIAL_PARAM_TYPE::SAMPLER:
		{
			pSamplerValue = param.pSamplerValue;
			break;
		}
		default:
			break;
		}
	}

	//================================================================
	//						Material
	//================================================================
	class MaterialInstance;

	class Material : public IMaterial, public RefCntObject
	{
		friend class MaterialInstance;

	public:
		Material(const std::string& name) :
			RefCntObject(),
			m_Name(name),
			m_ShaderName(),
			m_Params(0),
			m_MtlInsId(0),
			m_pShader(nullptr),
			m_bIsUserMtl(false)
		{}

		virtual ~Material()
		{
			for (size_t i = 0; i < m_Params.size(); ++i)
			{
				MaterialParam& param = m_Params[i];
				if (param.GetType() == MATERIAL_PARAM_TYPE::TEXTURE2D)
				{
					ITexture2D* pTexture = param.GetTexture2D();
					pTexture->Release();
				}
				if (param.GetType() == MATERIAL_PARAM_TYPE::SAMPLER)
				{
					ISamplerState* pSampler = param.GetSamplerState();
					pSampler->Release();
				}
			}

			if (m_pShader)
			{
				m_pShader->Release();
				m_pShader = nullptr;
			}
		}

		virtual void					Release() override;
		virtual const std::string&		GetName() const override { return m_Name; }

		virtual void					AddParam(const MaterialParam& param) override;

		virtual MaterialParam&			GetParamByName(const std::string& name) override;
		virtual MaterialParam&			GetParamByIndex(uint32_t index) override;
		virtual uint32_t				GetParamNum() const override;

		virtual bool					CreateMaterialShader() override;
		virtual IShader*				GetMaterialShader() override { return m_pShader; }

		bool							GetPropsAndParamsFromFile(const std::string& fileName);

		bool							IsUserMaterial() const { return m_bIsUserMtl; }

	private:
		std::string						m_Name;
		std::string						m_ShaderName;
		std::vector<MaterialParam>		m_Params;

		int								m_MtlInsId;

		IShader*						m_pShader;

		bool							m_bIsUserMtl;
	};

	//================================================================
	//					MaterialInstance
	//================================================================
	class MaterialInstance : public IMaterialInstance, public RefCntObject
	{
	public:
		MaterialInstance() :
			m_Name(""),
			m_Params(0),
			m_pMaterial(nullptr),
			m_pMtlParamsConstantBuffer(nullptr)
		{}

		MaterialInstance(IMaterial* pMtl);
		MaterialInstance(IMaterial* pMtl, const std::vector<MaterialParam>* pMtlParams);

		virtual ~MaterialInstance();

		virtual void					Release() override;

		virtual const std::string&		GetName() const override { return m_Name; }

		virtual IMaterial*				GetMaterial() const { return m_pMaterial; }

		virtual MaterialParam&			GetMaterialParamByIndex(uint32_t index) { return m_Params[index]; }
		virtual uint32_t				GetMaterialParamNum() const { return (uint32_t)(m_Params.size()); }

	private:
		std::string						m_Name;
		std::vector<MaterialParam>		m_Params;
		IMaterial*						m_pMaterial;

		IConstantBuffer*				m_pMtlParamsConstantBuffer;
	};

	//================================================================
	//					MaterialManager
	//================================================================
	class MaterialManager : public IMaterialManager
	{
		typedef std::unordered_map<std::string, IMaterial*>				MaterialMap;
		typedef std::unordered_map<std::string, IMaterialInstance*>		MaterialInsMap;

	public:
		MaterialManager() = default;
		virtual ~MaterialManager() = default;

		virtual IMaterial*				GetMaterial(const std::string& filePath) override;
		virtual IMaterialInstance*		GetMaterialInstance(IMaterial* pMaterial, const std::vector<MaterialParam>* pMtlParams = nullptr) override;

		virtual bool					LoadMaterialInstanceMap(const std::string& fileName, std::vector<std::string>& outMtlFiles) override;
		virtual bool					LoadMaterialInstance(const std::string& fileName, std::string& outMtlName, std::vector<MaterialParam>& outMtlParams) override;

		void							ReleaseMaterial(Material* pMaterial);
		void							ReleaseMaterialInstance(MaterialInstance* pMaterialIns);

	private:
		MaterialMap						m_MaterialMap;
		MaterialInsMap					m_MaterialInsMap;
	};

	MaterialManager		g_MaterialManager;
	IMaterialManager*	g_pMaterialManager = &g_MaterialManager;


	//================================================================
	//				Function Implementation
	//================================================================
	void Material::Release()
	{
		g_MaterialManager.ReleaseMaterial(this);
	}

	MaterialParam& Material::GetParamByName(const std::string& name)
	{
		for (size_t i = 0; i < m_Params.size(); ++i)
		{
			MaterialParam& param = m_Params[i];
			if (param.GetName() == name)
			{
				return param;
			}
		}

		return INVALID_MATERIAL_PARAM;
	}

	MaterialParam& Material::GetParamByIndex(uint32_t index)
	{
		if (index < m_Params.size())
		{
			return m_Params[index];
		}
		else
		{
			return INVALID_MATERIAL_PARAM;
		}
	}

	uint32_t Material::GetParamNum() const
	{
		return (uint32_t)(m_Params.size());
	}

	bool Material::CreateMaterialShader()
	{
		ShaderCompileDesc desc = ShaderCompileDesc(g_DirectionalLightingPixelShaderFilePath, nullptr, "Main", "ps_5_0", 0);
		desc.mtlShaderName = m_ShaderName;
		m_pShader = g_pIShaderManager->GetMaterialShader(desc, m_Name);
		if (!m_pShader)
		{
			return false;
		}

		return true;
	}

	void Material::AddParam(const MaterialParam& param)
	{
		if (param.GetType() != MATERIAL_PARAM_TYPE::UNKNOWN)
		{
			m_Params.push_back(param);
		}
	}

	bool Material::GetPropsAndParamsFromFile(const std::string& fileName)
	{
		std::ifstream fin(fileName);
		if (fin.is_open())
		{
			std::string line;
			while (std::getline(fin, line))
			{
				if (line == MTL_PROPS)
				{
					std::string mtlProp;
					while (mtlProp != "}")
					{
						size_t strStart = 0;
						size_t strEnd = 0;
						std::getline(fin, mtlProp);
						if (mtlProp.find(MTL_PROPS_USER_MTL) != std::string::npos)
						{
							strStart = mtlProp.find("=") + 1;
							strEnd = mtlProp.size();
							std::string isUserMtl = mtlProp.substr(strStart, strEnd - strStart);
							isUserMtl.erase(std::remove(isUserMtl.begin(), isUserMtl.end(), ' '), isUserMtl.end());
							m_bIsUserMtl = std::stoi(isUserMtl) == 1 ? true : false;
						}
						else if (mtlProp.find(MTL_PROPS_SHADER_FILE) != std::string::npos)
						{
							strStart = mtlProp.find("\"") + 1;
							strEnd = mtlProp.rfind("\"");
							m_ShaderName = mtlProp.substr(strStart, strEnd - strStart);
							m_ShaderName.erase(std::remove(m_ShaderName.begin(), m_ShaderName.end(), ' '), m_ShaderName.end());
						}
					} 
				}
				else if(line == MTL_PARAMS)
				{
					std::string mtlParam;
					while (mtlParam != "}")
					{
						size_t strStart = 0;
						size_t strEnd = 0;
						std::getline(fin, mtlParam);
						if (mtlParam.find(MTL_PROPS_FLOAT4) != std::string::npos)
						{
							strStart = mtlParam.find(MTL_PROPS_FLOAT4) + MTL_PROPS_FLOAT4.size();
							strEnd = mtlParam.find("=");
							std::string mtlParamName = mtlParam.substr(strStart, strEnd - strStart);
							mtlParamName.erase(std::remove(mtlParamName.begin(), mtlParamName.end(), ' '), mtlParamName.end());

							strStart = mtlParam.find("(") + 1;
							strEnd = mtlParam.rfind(")");
							std::string mtlParamValue = mtlParam.substr(strStart, strEnd - strStart);
							mtlParamValue.erase(std::remove(mtlParamValue.begin(), mtlParamValue.end(), ' '), mtlParamValue.end());

							Vector4 vec4Value;
							strStart = 0;
							strEnd = 0;
							float tempVec4[4] = {};
							int i = 0;
							while (strEnd < mtlParamValue.size())
							{
								while (mtlParamValue[strEnd] != ',' && strEnd < mtlParamValue.size())
								{
									strEnd++;
								}
								std::string value = mtlParamValue.substr(strStart, strEnd - strStart);
								tempVec4[i++] = std::stof(value);

								strStart = strEnd + 1;
								strEnd++;
							}
							
							vec4Value.x = tempVec4[0];
							vec4Value.y = tempVec4[1];
							vec4Value.z = tempVec4[2];
							vec4Value.w = tempVec4[3];

							MaterialParam vec4Param(mtlParamName, MATERIAL_PARAM_TYPE::VECTOR4);
							vec4Param.SetVector4(vec4Value);
							AddParam(vec4Param);
						}
						else if (mtlParam.find(MTL_PARAMS_TEXTURE2D) != std::string::npos)
						{
							strStart = mtlParam.find(MTL_PARAMS_TEXTURE2D) + MTL_PARAMS_TEXTURE2D.size();
							strEnd = mtlParam.find("=");
							std::string mtlParamName = mtlParam.substr(strStart, strEnd - strStart);
							mtlParamName.erase(std::remove(mtlParamName.begin(), mtlParamName.end(), ' '), mtlParamName.end());

							strStart = mtlParam.find("\"") + 1;
							strEnd = mtlParam.rfind("\"");
							std::string textureFileName = mtlParam.substr(strStart, strEnd - strStart);
							textureFileName.erase(std::remove(textureFileName.begin(), textureFileName.end(), ' '), textureFileName.end());

							std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
							ITexture2D* pTexture = g_pITextureManager->GetTexture2D(converter.from_bytes(textureFileName));
							if (!pTexture)
							{
								return false;
							}
							MaterialParam textureParam(mtlParamName, MATERIAL_PARAM_TYPE::TEXTURE2D);
							textureParam.SetTexture2D(pTexture);
							AddParam(textureParam);

							SamplerDesc samplerDesc = {};
							samplerDesc.name = mtlParamName + "Sampler";
							samplerDesc.filterMode = SAMPLER_FILTER::LINEAR;
							samplerDesc.addressMode = SAMPLER_ADDRESS::WRAP;
							ISamplerState* pTextureSampler = g_pISamplerStateManager->GetSamplerState(samplerDesc);
							if (!pTextureSampler)
							{
								return false;
							}
							MaterialParam textureSamplerParam(samplerDesc.name, MATERIAL_PARAM_TYPE::SAMPLER);
							textureSamplerParam.SetSamplerState(pTextureSampler);
							AddParam(textureSamplerParam);
						}
					}
				}
				else if (line == MTL_FUNCTIONS)
				{

				}
			}
		}
		else
		{
			return false;
		}

		return true;
	}


	MaterialInstance::MaterialInstance(IMaterial* pMtl) :
		m_Name(""),
		m_Params(0),
		m_pMaterial(pMtl)
	{
		Material* pRealMtl = (Material*)pMtl;
		int mtlInsId = pRealMtl->m_MtlInsId;
		pRealMtl->m_MtlInsId++;

		std::string mtlInsName = pMtl->GetName() + "_" + std::to_string(mtlInsId);
		m_Name = mtlInsName;

		uint32_t vec4ParamNum = 0;
		for (uint32_t i = 0; i < pMtl->GetParamNum(); ++i)
		{
			MaterialParam& mtlParam = pMtl->GetParamByIndex(i);

			switch (mtlParam.GetType())
			{
			case MATERIAL_PARAM_TYPE::VECTOR4:
			{
				m_Params.push_back(mtlParam);
				vec4ParamNum++;

				break;
			}
			case MATERIAL_PARAM_TYPE::TEXTURE2D:
			{
				ITexture2D* pTexture = mtlParam.GetTexture2D();
				const std::wstring& textureFileName = pTexture->GetName();

				ITexture2D* pNewTexture = g_pITextureManager->GetTexture2D(textureFileName);
				MaterialParam newTextureParam(mtlParam.GetName(), MATERIAL_PARAM_TYPE::TEXTURE2D);
				newTextureParam.SetTexture2D(pNewTexture);

				m_Params.push_back(newTextureParam);

				break;
			}
			case MATERIAL_PARAM_TYPE::SAMPLER:
			{
				ISamplerState* pSampler = mtlParam.GetSamplerState();
				SamplerDesc desc = pSampler->GetDesc();

				ISamplerState* pNewSampler = g_pISamplerStateManager->GetSamplerState(desc);
				MaterialParam newSamplerParam(mtlParam.GetName(), MATERIAL_PARAM_TYPE::SAMPLER);
				newSamplerParam.SetSamplerState(pNewSampler);

				m_Params.push_back(newSamplerParam);

				break;
			}
			default:
				break;
			}
		}

		BufferDesc cbDesc = {};
		cbDesc.name = "$Globals";
		cbDesc.byteWidth = sizeof(Vector4) * vec4ParamNum;
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = true;
		m_pMtlParamsConstantBuffer = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);
	}

	MaterialInstance::MaterialInstance(IMaterial* pMtl, const std::vector<MaterialParam>* pMtlParams):
		m_Name(""),
		m_Params(pMtlParams->size()),
		m_pMaterial(pMtl)
	{
		Material* pRealMtl = (Material*)pMtl;
		int mtlInsId = pRealMtl->m_MtlInsId;
		pRealMtl->m_MtlInsId++;

		std::string mtlInsName = pMtl->GetName() + "_" + std::to_string(mtlInsId);
		m_Name = mtlInsName;

		uint32_t vec4ParamNum = 0;
		for (uint32_t i = 0; i < pMtlParams->size(); ++i)
		{
			if (m_Params[i].GetType() == MATERIAL_PARAM_TYPE::VECTOR4)
			{
				vec4ParamNum++;
			}

			m_Params.push_back((*pMtlParams)[i]);
		}

		BufferDesc cbDesc = {};
		cbDesc.name = "$Globals";
		cbDesc.byteWidth = sizeof(Vector4) * vec4ParamNum;
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = true;
		m_pMtlParamsConstantBuffer = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);
	}

	MaterialInstance::~MaterialInstance()
	{
		for (size_t i = 0; i < m_Params.size(); ++i)
		{
			MaterialParam& param = m_Params[i];
			if (param.GetType() == MATERIAL_PARAM_TYPE::TEXTURE2D)
			{
				ITexture2D* pTexture = param.GetTexture2D();
				pTexture->Release();
			}
			if (param.GetType() == MATERIAL_PARAM_TYPE::SAMPLER)
			{
				ISamplerState* pSampler = param.GetSamplerState();
				pSampler->Release();
			}
		}

		if (m_pMtlParamsConstantBuffer)
		{
			m_pMtlParamsConstantBuffer->Release();
			m_pMtlParamsConstantBuffer = nullptr;
		}
	}

	void MaterialInstance::Release()
	{
		g_MaterialManager.ReleaseMaterialInstance(this);
	}

	IMaterial* MaterialManager::GetMaterial(const std::string& filePath)
	{
		Material* pMaterial = nullptr;

		auto materialIter = m_MaterialMap.find(filePath);
		if (materialIter == m_MaterialMap.end())
		{
			pMaterial = new Material(filePath);
			m_MaterialMap.insert({ filePath, pMaterial });

			if (!pMaterial->GetPropsAndParamsFromFile(filePath))
			{
				return nullptr;
			}
			
			if (pMaterial->IsUserMaterial())
			{
				pMaterial->CreateMaterialShader();
			}
		}
		else
		{
			pMaterial = (Material*)materialIter->second;
			pMaterial->AddRef();
		}

		return pMaterial;
	}

	IMaterialInstance* MaterialManager::GetMaterialInstance(IMaterial* pMaterial, const std::vector<MaterialParam>* pMtlParams /*= nullptr*/)
	{
		MaterialInstance* pMtlIns = nullptr;
		if (pMtlParams)
		{
			pMtlIns = new MaterialInstance(pMaterial, pMtlParams);
		}
		else
		{
			pMtlIns = new MaterialInstance(pMaterial);
		}

		m_MaterialInsMap.insert({ pMtlIns->GetName(), pMtlIns });

		return pMtlIns;
	}

	bool MaterialManager::LoadMaterialInstanceMap(const std::string& fileName, std::vector<std::string>& outMtlFiles)
	{
		std::ifstream fin(fileName);

		if (fin.is_open())
		{
			std::string line;
			size_t strStart = 0;
			size_t strEnd = 0;
			while (std::getline(fin, line))
			{
				std::string meshName;
				strStart = 0;
				strEnd = line.find("=");
				meshName = line.substr(strStart, strEnd - strStart);
				meshName.erase(std::remove(meshName.begin(), meshName.end(), ' '), meshName.end());

				std::string mtlinsName;
				strStart = strEnd + 1;
				strEnd = line.size();
				mtlinsName = line.substr(strStart, strEnd - strStart);
				mtlinsName.erase(std::remove(mtlinsName.begin(), mtlinsName.end(), ' '), mtlinsName.end());

				outMtlFiles.push_back(mtlinsName);
			}
		}
		else
		{
			return false;
		}

		fin.close();

		return true;
	}

	bool MaterialManager::LoadMaterialInstance(const std::string& fileName, std::string& outMtlName, std::vector<MaterialParam>& outMtlParams)
	{
		std::ifstream fin(fileName);
		if (!fin.is_open())
		{
			return false;
		}
		
		std::string line;
		std::getline(fin, line);

		size_t strStart = line.find("=") + 1;
		size_t strEnd = line.size();
		outMtlName = line.substr(strStart, strEnd - strStart);
		outMtlName.erase(std::remove(outMtlName.begin(), outMtlName.end(), ' '), outMtlName.end());
		outMtlName = MTL_USER_FILE_DIR + outMtlName;

		while (std::getline(fin, line))
		{
			if (line.find(MTL_PROPS_FLOAT4) != std::string::npos)
			{
				strStart = line.find(MTL_PROPS_FLOAT4) + MTL_PROPS_FLOAT4.size();
				strEnd = line.find("=");
				std::string mtlParamName = line.substr(strStart, strEnd - strStart);
				mtlParamName.erase(std::remove(mtlParamName.begin(), mtlParamName.end(), ' '), mtlParamName.end());

				strStart = line.find("(") + 1;
				strEnd = line.rfind(")");
				std::string mtlParamValue = line.substr(strStart, strEnd - strStart);
				mtlParamValue.erase(std::remove(mtlParamValue.begin(), mtlParamValue.end(), ' '), mtlParamValue.end());

				Vector4 vec4Value;
				strStart = 0;
				strEnd = 0;
				float tempVec4[4] = {};
				int i = 0;
				while (strEnd < mtlParamValue.size())
				{
					while (mtlParamValue[strEnd] != ',' && strEnd < mtlParamValue.size())
					{
						strEnd++;
					}
					std::string value = mtlParamValue.substr(strStart, strEnd - strStart);
					tempVec4[i++] = std::stof(value);

					strStart = strEnd + 1;
					strEnd++;
				}

				vec4Value.x = tempVec4[0];
				vec4Value.y = tempVec4[1];
				vec4Value.z = tempVec4[2];
				vec4Value.w = tempVec4[3];

				MaterialParam vec4Param(mtlParamName, MATERIAL_PARAM_TYPE::VECTOR4);
				vec4Param.SetVector4(vec4Value);

				outMtlParams.push_back(vec4Param);
			}
			else if (line.find(MTL_PARAMS_TEXTURE2D) != std::string::npos)
			{
				strStart = line.find(MTL_PARAMS_TEXTURE2D) + MTL_PARAMS_TEXTURE2D.size();
				strEnd = line.find("=");
				std::string mtlParamName = line.substr(strStart, strEnd - strStart);
				mtlParamName.erase(std::remove(mtlParamName.begin(), mtlParamName.end(), ' '), mtlParamName.end());

				strStart = line.find("\"") + 1;
				strEnd = line.rfind("\"");
				std::string textureFileName = line.substr(strStart, strEnd - strStart);
				textureFileName.erase(std::remove(textureFileName.begin(), textureFileName.end(), ' '), textureFileName.end());

				std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
				RenderDog::ITexture2D* pTexture = RenderDog::g_pITextureManager->GetTexture2D(converter.from_bytes(textureFileName));
				if (!pTexture)
				{
					return false;
				}
				RenderDog::MaterialParam textureParam(mtlParamName, RenderDog::MATERIAL_PARAM_TYPE::TEXTURE2D);
				textureParam.SetTexture2D(pTexture);
				outMtlParams.push_back(textureParam);

				RenderDog::SamplerDesc samplerDesc = {};
				samplerDesc.name = mtlParamName + "Sampler";
				samplerDesc.filterMode = RenderDog::SAMPLER_FILTER::LINEAR;
				samplerDesc.addressMode = RenderDog::SAMPLER_ADDRESS::WRAP;
				RenderDog::ISamplerState* pTextureSampler = RenderDog::g_pISamplerStateManager->GetSamplerState(samplerDesc);
				if (!pTextureSampler)
				{
					return false;
				}
				RenderDog::MaterialParam textureSamplerParam(samplerDesc.name, RenderDog::MATERIAL_PARAM_TYPE::SAMPLER);
				textureSamplerParam.SetSamplerState(pTextureSampler);
				outMtlParams.push_back(textureSamplerParam);
			}
		}
		
		fin.close();

		return true;
	}

	void MaterialManager::ReleaseMaterial(Material* pMaterial)
	{
		std::string matName = pMaterial->GetName();
		if (pMaterial->SubRef() == 0)
		{
			m_MaterialMap.erase(matName);
		}
	}

	void MaterialManager::ReleaseMaterialInstance(MaterialInstance* pMaterialIns)
	{
		std::string matName = pMaterialIns->GetName();
		if (pMaterialIns->SubRef() == 0)
		{
			m_MaterialInsMap.erase(matName);
		}
	}

}// namespace RenderDog