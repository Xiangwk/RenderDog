////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Shader.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "RefCntObject.h"
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
	public:
		virtual	~IShader() = default;

		virtual void				CompileFromFile(const ShaderCompileDesc& desc) = 0;

		virtual void*				GetCompiledCode() const = 0;
		virtual uint32_t			GetCompiledCodeSize() const = 0;

		virtual bool				Init() = 0;
		virtual void				Release() = 0;

		virtual void				SetToContext() = 0;
	};

	class IShaderManager
	{
	public:
		virtual	~IShaderManager() = default;

		virtual IShader*			CreateVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc) = 0;
		virtual IShader*			CreatePixelShader(const ShaderCompileDesc& desc) = 0;

		virtual void				ReleaseShader(IShader* pShader) = 0;
	};

	extern IShaderManager* g_pIShaderManager;

}// namespace RenderDog
