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
#include "GlobalValue.h"
#include "Material.h"

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
		explicit D3D11Shader(const ShaderCompileDesc& desc);
		virtual ~D3D11Shader();

		virtual const std::string&						GetFileName() const override { return m_fileName; }

		virtual ShaderParam*							GetShaderParamPtrByName(const std::string& name) override;
		virtual int										GetShaderResourceViewSlotByName(const std::string& name) override;
		virtual int										GetSamplerStateSlotByName(const std::string& name) override;

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
		D3D11VertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc);
		virtual ~D3D11VertexShader();

		virtual void				Release() override;

		virtual void				Apply(const ShaderPerObjParam* pPerObjParam = nullptr) override;

		virtual void				ApplyMaterialParams(IMaterialInstance* pMtlIns) override {};

	private:
		ID3D11VertexShader*			m_pVS;
		D3D11InputLayout*			m_pInputLayout;
		VERTEX_TYPE					m_VertexType;
	};

	class D3D11ModelVertexShader : public D3D11VertexShader
	{
	public:
		D3D11ModelVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc);
		virtual ~D3D11ModelVertexShader();

		virtual void				Release() override;

		virtual void				Apply(const ShaderPerObjParam* pPerObjParam = nullptr) override;
	};


	//=========================================================================
	//    D3D11PixelShader
	//=========================================================================
	class D3D11PixelShader : public D3D11Shader
	{
	public:
		explicit D3D11PixelShader(const ShaderCompileDesc& desc);
		virtual ~D3D11PixelShader();

		virtual void				Release() override;

		virtual void				Apply(const ShaderPerObjParam* pPerObjParam = nullptr) override;

		virtual void				ApplyMaterialParams(IMaterialInstance* pMtlIns) override;

	private:
		ID3D11PixelShader*			m_pPS;
	};

	class D3D11SkyPixelShader : public D3D11PixelShader
	{
	public:
		explicit D3D11SkyPixelShader(const ShaderCompileDesc& desc);
		virtual ~D3D11SkyPixelShader();

		virtual void				Release() override;

		virtual void				Apply(const ShaderPerObjParam* pPerObjParam = nullptr) override;

	protected:
		ShaderParam					m_SkyCubeTextureParam;
		ShaderParam					m_SkyCubeTextureSamplerParam;
	};

	class D3D11DirectionalLightingPixelShader : public D3D11PixelShader
	{
	public:
		explicit D3D11DirectionalLightingPixelShader(const ShaderCompileDesc& desc);
		virtual ~D3D11DirectionalLightingPixelShader();

		virtual void				Release() override;

		virtual void				Apply(const ShaderPerObjParam* pPerObjParam = nullptr) override;

	protected:
		ShaderParam					m_SkyCubeTextureParam;
		ShaderParam					m_SkyCubeTextureSamplerParam;
		ShaderParam					m_ShadowDepthTextureParam;
		ShaderParam					m_ShadowDepthTextureSamplerParam;
		ShaderParam					m_IblBrdfLutTextureParam;
		ShaderParam					m_IblBrdfLutTextureSamplerParam;
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
		virtual IShader*			GetModelVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc) override;

		virtual IShader*			GetDirectionLightingPixelShader(const ShaderCompileDesc& desc) override;
		virtual IShader*			GetSkyPixelShader(const ShaderCompileDesc& desc) override;

		virtual IShader*			GetMaterialShader(const ShaderCompileDesc& desc, const std::string& mtlName) override;

		void						ReleaseShader(D3D11Shader* pShader);

	private:
		ShaderHashMap				m_ShaderMap;
	};

	D3D11ShaderManager	g_D3D11ShaderManager;
	IShaderManager*		g_pIShaderManager = &g_D3D11ShaderManager;


	//=========================================================================
	//    Function Implementation
	//=========================================================================
	D3D11Shader::D3D11Shader(const ShaderCompileDesc& desc) :
		m_pCompiledCode(nullptr),
		m_pShaderReflector(nullptr),
		m_fileName(desc.fileName)
	{
		CompileFromFile(desc);
	}

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
			if (line == "#include \"MaterialShaders/Material.hlsl\"")
			{
				//FIXME!!! 这里需要修改为根据用户提供的MaterialShader文件来替换
				line = "#include \"MaterialShaders/" + desc.mtlShaderName + "\"";
			}
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

	ShaderParam* D3D11Shader::GetShaderParamPtrByName(const std::string& name)
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

	int D3D11Shader::GetShaderResourceViewSlotByName(const std::string& name)
	{
		auto srvIter = m_ShaderResourceViewMap.find(name);
		if (srvIter != m_ShaderResourceViewMap.end())
		{
			return srvIter->second;
		}
		else
		{
			return -1;
		}
	}

	int D3D11Shader::GetSamplerStateSlotByName(const std::string& name)
	{
		auto iter = m_SamplerStateMap.find(name);
		if (iter != m_SamplerStateMap.end())
		{
			return iter->second;
		}
		else
		{
			return -1;
		}
	}

	D3D11VertexShader::D3D11VertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc) :
		D3D11Shader(desc),
		m_pVS(nullptr),
		m_pInputLayout(nullptr),
		m_VertexType(vertexType)
	{
		g_pD3D11Device->CreateVertexShader(m_pCompiledCode->GetBufferPointer(), m_pCompiledCode->GetBufferSize(), nullptr, &m_pVS);

		m_pInputLayout = new D3D11InputLayout();
		m_pInputLayout->Init(m_VertexType, m_pCompiledCode->GetBufferPointer(), (uint32_t)m_pCompiledCode->GetBufferSize());
	}

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

	void D3D11VertexShader::Release()
	{
		g_D3D11ShaderManager.ReleaseShader(this);
	}

	void D3D11VertexShader::Apply(const ShaderPerObjParam* pPerObjParam/* = nullptr*/)
	{
		m_pInputLayout->SetToContext();

		g_pD3D11ImmediateContext->VSSetShader(m_pVS, nullptr, 0);
	}

	D3D11ModelVertexShader::D3D11ModelVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc) :
		D3D11VertexShader(vertexType, desc)
	{}

	D3D11ModelVertexShader::~D3D11ModelVertexShader()
	{}

	void D3D11ModelVertexShader::Release()
	{
		D3D11VertexShader::Release();
	}

	void D3D11ModelVertexShader::Apply(const ShaderPerObjParam* pPerObjParam/* = nullptr*/)
	{
		D3D11VertexShader::Apply(pPerObjParam);

		auto cbIter = m_ConstantBufferMap.find("ComVar_ConstantBuffer_PerObject");
		if (cbIter != m_ConstantBufferMap.end() && pPerObjParam->pPerObjectCB != nullptr)
		{
			uint32_t cbSlot = cbIter->second;
			ID3D11Buffer* pCB = (ID3D11Buffer*)(pPerObjParam->pPerObjectCB->GetResource());
			g_pD3D11ImmediateContext->VSSetConstantBuffers(cbSlot, 1, (ID3D11Buffer**)&pCB);
		}

		cbIter = m_ConstantBufferMap.find("ComVar_ConstantBuffer_BoneTransforms");
		if (cbIter != m_ConstantBufferMap.end() && pPerObjParam->pBoneTransformCB != nullptr)
		{
			uint32_t cbSlot = cbIter->second;
			ID3D11Buffer* pCB = (ID3D11Buffer*)(pPerObjParam->pBoneTransformCB->GetResource());
			g_pD3D11ImmediateContext->VSSetConstantBuffers(cbSlot, 1, (ID3D11Buffer**)&pCB);
		}

		IConstantBuffer* pViewParamCB = g_pIBufferManager->GetConstantBufferByName("ComVar_ConstantBuffer_ViewParam");
		if (pViewParamCB)
		{
			cbIter = m_ConstantBufferMap.find(pViewParamCB->GetName());
			if (cbIter != m_ConstantBufferMap.end())
			{
				uint32_t cbSlot = cbIter->second;
				ID3D11Buffer* pCB = (ID3D11Buffer*)(pViewParamCB->GetResource());
				g_pD3D11ImmediateContext->VSSetConstantBuffers(cbSlot, 1, (ID3D11Buffer**)&pCB);
			}
		}

		IConstantBuffer* pShadowMatrixConstantBuffer = g_pIBufferManager->GetConstantBufferByName("ComVar_ConstantBuffer_ShadowMatrixs");
		if (pShadowMatrixConstantBuffer)
		{
			cbIter = m_ConstantBufferMap.find(pShadowMatrixConstantBuffer->GetName());
			if (cbIter != m_ConstantBufferMap.end())
			{
				uint32_t cbSlot = cbIter->second;
				ID3D11Buffer* pCB = (ID3D11Buffer*)(pShadowMatrixConstantBuffer->GetResource());
				g_pD3D11ImmediateContext->VSSetConstantBuffers(cbSlot, 1, (ID3D11Buffer**)&pCB);
			}
		}	
	}

	D3D11PixelShader::D3D11PixelShader(const ShaderCompileDesc& desc) :
		D3D11Shader(desc),
		m_pPS(nullptr)
	{
		g_pD3D11Device->CreatePixelShader(m_pCompiledCode->GetBufferPointer(), m_pCompiledCode->GetBufferSize(), nullptr, &m_pPS);
	}

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

	void D3D11PixelShader::Release()
	{
		g_D3D11ShaderManager.ReleaseShader(this);
	}

	void D3D11PixelShader::Apply(const ShaderPerObjParam* pPerObjParam/* = nullptr*/)
	{
		g_pD3D11ImmediateContext->PSSetShader(m_pPS, nullptr, 0);
	}

	void D3D11PixelShader::ApplyMaterialParams(IMaterialInstance* pMtlIns)
	{
		if (!pMtlIns)
		{
			return;
		}

		uint32_t mtlParamNum = pMtlIns->GetMaterialParamNum();
		for (uint32_t i = 0; i < mtlParamNum; ++i)
		{
			MaterialParam& param = pMtlIns->GetMaterialParamByIndex(i);
			MATERIAL_PARAM_TYPE paramType = param.GetType();
			const std::string& paramName = param.GetName();
			switch (paramType)
			{
			case MATERIAL_PARAM_TYPE::UNKNOWN:
				break;
			case MATERIAL_PARAM_TYPE::SCALAR:
			{
				break;
			}
			case MATERIAL_PARAM_TYPE::VECTOR4:
			{
				break;
			}
			case MATERIAL_PARAM_TYPE::TEXTURE2D:
			{
				auto srvIter = m_ShaderResourceViewMap.find(paramName);
				if (srvIter == m_ShaderResourceViewMap.end())
				{
					return;
				}
				if (param.GetTexture2D())
				{
					ID3D11ShaderResourceView* pSRV = (ID3D11ShaderResourceView*)(param.GetTexture2D()->GetShaderResourceView());
					g_pD3D11ImmediateContext->PSSetShaderResources(srvIter->second, 1, &pSRV);
				}

				break;
			}
			case MATERIAL_PARAM_TYPE::SAMPLER:
			{
				auto samplerIter = m_SamplerStateMap.find(paramName);
				if (samplerIter == m_SamplerStateMap.end())
				{
					return;
				}
				ISamplerState* pSampler = param.GetSamplerState();
				if (pSampler)
				{
					pSampler->SetToPixelShader(samplerIter->second);
				}

				break;
			}
			default:
				break;
			}
		}
	}

	D3D11DirectionalLightingPixelShader::D3D11DirectionalLightingPixelShader(const ShaderCompileDesc& desc) :
		D3D11PixelShader(desc),
		m_SkyCubeTextureParam("ComVar_Texture_SkyCubeTexture", SHADER_PARAM_TYPE::TEXTURE),
		m_SkyCubeTextureSamplerParam("ComVar_Texture_SkyCubeTextureSampler", SHADER_PARAM_TYPE::SAMPLER),
		m_ShadowDepthTextureParam("ComVar_Texture_ShadowDepthTexture", SHADER_PARAM_TYPE::TEXTURE),
		m_ShadowDepthTextureSamplerParam("ComVar_Texture_ShadowDepthTextureSampler", SHADER_PARAM_TYPE::SAMPLER),
		m_IblBrdfLutTextureParam("ComVar_Texture_IblBrdfLutTexture", SHADER_PARAM_TYPE::TEXTURE),
		m_IblBrdfLutTextureSamplerParam("ComVar_Texture_IblBrdfLutTextureSampler", SHADER_PARAM_TYPE::SAMPLER)
		
	{
		m_ShaderParamMap.insert({ "ComVar_Texture_SkyCubeTexture", &m_SkyCubeTextureParam });
		m_ShaderParamMap.insert({ "ComVar_Texture_SkyCubeTextureSampler", &m_SkyCubeTextureSamplerParam });
		m_ShaderParamMap.insert({ "ComVar_Texture_ShadowDepthTexture", &m_ShadowDepthTextureParam });
		m_ShaderParamMap.insert({ "ComVar_Texture_ShadowDepthTextureSampler", &m_ShadowDepthTextureSamplerParam });
		m_ShaderParamMap.insert({ "ComVar_Texture_IblBrdfLutTexture", &m_IblBrdfLutTextureParam });
		m_ShaderParamMap.insert({ "ComVar_Texture_IblBrdfLutTextureSampler", &m_IblBrdfLutTextureSamplerParam });
	}

	D3D11DirectionalLightingPixelShader::~D3D11DirectionalLightingPixelShader()
	{}

	void D3D11DirectionalLightingPixelShader::Release()
	{
		D3D11PixelShader::Release();
	}

	void D3D11DirectionalLightingPixelShader::Apply(const ShaderPerObjParam* pPerObjParam/* = nullptr*/)
	{
		D3D11PixelShader::Apply(pPerObjParam);

		//LightingParams
		IConstantBuffer* pLightingParamsConstantBuffer = g_pIBufferManager->GetConstantBufferByName("ComVar_ConstantBuffer_LightingParam");
		if (pLightingParamsConstantBuffer)
		{
			auto cbIter = m_ConstantBufferMap.find(pLightingParamsConstantBuffer->GetName());
			if (cbIter != m_ConstantBufferMap.end())
			{
				uint32_t cbSlot = cbIter->second;
				ID3D11Buffer* pCB = (ID3D11Buffer*)(pLightingParamsConstantBuffer->GetResource());
				g_pD3D11ImmediateContext->PSSetConstantBuffers(cbSlot, 1, (ID3D11Buffer**)&pCB);
			}
		}

		//ShadowParams
		IConstantBuffer* pShadowParamBuffer = g_pIBufferManager->GetConstantBufferByName("ComVar_ConstantBuffer_ShadowParam");
		if (pShadowParamBuffer)
		{
			auto cbIter = m_ConstantBufferMap.find(pShadowParamBuffer->GetName());
			if (cbIter != m_ConstantBufferMap.end())
			{
				uint32_t cbSlot = cbIter->second;
				ID3D11Buffer* pCB = (ID3D11Buffer*)(pShadowParamBuffer->GetResource());
				g_pD3D11ImmediateContext->PSSetConstantBuffers(cbSlot, 1, (ID3D11Buffer**)&pCB);
			}
		}

		//SkyTexture
		auto srvIter = m_ShaderResourceViewMap.find(m_SkyCubeTextureParam.GetName());
		if (srvIter != m_ShaderResourceViewMap.end())
		{
			ID3D11ShaderResourceView* pSkySRV = (ID3D11ShaderResourceView*)(m_SkyCubeTextureParam.GetTexture()->GetShaderResourceView());
			g_pD3D11ImmediateContext->PSSetShaderResources(srvIter->second, 1, &pSkySRV);
		}

		auto samplerIter = m_SamplerStateMap.find(m_SkyCubeTextureSamplerParam.GetName());
		ISamplerState* pSkySampler = m_SkyCubeTextureSamplerParam.GetSampler();
		if (samplerIter != m_SamplerStateMap.end())
		{
			pSkySampler->SetToPixelShader(samplerIter->second);
		}

		//ShadowTexture
		srvIter = m_ShaderResourceViewMap.find(m_ShadowDepthTextureParam.GetName());
		if (srvIter != m_ShaderResourceViewMap.end())
		{
			ID3D11ShaderResourceView* pShadowDepthSRV = (ID3D11ShaderResourceView*)(m_ShadowDepthTextureParam.GetTexture()->GetShaderResourceView());
			g_pD3D11ImmediateContext->PSSetShaderResources(srvIter->second, 1, &pShadowDepthSRV);
		}

		samplerIter = m_SamplerStateMap.find(m_ShadowDepthTextureSamplerParam.GetName());
		ISamplerState* pShadowDepthSampler = m_ShadowDepthTextureSamplerParam.GetSampler();
		if (samplerIter != m_SamplerStateMap.end())
		{
			pShadowDepthSampler->SetToPixelShader(samplerIter->second);
		}

		//IBL
		srvIter = m_ShaderResourceViewMap.find(m_IblBrdfLutTextureParam.GetName());
		if (srvIter != m_ShaderResourceViewMap.end())
		{
			ID3D11ShaderResourceView* pIBLSRV = (ID3D11ShaderResourceView*)(m_IblBrdfLutTextureParam.GetTexture()->GetShaderResourceView());
			g_pD3D11ImmediateContext->PSSetShaderResources(srvIter->second, 1, &pIBLSRV);
		}

		samplerIter = m_SamplerStateMap.find(m_IblBrdfLutTextureSamplerParam.GetName());
		ISamplerState* pIBLSampler = m_IblBrdfLutTextureSamplerParam.GetSampler();
		if (samplerIter != m_SamplerStateMap.end())
		{
			pIBLSampler->SetToPixelShader(samplerIter->second);
		}
	}

	D3D11SkyPixelShader::D3D11SkyPixelShader(const ShaderCompileDesc& desc) :
		D3D11PixelShader(desc),
		m_SkyCubeTextureParam("ComVar_Texture_SkyCubeTexture", SHADER_PARAM_TYPE::TEXTURE),
		m_SkyCubeTextureSamplerParam("ComVar_Texture_SkyCubeTextureSampler", SHADER_PARAM_TYPE::SAMPLER)
	{
		m_ShaderParamMap.insert({ "ComVar_Texture_SkyCubeTexture", &m_SkyCubeTextureParam });
		m_ShaderParamMap.insert({ "ComVar_Texture_SkyCubeTextureSampler", &m_SkyCubeTextureSamplerParam });
	}

	D3D11SkyPixelShader::~D3D11SkyPixelShader()
	{}

	void D3D11SkyPixelShader::Release()
	{
		D3D11PixelShader::Release();
	}

	void D3D11SkyPixelShader::Apply(const ShaderPerObjParam* pPerObjParam/* = nullptr*/)
	{
		D3D11PixelShader::Apply(pPerObjParam);

		//SkyTexture
		auto srvIter = m_ShaderResourceViewMap.find(m_SkyCubeTextureParam.GetName());
		if (srvIter != m_ShaderResourceViewMap.end())
		{
			ID3D11ShaderResourceView* pSkySRV = (ID3D11ShaderResourceView*)(m_SkyCubeTextureParam.GetTexture()->GetShaderResourceView());
			g_pD3D11ImmediateContext->PSSetShaderResources(srvIter->second, 1, &pSkySRV);
		}

		auto samplerIter = m_SamplerStateMap.find(m_SkyCubeTextureSamplerParam.GetName());
		ISamplerState* pSkySampler = m_SkyCubeTextureSamplerParam.GetSampler();
		if (samplerIter != m_SamplerStateMap.end())
		{
			pSkySampler->SetToPixelShader(samplerIter->second);
		}
	}

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
			pVertexShader = new D3D11VertexShader(vertexType, desc);
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
			pPixelShader = new D3D11PixelShader(desc);
			m_ShaderMap.insert({ desc.fileName, pPixelShader });
		}

		return pPixelShader;
	}

	IShader* D3D11ShaderManager::GetModelVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc)
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
			pVertexShader = new D3D11ModelVertexShader(vertexType, desc);
			m_ShaderMap.insert({ desc.fileName, pVertexShader });
		}

		return pVertexShader;
	}

	IShader* D3D11ShaderManager::GetDirectionLightingPixelShader(const ShaderCompileDesc& desc)
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
			pPixelShader = new D3D11DirectionalLightingPixelShader(desc);
			m_ShaderMap.insert({ desc.fileName, pPixelShader });
		}

		return pPixelShader;
	}

	IShader* D3D11ShaderManager::GetSkyPixelShader(const ShaderCompileDesc& desc)
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
			pPixelShader = new D3D11SkyPixelShader(desc);
			m_ShaderMap.insert({ desc.fileName, pPixelShader });
		}

		return pPixelShader;
	}

	IShader* D3D11ShaderManager::GetMaterialShader(const ShaderCompileDesc& desc, const std::string& mtlName)
	{
		D3D11Shader* pPixelShader = nullptr;

		std::string mtlShaderName = desc.fileName + "_" + mtlName;
		auto shader = m_ShaderMap.find(mtlShaderName);
		if (shader != m_ShaderMap.end())
		{
			pPixelShader = shader->second;
			pPixelShader->AddRef();
		}
		else
		{
			pPixelShader = new D3D11DirectionalLightingPixelShader(desc);
			m_ShaderMap.insert({ mtlShaderName, pPixelShader });
		}

		return pPixelShader;
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