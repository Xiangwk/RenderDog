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
	class Material : public IMaterial, public RefCntObject
	{
	public:
		Material() :
			m_Name(""),
			m_Params(0)
		{}

		virtual ~Material() = default;

		virtual void				Release() override;
		virtual const std::string&	GetName() const override { return m_Name; }
		virtual MaterialParam		GetParamByName(const std::string& name) override;

		bool						AddParam(const MaterialParam& param);

	private:
		std::string					m_Name;
		std::vector<MaterialParam>	m_Params;
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

		virtual ~MaterialInstance() = default;

		virtual void				Release() override;

		virtual const std::string&	GetName() const override { return m_Name; }

	private:
		std::string					m_Name;
		std::vector<MaterialParam>	m_Params;
		Material*					m_pMaterial;
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
		virtual IMaterialInstance*		GetMaterialInstance(const IMaterial* pMaterial) override;

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

	void MaterialInstance::Release()
	{
		g_MaterialManager.ReleaseMaterialInstance(this);
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