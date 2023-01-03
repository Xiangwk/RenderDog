////////////////////////////////////////
// RenderDog <¡¤,¡¤>
// FileName: Material.cpp
// Written by Xiang Weikang
////////////////////////////////////////

#include "Material.h"
#include "RefCntObject.h"
#include "Shader.h"
#include "Texture.h"

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
		case MATERIAL_PARAM_TYPE::SCALAR:
		{
			m_FloatValue = param.m_FloatValue;
			break;
		}
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
			m_pMaterial(nullptr)
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
						if (mtlParam.find(MTL_PARAMS_TEXTURE2D) != std::string::npos)
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
							RenderDog::ITexture2D* pTexture = RenderDog::g_pITextureManager->CreateTexture2D(converter.from_bytes(textureFileName));
							if (!pTexture)
							{
								return false;
							}
							RenderDog::MaterialParam textureParam(mtlParamName, RenderDog::MATERIAL_PARAM_TYPE::TEXTURE2D);
							textureParam.SetTexture2D(pTexture);
							AddParam(textureParam);

							RenderDog::SamplerDesc samplerDesc = {};
							samplerDesc.name = mtlParamName + "Sampler";
							samplerDesc.filterMode = RenderDog::SAMPLER_FILTER::LINEAR;
							samplerDesc.addressMode = RenderDog::SAMPLER_ADDRESS::WRAP;
							RenderDog::ISamplerState* pTextureSampler = RenderDog::g_pISamplerStateManager->CreateSamplerState(samplerDesc);
							if (!pTextureSampler)
							{
								return false;
							}
							RenderDog::MaterialParam textureSamplerParam(samplerDesc.name, RenderDog::MATERIAL_PARAM_TYPE::SAMPLER);
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

		for (uint32_t i = 0; i < pMtl->GetParamNum(); ++i)
		{
			MaterialParam& mtlParam = pMtl->GetParamByIndex(i);

			switch (mtlParam.GetType())
			{
			case MATERIAL_PARAM_TYPE::TEXTURE2D:
			{
				ITexture2D* pTexture = mtlParam.GetTexture2D();
				const std::wstring& textureFileName = pTexture->GetName();

				ITexture2D* pNewTexture = RenderDog::g_pITextureManager->CreateTexture2D(textureFileName);
				RenderDog::MaterialParam newTextureParam(mtlParam.GetName(), RenderDog::MATERIAL_PARAM_TYPE::TEXTURE2D);
				newTextureParam.SetTexture2D(pNewTexture);

				m_Params.push_back(newTextureParam);

				break;
			}
			case MATERIAL_PARAM_TYPE::SAMPLER:
			{
				ISamplerState* pSampler = mtlParam.GetSamplerState();
				SamplerDesc desc = pSampler->GetDesc();

				RenderDog::ISamplerState* pNewSampler = RenderDog::g_pISamplerStateManager->CreateSamplerState(desc);
				RenderDog::MaterialParam newSamplerParam(mtlParam.GetName(), RenderDog::MATERIAL_PARAM_TYPE::SAMPLER);
				newSamplerParam.SetSamplerState(pNewSampler);

				m_Params.push_back(newSamplerParam);

				break;
			}
			default:
				break;
			}
		}
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

		for (uint32_t i = 0; i < pMtlParams->size(); ++i)
		{
			m_Params.push_back((*pMtlParams)[i]);
		}
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