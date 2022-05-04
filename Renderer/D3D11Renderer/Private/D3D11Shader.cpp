////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: D3D11Shader.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Shader.h"
#include "D3D11InputLayout.h"
#include "D3D11Renderer.h"

#include <fstream>
#include <d3d11.h>
#include <d3dcompiler.h>

namespace RenderDog
{
	class D3D11Shader : public IShader
	{
	public:
		D3D11Shader();
		virtual ~D3D11Shader();

		virtual void			CompileFromFile(const std::string& fileName,
												const ShaderMacro* shaderMacros,
												const std::string& entryPoint,
												const std::string& target,
												unsigned int compileFlag) override;

		virtual void*			GetCompiledCode() const override;
		virtual uint32_t		GetCompiledCodeSize() const override;

	protected:
		ID3DBlob*				m_pCompiledCode;
	};

	D3D11Shader::D3D11Shader() :
		m_pCompiledCode(nullptr)
	{}

	D3D11Shader::~D3D11Shader()
	{}

	void D3D11Shader::CompileFromFile(const std::string& fileName,
									  const ShaderMacro* shaderMacros,
									  const std::string& entryPoint,
									  const std::string& target,
									  unsigned int compileFlag)
	{
		std::ifstream input(fileName);
		if (!input)
		{
			MessageBox(NULL, "Cannot open shader file!", NULL, MB_OK);
			return;
		}

		std::string sourceCode;
		while (!input.eof())
		{
			std::string line;
			getline(input, line);
			sourceCode += (line + "\n");
		}
		input.close();

		D3D_SHADER_MACRO d3d11ShaderMacro = {};
		if (shaderMacros)
		{
			d3d11ShaderMacro.Name = shaderMacros->name.c_str();
			d3d11ShaderMacro.Definition = shaderMacros->definition.c_str();
		}
		ID3DInclude* pShaderInclude = D3D_COMPILE_STANDARD_FILE_INCLUDE;
		ID3DBlob* pErroMsg = nullptr;
		HRESULT hr = D3DCompile(sourceCode.c_str(), sourceCode.length(),
								fileName.c_str(), &d3d11ShaderMacro, pShaderInclude, entryPoint.c_str(),
								target.c_str(), compileFlag, 0, &m_pCompiledCode, &pErroMsg);

		if (SUCCEEDED(hr))
		{
			if (pErroMsg)
			{
				const char* output = static_cast<const char*>(pErroMsg->GetBufferPointer());
				OutputDebugString(output);
				pErroMsg->Release();

				return;
			}
		}
		else
		{
			MessageBox(NULL, "Compile shader failed!", NULL, MB_OK);
			if (pErroMsg)
			{
				const char* output = static_cast<const char*>(pErroMsg->GetBufferPointer());
				OutputDebugString(output);
				pErroMsg->Release();

				return;
			}
			else
			{
				return;
			}
		}
	}

	void* D3D11Shader::GetCompiledCode() const
	{
		return m_pCompiledCode->GetBufferPointer();
	}

	uint32_t D3D11Shader::GetCompiledCodeSize() const
	{
		return (uint32_t)m_pCompiledCode->GetBufferSize();
	}

	//=========================================================================
	//    VertexShader
	//=========================================================================

	class D3D11VertexShader : public D3D11Shader
	{
	public:
		D3D11VertexShader();
		D3D11VertexShader(VertexType vertexType);
		virtual ~D3D11VertexShader();

		virtual bool			Init() override;
		virtual void			Release() override;
			
		virtual void			SetToContext() override;

	private:
		ID3D11VertexShader*		m_pVS;
		D3D11InputLayout*		m_pInputLayout;
		VertexType				m_VertexType;
	};

	D3D11VertexShader::D3D11VertexShader() :
		D3D11Shader(),
		m_pVS(nullptr),
		m_pInputLayout(nullptr),
		m_VertexType(VertexType::RD_VERTEX_TYPE_SIMPLE)
	{}

	D3D11VertexShader::D3D11VertexShader(VertexType vertexType):
		D3D11Shader(),
		m_pVS(nullptr),
		m_pInputLayout(nullptr),
		m_VertexType(vertexType)
	{}

	D3D11VertexShader::~D3D11VertexShader()
	{}
	
	bool D3D11VertexShader::Init()
	{
		if (FAILED(g_pD3D11Device->CreateVertexShader(m_pCompiledCode->GetBufferPointer(), m_pCompiledCode->GetBufferSize(), nullptr, &m_pVS)))
		{
			return false;
		}

		m_pInputLayout = new D3D11InputLayout();
		if (!m_pInputLayout->Init(m_VertexType, m_pCompiledCode->GetBufferPointer(), (uint32_t)m_pCompiledCode->GetBufferSize()))
		{
			return false;
		}

		return true;
	}

	void D3D11VertexShader::Release()
	{
		if (m_pInputLayout)
		{
			m_pInputLayout->Release();

			delete m_pInputLayout;
			m_pInputLayout = nullptr;
		}

		if (m_pCompiledCode)
		{
			m_pCompiledCode->Release();
			m_pCompiledCode = nullptr;
		}

		if (m_pVS)
		{
			m_pVS->Release();
			m_pVS = nullptr;
		}
	}

	void D3D11VertexShader::SetToContext()
	{
		m_pInputLayout->SetToContext();

		g_pD3D11ImmediateContext->VSSetShader(m_pVS, nullptr, 0);
	}

	//=========================================================================
	//    PixelShader
	//=========================================================================
	class D3D11PixelShader : public D3D11Shader
	{
	public:
		D3D11PixelShader();
		virtual ~D3D11PixelShader();

		virtual bool			Init() override;
		virtual void			Release() override;

		virtual void			SetToContext() override;

	private:
		ID3D11PixelShader*		m_pPS;
	};

	D3D11PixelShader::D3D11PixelShader() :
		m_pPS(nullptr)
	{}

	D3D11PixelShader::~D3D11PixelShader()
	{}

	bool D3D11PixelShader::Init()
	{
		if (FAILED(g_pD3D11Device->CreatePixelShader(m_pCompiledCode->GetBufferPointer(), m_pCompiledCode->GetBufferSize(), nullptr, &m_pPS)))
		{
			return false;
		}

		return true;
	}

	void D3D11PixelShader::Release()
	{
		if (m_pCompiledCode)
		{
			m_pCompiledCode->Release();
			m_pCompiledCode = nullptr;
		}

		if (m_pPS)
		{
			m_pPS->Release();
			m_pPS = nullptr;
		}
	}

	void D3D11PixelShader::SetToContext()
	{
		g_pD3D11ImmediateContext->PSSetShader(m_pPS, nullptr, 0);
	}


	//=========================================================================
	//    ShaderManager
	//=========================================================================

	class D3D11ShaderManager : public IShaderManager
	{
	public:
		D3D11ShaderManager() = default;
		virtual ~D3D11ShaderManager() = default;

		virtual IShader*	CreateVertexShader(VertexType vertexType) override;
		virtual IShader*	CreatePixelShader() override;
	};

	D3D11ShaderManager	g_D3D11ShaderManager;
	IShaderManager*		g_pIShaderManager = &g_D3D11ShaderManager;

	IShader* D3D11ShaderManager::CreateVertexShader(VertexType vertexType)
	{
		IShader* pVertexShader = new D3D11VertexShader(vertexType);

		return pVertexShader;
	}

	IShader* D3D11ShaderManager::CreatePixelShader()
	{
		IShader* pPixelShader = new D3D11PixelShader();

		return pPixelShader;
	}

}// namespace RenderDog