////////////////////////////////////////
// RenderDog <¡¤,¡¤>
// FileName: Material.h
// Written by Xiang Weikang
// Interface of Material
////////////////////////////////////////

#pragma once

#include <string>

namespace RenderDog
{
	struct	Vector4;
	class	ITexture2D;
	class   ISamplerState;

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

		~MaterialParam() = default;

		float*							GetScalar() { return pfloatValue; }
		Vector4*						GetVector4() { return pVector4Value; }
		ITexture2D*						GetTexture2D() { return pTexture2DValue; }
		ISamplerState*					GetSamplerState() { return pSamplerState; }

		const std::string&				GetName() const { return m_Name; }
		MATERIAL_PARAM_TYPE				GetType() const { return m_ParamType; }

	private:
		union
		{
			float*						pfloatValue;
			Vector4*					pVector4Value;
			ITexture2D*					pTexture2DValue;
			ISamplerState*				pSamplerState;
		};

		std::string						m_Name;
		MATERIAL_PARAM_TYPE				m_ParamType;
	};

	class IMaterial
	{
	protected:
		virtual ~IMaterial() = default;

	public:
		virtual void					Release() = 0;

		virtual const std::string&		GetName() const = 0;

		virtual MaterialParam			GetParamByName(const std::string& name) = 0;
		virtual MaterialParam			GetParamByIndex(uint32_t index) = 0;
		virtual uint32_t				GetParamNum() const = 0;
	};


	class IMaterialInstance
	{
	protected:
		virtual ~IMaterialInstance() = default;

	public:
		virtual void					Release() = 0;

		virtual const std::string&		GetName() const = 0;

		virtual const IMaterial*		GetMaterial() const = 0;

		virtual const MaterialParam&	GetMaterialParamByIndex(uint32_t index) const = 0;
		virtual uint32_t				GetMaterialParamNum() const = 0;
	};


	class IMaterialManager
	{
	protected:
		virtual ~IMaterialManager() = default;

	public:
		virtual IMaterial*				GetMaterial(const std::string& filePath) = 0;
		virtual IMaterialInstance*		GetMaterialInstance(IMaterial* pMaterial) = 0;
	};

	extern IMaterialManager* g_pMaterialManager;

}// namespace RenderDog