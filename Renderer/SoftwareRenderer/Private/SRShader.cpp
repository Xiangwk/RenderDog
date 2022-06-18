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

#include <unordered_map>

namespace RenderDog
{
	class SRShader : public IShader, public RefCntObject
	{
	public:
		SRShader() = default;
		virtual ~SRShader() = default;

		virtual const std::string&	GetFileName() const { return m_fileName; }

	protected:
		std::string					m_fileName;
	};

	//=========================================================================
	//    VertexShader
	//=========================================================================
	class SRVertexShader : public SRShader
	{
	public:
		SRVertexShader();
		virtual ~SRVertexShader();

		virtual bool				Init() override;
		virtual void				Release() override;

		virtual void				SetToContext() override;

	private:
		ISRVertexShader*			m_pVS;
	};

	//=========================================================================
	//    PixelShader
	//=========================================================================
	class SRPixelShader : public SRShader
	{
	public:
		SRPixelShader();
		virtual ~SRPixelShader();

		virtual bool				Init() override;
		virtual void				Release() override;

		virtual void				SetToContext() override;

	private:
		ISRPixelShader* m_pPS;
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

		void						ReleaseShader(SRShader* pShader);

	private:
		ShaderHashMap				m_ShaderMap;
	};

	SRShaderManager g_SRShaderManager;
	IShaderManager* g_pIShaderManager = &g_SRShaderManager;


	//=========================================================================
	//		Function Implementation
	//=========================================================================
	SRVertexShader::SRVertexShader() :
		m_pVS(nullptr)
	{}

	SRVertexShader::~SRVertexShader()
	{}

	bool SRVertexShader::Init()
	{
		if (!g_pSRDevice->CreateVertexShader(&m_pVS))
		{
			return false;
		}

		return true;
	}

	void SRVertexShader::Release()
	{
		if (m_pVS)
		{
			m_pVS->Release();
			m_pVS = nullptr;
		}
	}

	void SRVertexShader::SetToContext()
	{
		g_pSRImmediateContext->VSSetShader(m_pVS);
	}


	SRPixelShader::SRPixelShader() :
		m_pPS(nullptr)
	{}

	SRPixelShader::~SRPixelShader()
	{}

	bool SRPixelShader::Init()
	{
		if (!g_pSRDevice->CreatePixelShader(&m_pPS))
		{
			return false;
		}

		return true;
	}

	void SRPixelShader::Release()
	{
		if (m_pPS)
		{
			m_pPS->Release();
			m_pPS = nullptr;
		}
	}

	void SRPixelShader::SetToContext()
	{
		g_pSRImmediateContext->PSSetShader(m_pPS);
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
			pVertexShader = new SRVertexShader();
			pVertexShader->Init();

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
			pPixelShader = new SRPixelShader();
			pPixelShader->Init();

			m_ShaderMap.insert({ desc.fileName, pPixelShader });
		}

		return pPixelShader;
	}

}// namespace RenderDog