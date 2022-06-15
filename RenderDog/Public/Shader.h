////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Shader.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "Vertex.h"

#include <string>

namespace RenderDog
{

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

	class IShader
	{
	protected:
		virtual	~IShader() = default;

	public:
		virtual bool				Init() = 0;
		virtual void				Release() = 0;

		virtual void				SetToContext() = 0;
	};

	class IShaderManager
	{
	public:
		virtual	~IShaderManager() = default;

		virtual IShader*			GetVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc) = 0;
		virtual IShader*			GetPixelShader(const ShaderCompileDesc& desc) = 0;
	};

	extern IShaderManager* g_pIShaderManager;

}// namespace RenderDog
