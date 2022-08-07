////////////////////////////////////////
//RenderDog <·,·>
//FileName: D3D11Shader.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Shader.h"
#include "RefCntObject.h"
#include "D3D11InputLayout.h"
#include "D3D11Renderer.h"
#include "Buffer.h"
#include "Texture.h"
#include "Matrix.h"

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
	public:
		D3D11Shader();
		virtual ~D3D11Shader();

		virtual const std::string&						GetFileName() const override { return m_fileName; }

		bool											CompileFromFile(const ShaderCompileDesc& desc);

	protected:
		std::string										m_fileName;

		ID3DBlob*										m_pCompiledCode;
		ID3D11ShaderReflection*							m_pShaderReflector;

		std::vector<D3D11_SHADER_INPUT_BIND_DESC>		m_ShaderInputBindDescs;
		std::unordered_map<std::string, uint32_t>		m_ShaderResourceViewMap;
		std::unordered_map<std::string, uint32_t>		m_SamplerStateMap;
		std::unordered_map<std::string, uint32_t>		m_ConstantBufferMap;
		std::unordered_map<std::string, ShaderParam*>	m_ShaderParamMap;
	};

	//=========================================================================
	//    D3D11VertexShader
	//=========================================================================
	class D3D11VertexShader : public D3D11Shader
	{
	public:
		D3D11VertexShader();
		explicit D3D11VertexShader(VERTEX_TYPE vertexType);
		virtual ~D3D11VertexShader();

		virtual bool				Init() override;
		virtual void				Release() override;

		virtual ShaderParam*		GetShaderParamPtrByName(const std::string& name) override { return nullptr; }

		virtual void				Apply() override;

	private:
		ID3D11VertexShader*			m_pVS;
		D3D11InputLayout*			m_pInputLayout;
		VERTEX_TYPE					m_VertexType;
	};

	class D3D11StaticModelVertexShader : public D3D11VertexShader
	{
	private:
		struct GlobalConstantData
		{
			Matrix4x4	viewMatrix;
			Matrix4x4	projMatrix;
			Vector4		mainCameraWorldPos;
		};

	public:
		D3D11StaticModelVertexShader();
		explicit D3D11StaticModelVertexShader(VERTEX_TYPE vertexType);
		virtual ~D3D11StaticModelVertexShader();

		virtual bool				Init() override;
		virtual void				Release() override;

		virtual ShaderParam*		GetShaderParamPtrByName(const std::string& name) override;

		virtual void				Apply() override;

	protected:
		ShaderParam					m_LocalToWorldMatrix;
		ShaderParam					m_WorldToViewMatrix;
		ShaderParam					m_ViewToClipMatrix;
		ShaderParam					m_WorldEyePostion;
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

		virtual ShaderParam*		GetShaderParamPtrByName(const std::string& name) override { return nullptr; }

		virtual void				Apply() override;

	private:
		ID3D11PixelShader*			m_pPS;
	};

	class D3D11DirectionalLightingPixelShader : public D3D11PixelShader
	{
	public:
		D3D11DirectionalLightingPixelShader();
		virtual ~D3D11DirectionalLightingPixelShader();

		virtual bool				Init() override;
		virtual void				Release() override;

		virtual ShaderParam*		GetShaderParamPtrByName(const std::string& name) override { return nullptr; }

		virtual void				Apply() override;

	protected:

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
		virtual IShader*			GetStaticModelVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc) override;

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
		m_pCompiledCode(nullptr),
		m_pShaderReflector(nullptr),
		m_fileName("")
	{}

	D3D11Shader::~D3D11Shader()
	{}

	bool D3D11Shader::CompileFromFile(const ShaderCompileDesc& desc)
	{
		std::ifstream input(desc.fileName);
		if (!input)
		{
			MessageBox(NULL, "Cannot open shader file!", NULL, MB_OK);
			return false;
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

				return false;
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
			}
			
			return false;
		}

		D3DReflect(m_pCompiledCode->GetBufferPointer(), m_pCompiledCode->GetBufferSize(), __uuidof(ID3D11ShaderReflection), (void**)&m_pShaderReflector);
		if (!m_pShaderReflector)
		{
			return false;
		}

		for (uint32_t i = 0; ; ++i)
		{
			D3D11_SHADER_INPUT_BIND_DESC desc;
			if (FAILED(m_pShaderReflector->GetResourceBindingDesc(i, &desc)))
			{
				break;
			}

			m_ShaderInputBindDescs.push_back(desc);
		}

		for (size_t i = 0; i < m_ShaderInputBindDescs.size(); ++i)
		{
			const D3D11_SHADER_INPUT_BIND_DESC& desc = m_ShaderInputBindDescs[i];
			if (desc.Type == D3D_SIT_TEXTURE)
			{
				m_ShaderResourceViewMap.insert({ desc.Name, desc.BindPoint });
			}
			
			if (desc.Type == D3D_SIT_SAMPLER)
			{
				m_SamplerStateMap.insert({ desc.Name, desc.BindPoint });
			}

			if (desc.Type == D3D_SIT_CBUFFER)
			{
				m_ConstantBufferMap.insert({ desc.Name, desc.BindPoint });
			}
		}

		m_fileName = desc.fileName;

		return true;
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

		if (m_pShaderReflector)
		{
			m_pShaderReflector->Release();
			m_pShaderReflector = nullptr;
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

	void D3D11VertexShader::Apply()
	{
		m_pInputLayout->SetToContext();

		g_pD3D11ImmediateContext->VSSetShader(m_pVS, nullptr, 0);
	}

	D3D11StaticModelVertexShader::D3D11StaticModelVertexShader() :
		D3D11VertexShader(),
		m_LocalToWorldMatrix("ComVar_Matrix_LocalToWorld", SHADER_PARAM_TYPE::MATRIX),
		m_WorldToViewMatrix("ComVar_Matrix_WorldToView", SHADER_PARAM_TYPE::MATRIX),
		m_ViewToClipMatrix("ComVar_Matrix_ViewToClip", SHADER_PARAM_TYPE::MATRIX),
		m_WorldEyePostion("ComVar_Vector_WorldEyePosition", SHADER_PARAM_TYPE::FLOAT_VECTOR)
	{
		m_ShaderParamMap.insert({ "ComVar_Matrix_LocalToWorld", &m_LocalToWorldMatrix });
		m_ShaderParamMap.insert({ "ComVar_Matrix_WorldToView", &m_WorldToViewMatrix });
		m_ShaderParamMap.insert({ "ComVar_Matrix_ViewToClip", &m_ViewToClipMatrix });
		m_ShaderParamMap.insert({ "ComVar_Vector_WorldEyePosition", &m_WorldEyePostion });
	}

	D3D11StaticModelVertexShader::D3D11StaticModelVertexShader(VERTEX_TYPE vertexType) :
		D3D11VertexShader(vertexType),
		m_LocalToWorldMatrix("ComVar_Matrix_LocalToWorld", SHADER_PARAM_TYPE::MATRIX),
		m_WorldToViewMatrix("ComVar_Matrix_WorldToView", SHADER_PARAM_TYPE::MATRIX),
		m_ViewToClipMatrix("ComVar_Matrix_ViewToClip", SHADER_PARAM_TYPE::MATRIX),
		m_WorldEyePostion("ComVar_Vector_WorldEyePosition", SHADER_PARAM_TYPE::FLOAT_VECTOR)
	{
		m_ShaderParamMap.insert({ "ComVar_Matrix_LocalToWorld", &m_LocalToWorldMatrix });
		m_ShaderParamMap.insert({ "ComVar_Matrix_WorldToView", &m_WorldToViewMatrix });
		m_ShaderParamMap.insert({ "ComVar_Matrix_ViewToClip", &m_ViewToClipMatrix });
		m_ShaderParamMap.insert({ "ComVar_Vector_WorldEyePosition", &m_WorldEyePostion });
	}

	D3D11StaticModelVertexShader::~D3D11StaticModelVertexShader()
	{}

	bool D3D11StaticModelVertexShader::Init()
	{
		return D3D11VertexShader::Init();
	}

	void D3D11StaticModelVertexShader::Release()
	{
		D3D11VertexShader::Release();
	}

	ShaderParam* D3D11StaticModelVertexShader::GetShaderParamPtrByName(const std::string& name)
	{
		auto shaderParamIter = m_ShaderParamMap.find(name);
		if (shaderParamIter != m_ShaderParamMap.end())
		{
			return shaderParamIter->second;
		}
		else
		{
			return nullptr;
		}
	}

	void D3D11StaticModelVertexShader::Apply()
	{
		D3D11VertexShader::Apply();

		//TODO: 更新ConstantBuffer，并设置到context上；
		IConstantBuffer* pGlobalConstantBuffer = g_pIBufferManager->GetConstantBufferByName("ComVar_ConstantBuffer_Global");
		if (!pGlobalConstantBuffer)
		{
			return;
		}

		GlobalConstantData globalCBData;
		globalCBData.viewMatrix = m_WorldToViewMatrix.GetMatrix4x4();
		globalCBData.projMatrix = m_ViewToClipMatrix.GetMatrix4x4();
		globalCBData.mainCameraWorldPos = m_WorldEyePostion.GetVector4();

		pGlobalConstantBuffer->Update(&globalCBData, sizeof(globalCBData));

		auto cbIter = m_ConstantBufferMap.find(pGlobalConstantBuffer->GetName());
		if (cbIter != m_ConstantBufferMap.end())
		{
			uint32_t cbSlot = cbIter->second;
			ID3D11Buffer* pCB = (ID3D11Buffer*)(pGlobalConstantBuffer->GetResource());
			g_pD3D11ImmediateContext->VSSetConstantBuffers(cbSlot, 1, (ID3D11Buffer**)&pCB);
		}
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

		if (m_pShaderReflector)
		{
			m_pShaderReflector->Release();
			m_pShaderReflector = nullptr;
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

	void D3D11PixelShader::Apply()
	{
		g_pD3D11ImmediateContext->PSSetShader(m_pPS, nullptr, 0);
	}

	/*void D3D11PixelShader::SetTextureByName(const std::string& name, ITexture* pTexture, ISamplerState* pSampler)
	{
		auto srvIter = m_ShaderResourceViewMap.find(name);
		if (srvIter == m_ShaderResourceViewMap.end())
		{
			return;
		}

		const std::string& samplerName = name + "Sampler";
		auto samplerIter = m_SamplerStateMap.find(samplerName);
		if (samplerIter == m_SamplerStateMap.end())
		{
			return;
		}

		uint32_t srtSlot = srvIter->second;
		ID3D11ShaderResourceView* pDiffSRV = (ID3D11ShaderResourceView*)(pTexture->GetShaderResourceView());
		g_pD3D11ImmediateContext->PSSetShaderResources(srtSlot, 1, &pDiffSRV);

		uint32_t samplerSlot = samplerIter->second;
		pSampler->SetToPixelShader(samplerSlot);
	}*/


	IShader* D3D11ShaderManager::GetVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc)
	{
		D3D11Shader* pVertexShader = nullptr;

		auto shader = m_ShaderMap.find(desc.fileName);
		if (shader != m_ShaderMap.end())
		{
			pVertexShader = shader->second;
			pVertexShader->AddRef();
		}
		else
		{
			pVertexShader = new D3D11VertexShader(vertexType);
			pVertexShader->CompileFromFile(desc);
			pVertexShader->Init();

			m_ShaderMap.insert({ desc.fileName, pVertexShader });
		}

		return pVertexShader;
	}

	IShader* D3D11ShaderManager::GetPixelShader(const ShaderCompileDesc& desc)
	{
		D3D11Shader* pPixelShader = nullptr;

		auto shader = m_ShaderMap.find(desc.fileName);
		if (shader != m_ShaderMap.end())
		{
			pPixelShader = shader->second;
			pPixelShader->AddRef();
		}
		else
		{
			pPixelShader = new D3D11PixelShader();
			pPixelShader->CompileFromFile(desc);
			pPixelShader->Init();

			m_ShaderMap.insert({ desc.fileName, pPixelShader });
		}

		return pPixelShader;
	}

	IShader* D3D11ShaderManager::GetStaticModelVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc)
	{
		D3D11Shader* pVertexShader = nullptr;

		auto shader = m_ShaderMap.find(desc.fileName);
		if (shader != m_ShaderMap.end())
		{
			pVertexShader = shader->second;
			pVertexShader->AddRef();
		}
		else
		{
			pVertexShader = new D3D11StaticModelVertexShader(vertexType);
			pVertexShader->CompileFromFile(desc);
			pVertexShader->Init();

			m_ShaderMap.insert({ desc.fileName, pVertexShader });
		}

		return pVertexShader;
	}

	void D3D11ShaderManager::ReleaseShader(D3D11Shader* pShader)
	{
		if (pShader)
		{
			std::string shaderName = pShader->GetFileName();

			int refCnt = pShader->SubRef();
			if (refCnt == 0)
			{
				m_ShaderMap.erase(shaderName);
			}
		}
	}

}// namespace RenderDog