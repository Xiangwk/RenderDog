////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Shader.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "Vertex.h"
#include "Matrix.h"
#include "GlobalValue.h"

#include <string>

namespace RenderDog
{
	class	ITexture;
	class	ISamplerState;
	class	IConstantBuffer;

	//VertexShader
	const std::string g_SimpleModelVertexShadreFilePath			= "Shaders/SimpleModelVertexShader.hlsl";
	const std::string g_StaticModelVertexShaderFilePath			= "Shaders/StaticModelVertexShader.hlsl";
	const std::string g_SkinModelVertexShaderFilePath			= "Shaders/SkinModelVertexShader.hlsl";
	const std::string g_SkyVertexShaderFilePath					= "Shaders/SkyVertexShader.hlsl";
	const std::string g_ShadowDepthStaticVertexShaderFilePath	= "Shaders/ShadowDepthStaticModelVertexShader.hlsl";
	const std::string g_ShadowDepthSkinVertexShaderFilePath		= "Shaders/ShadowDepthSkinModelVertexShader.hlsl";
	
	//PixelShader
	const std::string g_SingleColorPixelShader					= "Shaders/SingleColorPixelShader.hlsl";
	const std::string g_DirectionalLightingPixelShaderFilePath	= "Shaders/PhongLightingPixelShader.hlsl";
	const std::string g_SkyPixelShaderFilePath					= "Shaders/SkyPixelShader.hlsl";
	const std::string g_ShadowDepthPixelShaderFilePath			= "Shaders/ShadowDepthPixelShader.hlsl";

	enum class SHADER_PARAM_TYPE
	{
		UNKNOWN = 0,
		FLOAT_SCALAR,
		FLOAT_VECTOR,
		MATRIX,
		TEXTURE,
		SAMPLER
	};

	struct SkinModelPerObjectTransform
	{
		Matrix4x4 BoneFinalTransformMatrix[g_MaxBoneNum];

		SkinModelPerObjectTransform()
		{
			for (int i = 0; i < g_MaxBoneNum; ++i)
			{
				BoneFinalTransformMatrix[i].Identity();
			}
		}
	};

	struct ViewParamData
	{
		Matrix4x4	worldToViewMatrix;
		Matrix4x4	viewToClipMatrix;
		Vector4		mainCameraWorldPos;
	};

	struct DirectionalLightData
	{
		Vector4		direction;
		Vector4		color;
	};

	struct ShadowDepthMatrixData
	{
		Matrix4x4	worldToViewMatrix;
		Matrix4x4	viewToClipMatrix;
	};

	struct ShadowParamData
	{
		Vector4		param0;			//x: shadowDepthOffset, y: shadowDistance
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

		ShaderParam(const ShaderParam& param) :
			m_Name(param.m_Name),
			m_Type(param.m_Type)
		{
			switch (m_Type)
			{
			case SHADER_PARAM_TYPE::UNKNOWN:
				break;
			case SHADER_PARAM_TYPE::FLOAT_SCALAR:
				m_Float = param.m_Float;
				break;
			case SHADER_PARAM_TYPE::FLOAT_VECTOR:
				m_Vector = param.m_Vector;
				break;
			case SHADER_PARAM_TYPE::MATRIX:
				m_Matrix = param.m_Matrix;
				break;
			case SHADER_PARAM_TYPE::TEXTURE:
				m_pTexture = param.m_pTexture;
				break;
			case SHADER_PARAM_TYPE::SAMPLER:
				m_pSampler = param.m_pSampler;
				break;
			default:
				break;
			}
		}

		ShaderParam(const std::string& name, SHADER_PARAM_TYPE paramType) :
			m_Name(name),
			m_Type(paramType)
		{}

		~ShaderParam()
		{}

		ShaderParam& operator=(const ShaderParam& param)
		{
			m_Name = param.m_Name;
			m_Type = param.m_Type;

			switch (m_Type)
			{
			case RenderDog::SHADER_PARAM_TYPE::UNKNOWN:
				break;
			case RenderDog::SHADER_PARAM_TYPE::FLOAT_SCALAR:
				m_Float = param.m_Float;
				break;
			case RenderDog::SHADER_PARAM_TYPE::FLOAT_VECTOR:
				m_Vector = param.m_Vector;
				break;
			case RenderDog::SHADER_PARAM_TYPE::MATRIX:
				m_Matrix = param.m_Matrix;
				break;
			case RenderDog::SHADER_PARAM_TYPE::TEXTURE:
				m_pTexture = param.m_pTexture;
				break;
			case RenderDog::SHADER_PARAM_TYPE::SAMPLER:
				m_pSampler = param.m_pSampler;
				break;
			default:
				break;
			}

			return *this;
		}

		const std::string&		GetName() const							{ return m_Name; }
		SHADER_PARAM_TYPE		GetType() const							{ return m_Type; }

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

	struct ShaderPerObjParam
	{
		IConstantBuffer* pPerObjectCB;
		IConstantBuffer* pBoneTransformCB;

		ShaderPerObjParam() :
			pPerObjectCB(nullptr),
			pBoneTransformCB(nullptr)
		{}
	};

	class IShader
	{
	protected:
		virtual	~IShader() = default;

	public:
		virtual void				Release() = 0;

		virtual const std::string&	GetFileName() const = 0;

		virtual ShaderParam*		GetShaderParamPtrByName(const std::string& name) = 0;

		virtual void				Apply(const ShaderPerObjParam* pPerObjParam = nullptr) = 0;
	};

	class IShaderManager
	{
	public:
		virtual	~IShaderManager() = default;

		virtual IShader*			GetVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc) = 0;
		virtual IShader*			GetPixelShader(const ShaderCompileDesc& desc) = 0;

		virtual IShader*			GetModelVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc) = 0;

		virtual IShader*			GetDirectionLightingPixelShader(const ShaderCompileDesc& desc) = 0;
		virtual IShader*			GetSkyPixelShader(const ShaderCompileDesc& desc) = 0;
	};

	extern IShaderManager* g_pIShaderManager;

}// namespace RenderDog
