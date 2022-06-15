////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: D3D11Shader.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Shader.h"
#include "RefCntObject.h"
#include "D3D11InputLayout.h"
#include "D3D11Renderer.h"

#include <fstream>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <unordered_map>

namespace RenderDog
{
	//=========================================================================
	//    D3D11Shader
	//=========================================================================
	class D3D11Shader : public IShader, public RefCntObject
	{
		friend class D3D11ShaderManager;

	public:
		D3D11Shader();
		virtual ~D3D11Shader();

		void						CompileFromFile(const ShaderCompileDesc& desc);

	protected:
		ID3DBlob*					m_pCompiledCode;
	};

	//=========================================================================
	//    D3D11VertexShader
	//=========================================================================
	class D3D11VertexShader : public D3D11Shader
	{
	public:
		D3D11VertexShader();
		D3D11VertexShader(VERTEX_TYPE vertexType);
		virtual ~D3D11VertexShader();

		virtual bool				Init() override;
		virtual void				Release() override;

		virtual void				SetToContext() override;

	private:
		ID3D11VertexShader* m_pVS;
		D3D11InputLayout* m_pInputLayout;
		VERTEX_TYPE					m_VertexType;
	};

	//=========================================================================
	//    D3D11PixelShader
	//=========================================================================
	class D3D11PixelShader : public D3D11Shader
	{
	public:
		D3D11PixelShader();
		virtual ~D3D11PixelShader();

		virtual bool				Init() override;
		virtual void				Release() override;

		virtual void				SetToContext() override;

	private:
		ID3D11PixelShader* m_pPS;
	};

	//=========================================================================
	//    D3D11ShaderManager
	//=========================================================================
	class D3D11ShaderManager : public IShaderManager
	{
	private:
		typedef std::unordered_map<std::string, D3D11Shader*> ShaderHashMap;

	public:
		D3D11ShaderManager() = default;
		virtual ~D3D11ShaderManager() = default;

		virtual IShader*			GetVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc) override;
		virtual IShader*			GetPixelShader(const ShaderCompileDesc& desc) override;

		void						ReleaseShader(D3D11Shader* pShader);

	private:
		ShaderHashMap				m_ShaderMap;
	};

	D3D11ShaderManager	g_D3D11ShaderManager;
	IShaderManager*		g_pIShaderManager = &g_D3D11ShaderManager;


	//=========================================================================
	//    Function Implementation
	//=========================================================================
	D3D11Shader::D3D11Shader() :
		m_pCompiledCode(nullptr)
	{}

	D3D11Shader::~D3D11Shader()
	{}

	void D3D11Shader::CompileFromFile(const ShaderCompileDesc& desc)
	{
		std::ifstream input(desc.fileName);
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
		if (desc.shaderMacros)
		{
			d3d11ShaderMacro.Name = desc.shaderMacros->name.c_str();
			d3d11ShaderMacro.Definition = desc.shaderMacros->definition.c_str();
		}
		ID3DInclude* pShaderInclude = D3D_COMPILE_STANDARD_FILE_INCLUDE;
		ID3DBlob* pErroMsg = nullptr;
		HRESULT hr = D3DCompile(sourceCode.c_str(), sourceCode.length(),
								desc.fileName.c_str(), &d3d11ShaderMacro, pShaderInclude, desc.entryPoint.c_str(),
								desc.target.c_str(), desc.compileFlag, 0, &m_pCompiledCode, &pErroMsg);

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

	D3D11VertexShader::D3D11VertexShader() :
		D3D11Shader(),
		m_pVS(nullptr),
		m_pInputLayout(nullptr),
		m_VertexType(VERTEX_TYPE::SIMPLE)
	{}

	D3D11VertexShader::D3D11VertexShader(VERTEX_TYPE vertexType):
		D3D11Shader(),
		m_pVS(nullptr),
		m_pInputLayout(nullptr),
		m_VertexType(vertexType)
	{}

	D3D11VertexShader::~D3D11VertexShader()
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
		g_D3D11ShaderManager.ReleaseShader(this);
	}

	void D3D11VertexShader::SetToContext()
	{
		m_pInputLayout->SetToContext();

		g_pD3D11ImmediateContext->VSSetShader(m_pVS, nullptr, 0);
	}

	

	D3D11PixelShader::D3D11PixelShader() :
		m_pPS(nullptr)
	{}

	D3D11PixelShader::~D3D11PixelShader()
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
		g_D3D11ShaderManager.ReleaseShader(this);
	}

	void D3D11PixelShader::SetToContext()
	{
		g_pD3D11ImmediateContext->PSSetShader(m_pPS, nullptr, 0);
	}


	IShader* D3D11ShaderManager::GetVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc)
	{
		D3D11Shader* pVertexShader = nullptr;

		auto shader = m_ShaderMap.find(desc.fileName);
		if (shader != m_ShaderMap.end())
		{
			pVertexShader = shader->second;
		}
		else
		{
			pVertexShader = new D3D11VertexShader(vertexType);
			pVertexShader->CompileFromFile(desc);
			pVertexShader->Init();

			m_ShaderMap.insert({ desc.fileName, pVertexShader });
		}

		pVertexShader->AddRef();

		return pVertexShader;
	}

	IShader* D3D11ShaderManager::GetPixelShader(const ShaderCompileDesc& desc)
	{
		D3D11Shader* pPixelShader = nullptr;

		auto shader = m_ShaderMap.find(desc.fileName);
		if (shader != m_ShaderMap.end())
		{
			pPixelShader = shader->second;
		}
		else
		{
			pPixelShader = new D3D11PixelShader();
			pPixelShader->CompileFromFile(desc);
			pPixelShader->Init();

			m_ShaderMap.insert({ desc.fileName, pPixelShader });
		}

		pPixelShader->AddRef();

		return pPixelShader;
	}

	void D3D11ShaderManager::ReleaseShader(D3D11Shader* pShader)
	{
		if (pShader)
		{
			pShader->SubRef();
		}
	}

}// namespace RenderDog