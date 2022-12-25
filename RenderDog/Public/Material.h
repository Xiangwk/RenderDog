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

	enum class MATERIAL_PARAM_TYPE
	{
		UNKNOWN = 0,
		SCALAR,
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

		~MaterialParam() = default;

		MaterialParam& operator=(const MaterialParam& param);

		float							GetScalar() { return m_FloatValue; }
		Vector4							GetVector4() { return m_Vector4Value; }
		ITexture2D*						GetTexture2D() { return m_pTexture2DValue; }
		ISamplerState*					GetSamplerState() { return pSamplerValue; }

		void							SetScalar(float value) { m_FloatValue = value; }
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
			float						m_FloatValue;
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
		virtual IMaterial*				GetMaterial(const std::string& filePath, bool bIsUserMtl) = 0;
		virtual IMaterialInstance*		GetMaterialInstance(IMaterial* pMaterial, const std::vector<MaterialParam>* pMtlParams = nullptr) = 0;
	};

	extern IMaterialManager* g_pMaterialManager;

}// namespace RenderDog