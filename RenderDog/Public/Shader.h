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

	class IShader
	{
	public:
		virtual	~IShader() = default;

		virtual void		CompileFromFile(const std::string& fileName,
											const ShaderMacro* shaderMacros,
											const std::string& entryPoint,
											const std::string& target,
											unsigned int compileFlag) = 0;

		virtual void*		GetCompiledCode() const = 0;
		virtual uint32_t	GetCompiledCodeSize() const = 0;

		virtual bool		Init() = 0;
		virtual void		Release() = 0;

		virtual void		SetToContext() = 0;
	};

	class IShaderManager
	{
	public:
		virtual	~IShaderManager() = default;

		virtual IShader*	CreateVertexShader(VertexType vertexType) = 0;
		virtual IShader*	CreatePixelShader() = 0;
	};

	extern IShaderManager*  g_pIShaderManager;

}// namespace RenderDog
