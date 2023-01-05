////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SRShader.cpp
//Written by Xiang Weikang
//Desc: Software Renderer Buffer
////////////////////////////////////////

#include "Shader.h"
#include "RefCntObject.h"
#include "SoftwareRender3D.h"
#include "SoftwareRenderer.h"
#include "Buffer.h"
#include "Texture.h"

#include <unordered_map>

namespace RenderDog
{
	class SRShader : public IShader, public RefCntObject
	{
	public:
		explicit SRShader(const ShaderCompileDesc& desc);
		virtual ~SRShader() = default;

		virtual const std::string&						GetFileName() const { return m_fileName; }

		virtual ShaderParam*							GetShaderParamPtrByName(const std::string& name) override;
		virtual int										GetShaderResourceViewSlotByName(const std::string& name) override { return -1; }
		virtual int										GetSamplerStateSlotByName(const std::string& name) override { return -1; }

	protected:
		std::string										m_fileName;

		std::unordered_map<std::string, ShaderParam*>	m_ShaderParamMap;
	};

	//=========================================================================
	//    VertexShader
	//=========================================================================
	class SRVertexShader : public SRShader
	{
	public:
		explicit SRVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc);
		virtual ~SRVertexShader();

		virtual void				Release() override;

		virtual void				Apply(const ShaderPerObjParam* pPerObjParam = nullptr) override;

		virtual void				ApplyMaterialParams(IMaterialInstance* pMtlIns) override {};

