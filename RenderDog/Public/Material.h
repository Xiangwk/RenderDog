////////////////////////////////////////
// RenderDog <¡¤,¡¤>
// FileName: Material.h
// Written by Xiang Weikang
// Interface of Material
////////////////////////////////////////

#pragma once

#include "Vector.h"

#include <string>
#include <vector>

namespace RenderDog
{
	class	ITexture2D;
	class   ISamplerState;
	class   IShader;

	const std::string MTL_PROPS				= "MaterialProperties";
	const std::string MTL_PARAMS			= "MaterialParameters";
	const std::string MTL_FUNCTIONS			= "MaterialFunctions";

	const std::string MTL_PROPS_USER_MTL	= "UserMaterial";
	const std::string MTL_PROPS_SHADER_FILE = "ShaderFile";
	const std::string MTL_PROPS_FLOAT4		= "float4";
	const std::string MTL_PARAMS_TEXTURE2D	= "Texture2D";

	const std::string MTLINS_DIR			= "MaterialInstance/";

	enum class MATERIAL_PARAM_TYPE
	{
		UNKNOWN = 0,
		VECTOR4,
		TEXTURE2D,
		SAMPLER
	};

	class MaterialParam
	{
	public:
		MaterialParam() :
			m_Name(""),
			m_ParamType(MATERIAL_PARAM_TYPE::UNKNOWN)
		{}

		MaterialParam(const std::string& name, MATERIAL_PARAM_TYPE type) :
			m_Name(name),
			m_ParamType(type)
		{}

		MaterialParam(const MaterialParam& param);

		~MaterialParam()
		{}

		MaterialParam& operator=(const MaterialParam& param);

		Vector4							GetVector4() { return m_Vector4Value; }
		ITexture2D*						GetTexture2D() { return m_pTexture2DValue; }
		ISamplerState*					GetSamplerState() { return pSamplerValue; }

		void							SetVector4(const Vector4& value) { m_Vector4Value = value; }
		void							SetTexture2D(ITexture2D* pValue) { m_pTexture2DValue = pValue; }
		void							SetSamplerState(ISamplerState* pValue) { pSamplerValue = pValue; }

		const std::string&				GetName() const { return m_Name; }
		MATERIAL_PARAM_TYPE				GetType() const { return m_ParamType; }

	private:
		void							CloneParamValue(const MaterialParam& param);

	private:
		union
		{
			Vector4						m_Vector4Value;
			ITexture2D*					m_pTexture2DValue;
			ISamplerState*				pSamplerValue;
		};

		std::string						m_Name;
		MATERIAL_PARAM_TYPE				m_ParamType;
	};

	extern MaterialParam INVALID_MATERIAL_PARAM;

	class IMaterial
	{
	protected:
		virtual ~IMaterial() = default;

	public:
		virtual void					Release() = 0;

		virtual const std::string&		GetName() const = 0;

		virtual void					AddParam(const MaterialParam& param) = 0;

		virtual MaterialParam&			GetParamByName(const std::string& name) = 0;
		virtual MaterialParam&			GetParamByIndex(uint32_t index) = 0;
		virtual uint32_t				GetParamNum() const = 0;

		virtual bool					CreateMaterialShader() = 0;
		virtual IShader*				GetMaterialShader() = 0;
	};


	class IMaterialInstance
	{
	protected:
		virtual ~IMaterialInstance() = default;

	public:
		virtual void					Release() = 0;

		virtual const std::string&		GetName() const = 0;

		virtual IMaterial*				GetMaterial() const = 0;

		virtual MaterialParam&			GetMaterialParamByIndex(uint32_t index) = 0;
		virtual uint32_t				GetMaterialParamNum() const = 0;
	};


	class IMaterialManager
	{
	protected:
		virtual ~IMaterialManager() = default;

	public:
		virtual IMaterial*				GetMaterial(const std::string& filePath) = 0;
		virtual IMaterialInstance*		GetMaterialInstance(IMaterial* pMaterial, const std::vector<MaterialParam>* pMtlParams = nullptr) = 0;
	};

	extern IMaterialManager* g_pMaterialManager;

}// namespace RenderDog