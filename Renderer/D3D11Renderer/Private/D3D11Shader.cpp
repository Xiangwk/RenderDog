////////////////////////////////////////
//RenderDog <¡¤,¡¤>
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

		struct ShadowDepthConstantData
		{
			Matrix4x4	viewMatrix;
			Matrix4x4	projMatrix;
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
		ShaderParam					m_WorldToViewMatrixParam;
		ShaderParam					m_ViewToClipMatrixParam;
		ShaderParam					m_WorldEyePostionParam;

		ShaderParam					m_ShadowWorldToViewMatrixParam;
		ShaderParam					m_ShadowViewToClipMatrixParam;
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

	class D3D11SkyPixelShader : public D3D11PixelShader
	{
	public:
		D3D11SkyPixelShader();
		virtual ~D3D11SkyPixelShader();

		virtual bool				Init() override;
		virtual void				Release() override;

		virtual ShaderParam*		GetShaderParamPtrByName(const std::string& name) override;

		virtual void				Apply() override;

	protected:
		ShaderParam					m_SkyCubeTextureParam;
		ShaderParam					m_SkyCubeTextureSamplerParam;
	};

	class D3D11DirectionalLightingPixelShader : public D3D11PixelShader
	{
	public:
		D3D11DirectionalLightingPixelShader();
		virtual ~D3D11DirectionalLightingPixelShader();

		virtual bool				Init() override;
		virtual void				Release() override;

		virtual ShaderParam*		GetShaderParamPtrByName(const std::string& name) override;

		virtual void				Apply() override;

	protected:
		ShaderParam					m_SkyCubeTextureParam;
		ShaderParam					m_SkyCubeTextureSamplerParam;
		ShaderParam					m_DiffuseTextureParam;
		ShaderParam					m_DiffuseTextureSamplerParam;
		ShaderParam					m_NormalTextureParam;
		ShaderParam					m_NormalTextureSamplerParam;
		ShaderParam					m_ShadowDepthTextureParam;
		ShaderParam					m_ShadowDepthTextureSamplerParam;

		ShaderParam					m_ShadowParam0;						//x: shadow offset, y: shadowMap Size
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

		virtual IShader*			GetDirectionLightingPixelShader(const ShaderCompileDesc& desc) override;
		virtual IShader*			GetSkyPixelShader(const ShaderCompileDesc& desc) override;

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
		m_WorldToViewMatrixParam("ComVar_Matrix_WorldToView", SHADER_PARAM_TYPE::MATRIX),
		m_ViewToClipMatrixParam("ComVar_Matrix_ViewToClip", SHADER_PARAM_TYPE::MATRIX),
		m_WorldEyePostionParam("ComVar_Vector_WorldEyePosition", SHADER_PARAM_TYPE::FLOAT_VECTOR),
		m_ShadowWorldToViewMatrixParam("ComVar_Matrix_ShadowView", SHADER_PARAM_TYPE::MATRIX),
		m_ShadowViewToClipMatrixParam("ComVar_Matrix_ShadowProjection", SHADER_PARAM_TYPE::MATRIX)
	{
		m_ShaderParamMap.insert({ "ComVar_Matrix_WorldToView", &m_WorldToViewMatrixParam });
		m_ShaderParamMap.insert({ "ComVar_Matrix_ViewToClip", &m_ViewToClipMatrixParam });
		m_ShaderParamMap.insert({ "ComVar_Vector_WorldEyePosition", &m_WorldEyePostionParam });
		m_ShaderParamMap.insert({ "ComVar_Matrix_ShadowView", &m_ShadowWorldToViewMatrixParam});
		m_ShaderParamMap.insert({ "ComVar_Matrix_ShadowProjection", &m_ShadowViewToClipMatrixParam });
	}

	D3D11StaticModelVertexShader::D3D11StaticModelVertexShader(VERTEX_TYPE vertexType) :
		D3D11VertexShader(vertexType),
		m_WorldToViewMatrixParam("ComVar_Matrix_WorldToView", SHADER_PARAM_TYPE::MATRIX),
		m_ViewToClipMatrixParam("ComVar_Matrix_ViewToClip", SHADER_PARAM_TYPE::MATRIX),
		m_WorldEyePostionParam("ComVar_Vector_WorldEyePosition", SHADER_PARAM_TYPE::FLOAT_VECTOR),
		m_ShadowWorldToViewMatrixParam("ComVar_Matrix_ShadowView", SHADER_PARAM_TYPE::MATRIX),
		m_ShadowViewToClipMatrixParam("ComVar_Matrix_ShadowProjection", SHADER_PARAM_TYPE::MATRIX)
	{
		m_ShaderParamMap.insert({ "ComVar_Matrix_WorldToView", &m_WorldToViewMatrixParam });
		m_ShaderParamMap.insert({ "ComVar_Matrix_ViewToClip", &m_ViewToClipMatrixParam });
		m_ShaderParamMap.insert({ "ComVar_Vector_WorldEyePosition", &m_WorldEyePostionParam });
		m_ShaderParamMap.insert({ "ComVar_Matrix_ShadowView", &m_ShadowWorldToViewMatrixParam });
		m_ShaderParamMap.insert({ "ComVar_Matrix_ShadowProjection", &m_ShadowViewToClipMatrixParam });
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

		IConstantBuffer* pGlobalConstantBuffer = g_pIBufferManager->GetConstantBufferByName("ComVar_ConstantBuffer_Global");
		if (!pGlobalConstantBuffer)
		{
			return;
		}

		GlobalConstantData globalCBData;
		globalCBData.viewMatrix = m_WorldToViewMatrixParam.GetMatrix4x4();
		globalCBData.projMatrix = m_ViewToClipMatrixParam.GetMatrix4x4();
		globalCBData.mainCameraWorldPos = m_WorldEyePostionParam.GetVector4();

		pGlobalConstantBuffer->Update(&globalCBData, sizeof(globalCBData));

		auto cbIter = m_ConstantBufferMap.find(pGlobalConstantBuffer->GetName());
		if (cbIter != m_ConstantBufferMap.end())
		{
			uint32_t cbSlot = cbIter->second;
			ID3D11Buffer* pCB = (ID3D11Buffer*)(pGlobalConstantBuffer->GetResource());
			g_pD3D11ImmediateContext->VSSetConstantBuffers(cbSlot, 1, (ID3D11Buffer**)&pCB);
		}

		IConstantBuffer* pShadowMatrixConstantBuffer = g_pIBufferManager->GetConstantBufferByName("ComVar_ConstantBuffer_ShadowMatrixs");
		if (!pShadowMatrixConstantBuffer)
		{
			return;
		}

		ShadowDepthConstantData shadowMatrixCBData;
		shadowMatrixCBData.viewMatrix = m_ShadowWorldToViewMatrixParam.GetMatrix4x4();
		shadowMatrixCBData.projMatrix = m_ShadowViewToClipMatrixParam.GetMatrix4x4();
		pShadowMatrixConstantBuffer->Update(&shadowMatrixCBData, sizeof(shadowMatrixCBData));

		cbIter = m_ConstantBufferMap.find(pShadowMatrixConstantBuffer->GetName());
		if (cbIter != m_ConstantBufferMap.end())
		{
			uint32_t cbSlot = cbIter->second;
			ID3D11Buffer* pCB = (ID3D11Buffer*)(pShadowMatrixConstantBuffer->GetResource());
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

	D3D11DirectionalLightingPixelShader::D3D11DirectionalLightingPixelShader() :
		m_SkyCubeTextureParam("ComVar_Texture_SkyCubeTexture", SHADER_PARAM_TYPE::TEXTURE),
		m_SkyCubeTextureSamplerParam("ComVar_Texture_SkyCubeTextureSampler", SHADER_PARAM_TYPE::SAMPLER),
		m_DiffuseTextureParam("DiffuseTexture", SHADER_PARAM_TYPE::TEXTURE),
		m_DiffuseTextureSamplerParam("DiffuseTextureSampler", SHADER_PARAM_TYPE::SAMPLER),
		m_NormalTextureParam("NormalTexture", SHADER_PARAM_TYPE::TEXTURE),
		m_NormalTextureSamplerParam("NormalTextureSampler", SHADER_PARAM_TYPE::SAMPLER),
		m_ShadowDepthTextureParam("ComVar_Texture_ShadowDepthTexture", SHADER_PARAM_TYPE::TEXTURE),
		m_ShadowDepthTextureSamplerParam("ComVar_Texture_ShadowDepthTextureSampler", SHADER_PARAM_TYPE::SAMPLER),
		m_ShadowParam0("ComVar_Vector_ShadowParam0", SHADER_PARAM_TYPE::FLOAT_VECTOR)
	{
		m_ShaderParamMap.insert({ "ComVar_Texture_SkyCubeTexture", &m_SkyCubeTextureParam });
		m_ShaderParamMap.insert({ "ComVar_Texture_SkyCubeTextureSampler", &m_SkyCubeTextureSamplerParam });
		m_ShaderParamMap.insert({ "DiffuseTexture", &m_DiffuseTextureParam });
		m_ShaderParamMap.insert({ "DiffuseTextureSampler", &m_DiffuseTextureSamplerParam });
		m_ShaderParamMap.insert({ "NormalTexture", &m_NormalTextureParam });
		m_ShaderParamMap.insert({ "NormalTextureSampler", &m_NormalTextureSamplerParam });
		m_ShaderParamMap.insert({ "ComVar_Texture_ShadowDepthTexture", &m_ShadowDepthTextureParam });
		m_ShaderParamMap.insert({ "ComVar_Texture_ShadowDepthTextureSampler", &m_ShadowDepthTextureSamplerParam });
		m_ShaderParamMap.insert({ "ComVar_Vector_ShadowParam0", &m_ShadowParam0 });
	}

	D3D11DirectionalLightingPixelShader::~D3D11DirectionalLightingPixelShader()
	{}

	bool D3D11DirectionalLightingPixelShader::Init()
	{
		return D3D11PixelShader::Init();
	}

	void D3D11DirectionalLightingPixelShader::Release()
	{
		D3D11PixelShader::Release();
	}

	ShaderParam* D3D11DirectionalLightingPixelShader::GetShaderParamPtrByName(const std::string& name)
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

	void D3D11DirectionalLightingPixelShader::Apply()
	{
		D3D11PixelShader::Apply();

		//SkyTexture
		auto srvIter = m_ShaderResourceViewMap.find(m_SkyCubeTextureParam.GetName());
		if (srvIter == m_ShaderResourceViewMap.end())
		{
			return;
		}
		ID3D11ShaderResourceView* pSkySRV = (ID3D11ShaderResourceView*)(m_SkyCubeTextureParam.GetTexture()->GetShaderResourceView());
		g_pD3D11ImmediateContext->PSSetShaderResources(srvIter->second, 1, &pSkySRV);

		auto samplerIter = m_SamplerStateMap.find(m_SkyCubeTextureSamplerParam.GetName());
		ISamplerState* pSkySampler = m_SkyCubeTextureSamplerParam.GetSampler();
		if (samplerIter == m_SamplerStateMap.end())
		{
			return;
		}
		pSkySampler->SetToPixelShader(samplerIter->second);

		//DiffuseTexture
		srvIter = m_ShaderResourceViewMap.find(m_DiffuseTextureParam.GetName());
		if (srvIter == m_ShaderResourceViewMap.end())
		{
			return;
		}
		ID3D11ShaderResourceView* pDiffSRV = (ID3D11ShaderResourceView*)(m_DiffuseTextureParam.GetTexture()->GetShaderResourceView());
		g_pD3D11ImmediateContext->PSSetShaderResources(srvIter->second, 1, &pDiffSRV);

		samplerIter = m_SamplerStateMap.find(m_DiffuseTextureSamplerParam.GetName());
		ISamplerState* pDiffSampler = m_DiffuseTextureSamplerParam.GetSampler();
		if (samplerIter == m_SamplerStateMap.end())
		{
			return;
		}
		pDiffSampler->SetToPixelShader(samplerIter->second);

		//NormalTexture
		srvIter = m_ShaderResourceViewMap.find(m_NormalTextureParam.GetName());
		if (srvIter == m_ShaderResourceViewMap.end())
		{
			return;
		}
		ID3D11ShaderResourceView* pNormSRV = (ID3D11ShaderResourceView*)(m_NormalTextureParam.GetTexture()->GetShaderResourceView());
		g_pD3D11ImmediateContext->PSSetShaderResources(srvIter->second, 1, &pNormSRV);

		samplerIter = m_SamplerStateMap.find(m_NormalTextureSamplerParam.GetName());
		ISamplerState* pNormSampler = m_NormalTextureSamplerParam.GetSampler();
		if (samplerIter == m_SamplerStateMap.end())
		{
			return;
		}
		pNormSampler->SetToPixelShader(samplerIter->second);

		//ShadowTexture
		srvIter = m_ShaderResourceViewMap.find(m_ShadowDepthTextureParam.GetName());
		if (srvIter == m_ShaderResourceViewMap.end())
		{
			return;
		}
		ID3D11ShaderResourceView* pShadowDepthSRV = (ID3D11ShaderResourceView*)(m_ShadowDepthTextureParam.GetTexture()->GetShaderResourceView());
		g_pD3D11ImmediateContext->PSSetShaderResources(srvIter->second, 1, &pShadowDepthSRV);

		samplerIter = m_SamplerStateMap.find(m_ShadowDepthTextureSamplerParam.GetName());
		ISamplerState* pShadowDepthSampler = m_ShadowDepthTextureSamplerParam.GetSampler();
		if (samplerIter == m_SamplerStateMap.end())
		{
			return;
		}
		pShadowDepthSampler->SetToPixelShader(samplerIter->second);

		IConstantBuffer* pShadowParamBuffer = g_pIBufferManager->GetConstantBufferByName("ComVar_ConstantBuffer_ShadowParam");
		if (!pShadowParamBuffer)
		{
			return;
		}
		Vector4 shadowParam0 = m_ShadowParam0.GetVector4();
		pShadowParamBuffer->Update(&shadowParam0, sizeof(shadowParam0));

		auto cbIter = m_ConstantBufferMap.find(pShadowParamBuffer->GetName());
		if (cbIter != m_ConstantBufferMap.end())
		{
			uint32_t cbSlot = cbIter->second;
			ID3D11Buffer* pCB = (ID3D11Buffer*)(pShadowParamBuffer->GetResource());
			g_pD3D11ImmediateContext->PSSetConstantBuffers(cbSlot, 1, (ID3D11Buffer**)&pCB);
		}
	}

	D3D11SkyPixelShader::D3D11SkyPixelShader() :
		m_SkyCubeTextureParam("ComVar_Texture_SkyCubeTexture", SHADER_PARAM_TYPE::TEXTURE),
		m_SkyCubeTextureSamplerParam("ComVar_Texture_SkyCubeTextureSampler", SHADER_PARAM_TYPE::SAMPLER)
	{
		m_ShaderParamMap.insert({ "ComVar_Texture_SkyCubeTexture", &m_SkyCubeTextureParam });
		m_ShaderParamMap.insert({ "ComVar_Texture_SkyCubeTextureSampler", &m_SkyCubeTextureSamplerParam });
	}

	D3D11SkyPixelShader::~D3D11SkyPixelShader()
	{}

	bool D3D11SkyPixelShader::Init()
	{
		return D3D11PixelShader::Init();
	}

	void D3D11SkyPixelShader::Release()
	{
		D3D11PixelShader::Release();
	}

	ShaderParam* D3D11SkyPixelShader::GetShaderParamPtrByName(const std::string& name)
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

	void D3D11SkyPixelShader::Apply()
	{
		D3D11PixelShader::Apply();

		//SkyTexture
		auto srvIter = m_ShaderResourceViewMap.find(m_SkyCubeTextureParam.GetName());
		if (srvIter == m_ShaderResourceViewMap.end())
		{
			return;
		}
		ID3D11ShaderResourceView* pSkySRV = (ID3D11ShaderResourceView*)(m_SkyCubeTextureParam.GetTexture()->GetShaderResourceView());
		g_pD3D11ImmediateContext->PSSetShaderResources(srvIter->second, 1, &pSkySRV);

		auto samplerIter = m_SamplerStateMap.find(m_SkyCubeTextureSamplerParam.GetName());
		ISamplerState* pSkySampler = m_SkyCubeTextureSamplerParam.GetSampler();
		if (samplerIter == m_SamplerStateMap.end())
		{
			return;
		}
		pSkySampler->SetToPixelShader(samplerIter->second);
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
			pPixelShader = new D3D11DirectionalLightingPixelShader();
			pPixelShader->CompileFromFile(desc);
			pPixelShader->Init();

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
			pPixelShader = new D3D11SkyPixelShader();
			pPixelShader->CompileFromFile(desc);
			pPixelShader->Init();

			m_ShaderMap.insert({ desc.fileName, pPixelShader });
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