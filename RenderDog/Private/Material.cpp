////////////////////////////////////////
// RenderDog <¡¤,¡¤>
// FileName: Material.cpp
// Written by Xiang Weikang
////////////////////////////////////////

#include "Material.h"
#include "RefCntObject.h"

#include <vector>
#include <unordered_map>

namespace RenderDog
{
	//================================================================
	//						Material
	//================================================================
	class MaterialInstance;

	class Material : public IMaterial, public RefCntObject
	{
		friend class MaterialInstance;

	public:
		Material() :
			RefCntObject(),
			m_Name(""),
			m_Params(0),
			m_MtlInsId(0)
		{}

		virtual ~Material() = default;

		virtual void				Release() override;
		virtual const std::string&	GetName() const override { return m_Name; }

		virtual MaterialParam		GetParamByName(const std::string& name) override;
		virtual MaterialParam		GetParamByIndex(uint32_t index) override;
		virtual uint32_t			GetParamNum() const override;

		bool						AddParam(const MaterialParam& param);

	private:
		std::string					m_Name;
		std::vector<MaterialParam>	m_Params;

		int							m_MtlInsId;
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

		virtual ~MaterialInstance() = default;

		virtual void				Release() override;

		virtual const std::string&	GetName() const override { return m_Name; }

	private:
		std::string					m_Name;
		std::vector<MaterialParam>	m_Params;
		IMaterial*					m_pMaterial;
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
		virtual IMaterialInstance*		GetMaterialInstance(IMaterial* pMaterial) override;

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

	MaterialParam Material::GetParamByName(const std::string& name)
	{
		for (size_t i = 0; i < m_Params.size(); ++i)
		{
			MaterialParam& param = m_Params[i];
			if (param.GetName() == name)
			{
				return param;
			}
		}

		return MaterialParam();
	}

	MaterialParam Material::GetParamByIndex(uint32_t index)
	{
		if (index < m_Params.size())
		{
			return m_Params[index];
		}
		else
		{
			return MaterialParam();
		}
	}

	uint32_t Material::GetParamNum() const
	{
		return (uint32_t)(m_Params.size());
	}

	bool Material::AddParam(const MaterialParam& param)
	{
		if (param.GetType() != MATERIAL_PARAM_TYPE::UNKNOWN)
		{
			m_Params.push_back(param);
			return true;
		}
		else
		{
			return false;
		}
	}

	MaterialInstance::MaterialInstance(IMaterial* pMtl) :
		m_Name(""),
		m_Params(0),
		m_pMaterial(pMtl)
	{
		Material* pRealMtl = (Material*)pMtl;
		int mtlInsId = pRealMtl->m_MtlInsId;

		std::string mtlInsName = pMtl->GetName() + "_" + std::to_string(mtlInsId);
		m_Name = mtlInsName;

		for (uint32_t i = 0; i < pMtl->GetParamNum(); ++i)
		{
			const MaterialParam& mtlParam = pMtl->GetParamByIndex(i);
			m_Params.push_back(mtlParam);
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
			pMaterial = new Material();
			m_MaterialMap.insert({ filePath, pMaterial });
		}
		else
		{
			pMaterial = (Material*)materialIter->second;
			pMaterial->AddRef();
		}

		return pMaterial;
	}

	IMaterialInstance* MaterialManager::GetMaterialInstance(IMaterial* pMaterial)
	{
		MaterialInstance* pMtlIns = new MaterialInstance(pMaterial);

		m_MaterialInsMap.insert({ pMtlIns->GetName(), pMtlIns });

		return pMtlIns;
	}

	void MaterialManager::ReleaseMaterial(Material* pMaterial)
	{
		const std::string& matName = pMaterial->GetName();
		if (pMaterial->SubRef() == 0)
		{
			m_MaterialMap.erase(matName);
		}
	}

	void MaterialManager::ReleaseMaterialInstance(MaterialInstance* pMaterialIns)
	{
		const std::string& matName = pMaterialIns->GetName();
		if (pMaterialIns->SubRef() == 0)
		{
			m_MaterialInsMap.erase(matName);
		}
	}

}// namespace RenderDog