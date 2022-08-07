////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Shader.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "Vertex.h"
#include "Matrix.h"

#include <string>

namespace RenderDog
{
	class	ITexture;
	class	ISamplerState;
	class	IConstantBuffer;

	enum class SHADER_PARAM_TYPE
	{
		UNKNOWN = 0,
		FLOAT_SCALAR,
		FLOAT_VECTOR,
		MATRIX,
		TEXTURE,
		SAMPLER
	};

	struct ShaderMacro
	{
		std::string	name;
		std::string	definition;
	};

	struct ShaderCompileDesc
	{
		std::string		fileName;
		ShaderMacro*	shaderMacros;
		std::string		entryPoint;
		std::string		target;
		unsigned int	compileFlag;

		ShaderCompileDesc(const std::string& file,
			ShaderMacro* macros,
			const std::string& entry,
			const std::string& target,
			unsigned int flag) :
			fileName(file),
			shaderMacros(macros),
			entryPoint(entry),
			target(target),
			compileFlag(flag)
		{}
	};

	class ShaderParam
	{
	public:
		ShaderParam() :
			m_Name(""),
			m_Type(SHADER_PARAM_TYPE::UNKNOWN)
		{}
		ShaderParam(const std::string& name, SHADER_PARAM_TYPE paramType) :
			m_Name(name),
			m_Type(paramType)
		{}

		~ShaderParam()
		{}

		const std::string&		GetName() const							{ return m_Name; }

		void					SetFloat(float value)					{ m_Float = value; }
		void					SetVector4(const Vector4& value)		{ m_Vector = value; }
		void					SetMatrix4x4(const Matrix4x4& value)	{ m_Matrix = value; }
		void					SetTexture(ITexture* pValue)			{ m_pTexture = pValue; }
		void					SetSampler(ISamplerState* pValue)		{ m_pSampler = pValue; }

		float					GetFloat() const						{ return m_Float; }
		Vector4					GetVector4() const						{ return m_Vector; }
		Matrix4x4				GetMatrix4x4() const					{ return m_Matrix; }
		ITexture*				GetTexture() const						{ return m_pTexture; }
		ISamplerState*			GetSampler() const						{ return m_pSampler; }

	private:
		union
		{
			float				m_Float;
			Vector4				m_Vector;
			Matrix4x4			m_Matrix;
			ITexture*			m_pTexture;
			ISamplerState*		m_pSampler;
		};

		std::string				m_Name;
		SHADER_PARAM_TYPE		m_Type;
	};

	class IShader
	{
	protected:
		virtual	~IShader() = default;

	public:
		virtual bool				Init() = 0;
		virtual void				Release() = 0;

		virtual const std::string&	GetFileName() const = 0;

		virtual ShaderParam*		GetShaderParamPtrByName(const std::string& name) = 0;

		virtual void				Apply() = 0;
	};

	class IShaderManager
	{
	public:
		virtual	~IShaderManager() = default;

		virtual IShader*			GetVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc) = 0;
		virtual IShader*			GetPixelShader(const ShaderCompileDesc& desc) = 0;

		virtual IShader*			GetStaticModelVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc) = 0;
		virtual IShader*			GetDirectionLightingPixelShader(const ShaderCompileDesc& desc) = 0;
	};

	extern IShaderManager* g_pIShaderManager;

}// namespace RenderDog