	private:
		ISRVertexShader*			m_pVS;
	};

	//=========================================================================
	//    PixelShader
	//=========================================================================
	class SRPixelShader : public SRShader
	{
	public:
		explicit SRPixelShader(const ShaderCompileDesc& desc);
		virtual ~SRPixelShader();

		virtual void				Release() override;

		virtual void				Apply(const ShaderPerObjParam* pPerObjParam = nullptr) override;

		virtual void				ApplyMaterialParams(IMaterialInstance* pMtlIns) override {};

	private:
		ISRPixelShader*				m_pPS;

		ShaderParam					m_DiffuseTextureParam;
		ShaderParam					m_DiffuseTextureSamplerParam;
		ShaderParam					m_NormalTextureParam;
		ShaderParam					m_NormalTextureSamplerParam;
	};

	//=========================================================================
	//    ShaderManager
	//=========================================================================
	class SRShaderManager : public IShaderManager
	{
	private:
		typedef std::unordered_map<std::string, SRShader*> ShaderHashMap;

	public:
		SRShaderManager() = default;
		virtual ~SRShaderManager() = default;

		virtual IShader*			GetVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc) override;
		virtual IShader*			GetPixelShader(const ShaderCompileDesc& desc) override;

		virtual IShader*			GetModelVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc) override;

		virtual IShader*			GetDirectionLightingPixelShader(const ShaderCompileDesc& desc) override;
		virtual IShader*			GetSkyPixelShader(const ShaderCompileDesc& desc) override;

		virtual IShader*			GetMaterialShader(const ShaderCompileDesc& desc, const std::string& mtlName) override;

		void						ReleaseShader(SRShader* pShader);

	private:
		ShaderHashMap				m_ShaderMap;
	};

	SRShaderManager g_SRShaderManager;
	IShaderManager* g_pIShaderManager = &g_SRShaderManager;


	//=========================================================================
	//		Function Implementation
	//=========================================================================
	SRShader::SRShader(const ShaderCompileDesc& desc) :
		m_fileName(desc.fileName)
	{}

	ShaderParam* SRShader::GetShaderParamPtrByName(const std::string& name)
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


	SRVertexShader::SRVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc) :
		SRShader(desc),
		m_pVS(nullptr)
	{
		g_pSRDevice->CreateVertexShader(&m_pVS);
	}

	SRVertexShader::~SRVertexShader()
	{
		if (m_pVS)
		{
			m_pVS->Release();
			m_pVS = nullptr;
		}
	}

	void SRVertexShader::Release()
	{
		g_SRShaderManager.ReleaseShader(this);
	}

	void SRVertexShader::Apply(const ShaderPerObjParam* pPerObjParam /* = nullptr */)
	{
		g_pSRImmediateContext->VSSetShader(m_pVS);

		if (pPerObjParam->pPerObjectCB != nullptr)
		{
			ISRBuffer* pPerObjCB = (ISRBuffer*)(pPerObjParam->pPerObjectCB->GetResource());
			g_pSRImmediateContext->VSSetConstantBuffer(1, &pPerObjCB);
		}

		IConstantBuffer* pViewParamCB = g_pIBufferManager->GetConstantBufferByName("ComVar_ConstantBuffer_ViewParam");
		ISRBuffer* pGlobalCB = (ISRBuffer*)(pViewParamCB->GetResource());
		g_pSRImmediateContext->VSSetConstantBuffer(0, &pGlobalCB);
	}


	SRPixelShader::SRPixelShader(const ShaderCompileDesc& desc) :
		SRShader(desc),
		m_pPS(nullptr),
		m_DiffuseTextureParam("LocVar_Material_DiffuseTexture", SHADER_PARAM_TYPE::TEXTURE),
		m_DiffuseTextureSamplerParam("LocVar_Material_DiffuseTextureSampler", SHADER_PARAM_TYPE::SAMPLER),
		m_NormalTextureParam("LocVar_Material_NormalTexture", SHADER_PARAM_TYPE::TEXTURE),
		m_NormalTextureSamplerParam("LocVar_Material_NormalTextureSampler", SHADER_PARAM_TYPE::SAMPLER)
	{
		g_pSRDevice->CreatePixelShader(&m_pPS);

		m_ShaderParamMap.insert({ "LocVar_Material_DiffuseTexture", &m_DiffuseTextureParam });
		m_ShaderParamMap.insert({ "LocVar_Material_DiffuseTextureSampler", &m_DiffuseTextureSamplerParam });
		m_ShaderParamMap.insert({ "LocVar_Material_NormalTexture", &m_NormalTextureParam });
		m_ShaderParamMap.insert({ "LocVar_Material_NormalTextureSampler", &m_NormalTextureSamplerParam });
	}

	SRPixelShader::~SRPixelShader()
	{
		if (m_pPS)
		{
			m_pPS->Release();
			m_pPS = nullptr;
		}
	}

	void SRPixelShader::Release()
	{
		g_SRShaderManager.ReleaseShader(this);
	}

	void SRPixelShader::Apply(const ShaderPerObjParam* pPerObjParam /* = nullptr */)
	{
		g_pSRImmediateContext->PSSetShader(m_pPS);

		IConstantBuffer* pLightingParamsConstantBuffer = g_pIBufferManager->GetConstantBufferByName("ComVar_ConstantBuffer_LightingParam");
		ISRBuffer* pLightingCB = (ISRBuffer*)(pLightingParamsConstantBuffer->GetResource());
		g_pSRImmediateContext->PSSetConstantBuffer(0, &pLightingCB);

		ISRShaderResourceView* pSRV = (ISRShaderResourceView*)(m_NormalTextureParam.GetTexture()->GetShaderResourceView());
		g_pSRImmediateContext->PSSetShaderResource(&pSRV);

		m_NormalTextureSamplerParam.GetSampler()->SetToPixelShader(0);
	}
	

	IShader* SRShaderManager::GetVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc)
	{
		SRShader* pVertexShader = nullptr;

		auto shader = m_ShaderMap.find(desc.fileName);
		if (shader != m_ShaderMap.end())
		{
			pVertexShader = shader->second;
			pVertexShader->AddRef();
		}
		else
		{
			pVertexShader = new SRVertexShader(vertexType, desc);
			m_ShaderMap.insert({ desc.fileName, pVertexShader });
		}

		return pVertexShader;
	}

	IShader* SRShaderManager::GetPixelShader(const ShaderCompileDesc& desc)
	{
		SRShader* pPixelShader = nullptr;

		auto shader = m_ShaderMap.find(desc.fileName);
		if (shader != m_ShaderMap.end())
		{
			pPixelShader = shader->second;
			pPixelShader->AddRef();
		}
		else
		{
			pPixelShader = new SRPixelShader(desc);
			m_ShaderMap.insert({ desc.fileName, pPixelShader });
		}

		return pPixelShader;
	}

	IShader* SRShaderManager::GetModelVertexShader(VERTEX_TYPE vertexType, const ShaderCompileDesc& desc)
	{
		//TODO:
		return GetVertexShader(vertexType, desc);
	}

	IShader* SRShaderManager::GetDirectionLightingPixelShader(const ShaderCompileDesc& desc)
	{
		//TODO:
		return GetPixelShader(desc);
	}

	IShader* SRShaderManager::GetSkyPixelShader(const ShaderCompileDesc& desc)
	{
		//TODO:
		return nullptr;
	}

	IShader* SRShaderManager::GetMaterialShader(const ShaderCompileDesc& desc, const std::string& mtlName)
	{
		//TODO:
		return nullptr;
	}

	void SRShaderManager::ReleaseShader(SRShader* pShader)
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