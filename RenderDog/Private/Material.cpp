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
#include <sstream>
#include <iostream>
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
			m_pShader(nullptr)
		{
			m_ShaderName = m_Name.substr(0, m_Name.rfind(".")) + ".hlsl";
		}

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

		bool							GetParamsFromFile(const std::string& fileName);

	private:
		std::string						m_Name;
		std::string						m_ShaderName;
		std::vector<MaterialParam>		m_Params;

		int								m_MtlInsId;

		IShader*						m_pShader;
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

		virtual IMaterial*				GetMaterial(const std::string& filePath, bool bIsUserMtl) override;
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

	bool Material::GetParamsFromFile(const std::string& fileName)
	{
		std::ifstream fin(fileName);
		if (fin.is_open())
		{
			std::string line;
			while (std::getline(fin, line))
			{
				if (line == "MaterialPropertys")
				{
					std::string mtlProp;
					do
					{
						std::getline(fin, mtlProp);
						if (mtlProp.find("ShaderFile"))
						{
							m_ShaderName = mtlProp.substr(mtlProp.find("\""), mtlProp.rfind("\""));
						}
					} while (mtlProp == "}");
				}
				else if(line == "MaterialParameters")
				{
					std::string mtlParam;
					do
					{
						std::getline(fin, mtlParam);
						if (mtlParam.find("Texture2D"))
						{
							std::string mtlParamName = mtlParam.substr(mtlParam.find("Texture2D"), mtlParam.find("="));
							std::string textureFileName = mtlParam.substr(mtlParam.find("\""), mtlParam.rfind("\""));

							std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
							RenderDog::ITexture2D* pTexture = RenderDog::g_pITextureManager->CreateTexture2D(converter.from_bytes(textureFileName));
							if (!pTexture)
							{
								return false;
							}
							RenderDog::MaterialParam textureParam(mtlParamName, RenderDog::MATERIAL_PARAM_TYPE::TEXTURE2D);
							textureParam.SetTexture2D(pTexture);
							AddParam(textureParam);
						}
					} while (mtlParam == "}");
				}
				else if (line == "MaterialFunctions")
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

	IMaterial* MaterialManager::GetMaterial(const std::string& filePath, bool bIsUserMtl)
	{
		Material* pMaterial = nullptr;

		auto materialIter = m_MaterialMap.find(filePath);
		if (materialIter == m_MaterialMap.end())
		{
			pMaterial = new Material(filePath);
			m_MaterialMap.insert({ filePath, pMaterial });

			/*if (!pMaterial->GetParamsFromFile(filePath))
			{
				return nullptr;
			}*/
			
			if (bIsUserMtl)
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