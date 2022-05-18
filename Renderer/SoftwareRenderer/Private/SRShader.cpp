////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SRShader.cpp
//Written by Xiang Weikang
//Desc: Software Renderer Buffer
////////////////////////////////////////

#include "Shader.h"
#include "SoftwareRender3D.h"
#include "SoftwareRenderer.h"

namespace RenderDog
{
	class SRShader : public IShader
	{
	public:
		SRShader() = default;
		virtual ~SRShader() = default;

		virtual void			CompileFromFile(const std::string& fileName,
												const ShaderMacro* shaderMacros,
												const std::string& entryPoint,
												const std::string& target,
												unsigned int compileFlag) override {}

		virtual void*			GetCompiledCode() const override { return nullptr; }
		virtual uint32_t		GetCompiledCodeSize() const override { return 0; }
	};

	//=========================================================================
	//    VertexShader
	//=========================================================================

	class SRVertexShader : public SRShader
	{
	public:
		SRVertexShader();
		virtual ~SRVertexShader();

		virtual bool			Init() override;
		virtual void			Release() override;

		virtual void			SetToContext() override;

	private:
		ISRVertexShader*		m_pVS;
	};

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

	//=========================================================================
	//    PixelShader
	//=========================================================================

	class SRPixelShader : public SRShader
	{
	public:
		SRPixelShader();
		virtual ~SRPixelShader();

		virtual bool			Init() override;
		virtual void			Release() override;

		virtual void			SetToContext() override;

	private:
		ISRPixelShader*			m_pPS;
	};

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

	//=========================================================================
	//    ShaderManager
	//=========================================================================

	class SRShaderManager : public IShaderManager
	{
	public:
		SRShaderManager() = default;
		virtual ~SRShaderManager() = default;

		virtual IShader* CreateVertexShader(VERTEX_TYPE vertexType) override;
		virtual IShader* CreatePixelShader() override;
	};

	SRShaderManager g_SRShaderManager;
	IShaderManager* g_pIShaderManager = &g_SRShaderManager;

	IShader* SRShaderManager::CreateVertexShader(VERTEX_TYPE vertexType)
	{
		IShader* pVertexShader = new SRVertexShader();

		return pVertexShader;
	}

	IShader* SRShaderManager::CreatePixelShader()
	{
		IShader* pPixelShader = new SRPixelShader();

		return pPixelShader;
	}

}// namespace RenderDog