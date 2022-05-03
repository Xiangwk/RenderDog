////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SRTexture.cpp
//Written by Xiang Weikang
//Desc: Software Renderer Texture
////////////////////////////////////////

#include "Texture.h"
#include "Vector.h"
#include "SoftwareRender3D.h"
#include "SoftwareRenderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "StbImage/stb_image.h"

namespace RenderDog
{
	//==================================================
	//		Texture2D
	//==================================================

	class SRTexture2D : public ITexture2D
	{
	public:
		SRTexture2D(const TextureDesc& desc);
		virtual ~SRTexture2D();

		virtual bool				LoadFromFile(const std::wstring& filePath) override;

		virtual void				Release() override;

		virtual void*				GetRenderTargetView() override { return (void*)m_pRTV; }
		virtual void*				GetShaderResourceView() override { return (void*)m_pSRV; }

	private:
		ISRTexture2D*				m_pTexture2D;
		ISRRenderTargetView*		m_pRTV;
		ISRShaderResourceView*		m_pSRV;
	};

	SRTexture2D::SRTexture2D(const TextureDesc& desc) :
		m_pTexture2D(nullptr),
		m_pRTV(nullptr),
		m_pSRV(nullptr)
	{
		//TODO!!!
	}

	SRTexture2D::~SRTexture2D()
	{
		if (m_pTexture2D)
		{
			m_pTexture2D->Release();
			m_pTexture2D = nullptr;
		}

		if (m_pRTV)
		{
			m_pRTV->Release();
			m_pRTV = nullptr;
		}

		if (m_pSRV)
		{
			m_pSRV->Release();
			m_pSRV = nullptr;
		}
	}

	bool SRTexture2D::LoadFromFile(const std::wstring& filePath)
	{
		uint32_t nTexWidth = 0;
		uint32_t nTexHeight = 0;
		int nChannels = 0;
		
		std::string tempFilePath;
		int len = WideCharToMultiByte(CP_ACP, 0, filePath.c_str(), filePath.size(), nullptr, 0, nullptr, nullptr);
		char* buffer = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, filePath.c_str(), filePath.size(), buffer, len, nullptr, nullptr);
		buffer[len] = '\0';
		tempFilePath.append(buffer);
		delete [] buffer;

		unsigned char* pTexData = stbi_load(tempFilePath.c_str(), (int*)&nTexWidth, (int*)&nTexHeight, &nChannels, 0);
		if (!pTexData)
		{
			return false;
		}

		SRTexture2DDesc tex2DDesc;
		tex2DDesc.width = nTexWidth;
		tex2DDesc.height = nTexHeight;
		if (nChannels == 3 || nChannels == 4)
		{
			tex2DDesc.format = SR_FORMAT::R32G32B32A32_FLOAT;
		}

		Vector4* pColor = new Vector4[nTexWidth * nTexHeight];
		if (!pColor)
		{
			return false;
		}

		unsigned char* pSrc = pTexData;
		for (uint32_t row = 0; row < nTexHeight; ++row)
		{
			pSrc = pTexData + row * nChannels * nTexWidth;
			for (uint32_t col = 0; col < nTexWidth; ++col)
			{
				float r = pSrc[0] / 255.0f;
				float g = pSrc[1] / 255.0f;
				float b = pSrc[2] / 255.0f;
				float a = 0.0f;
				if (nChannels == 4)
				{
					a = pSrc[3] / 255.0f;
				}
				else
				{
					a = 1.0f;
				}

				Vector4 fColor = { r, g, b, a };
				pColor[row * nTexWidth + col] = fColor;

				pSrc += nChannels;
			}
		}

		stbi_image_free(pTexData);

		SRSubResourceData initData;
		initData.pSysMem = pColor;
		initData.sysMemPitch = nTexWidth * nTexHeight * sizeof(Vector4);
		if (!g_pSRDevice->CreateTexture2D(&tex2DDesc, &initData, &m_pTexture2D))
		{
			return false;
		}

		SRShaderResourceViewDesc srvDesc;
		srvDesc.format = tex2DDesc.format;
		srvDesc.viewDimension = SR_SRV_DIMENSION::TEXTURE2D;
		if (!g_pSRDevice->CreateShaderResourceView(m_pTexture2D, &srvDesc, &m_pSRV))
		{
			return false;
		}

		delete[] pColor;

		return true;
	}

	void SRTexture2D::Release()
	{

	}

	//==================================================
	//		Texture Manager
	//==================================================

	class SRTextureManager : public ITextureManager
	{
	public:
		SRTextureManager() = default;
		virtual ~SRTextureManager() = default;

		virtual ITexture2D* CreateTexture2D(const TextureDesc& desc) override;
		virtual void		ReleaseTexture(ITexture* pTexture) override;
	};

	SRTextureManager g_SRTextureManager;
	ITextureManager* g_pITextureManager = &g_SRTextureManager;

	ITexture2D* SRTextureManager::CreateTexture2D(const TextureDesc& desc)
	{
		ITexture2D* pTexture = new SRTexture2D(desc);
		pTexture->AddRef();

		return pTexture;
	}

	void SRTextureManager::ReleaseTexture(ITexture* pTexture)
	{
		pTexture->SubRef();
	}

	//==================================================
	//		SamplerState
	//==================================================

	class SRSamplerState : public ISamplerState
	{
	public:
		SRSamplerState();
		virtual ~SRSamplerState();

		virtual bool Init(const SamplerDesc& desc) override;
		virtual void Release() override;

		virtual void SetToPixelShader(uint32_t startSlot) override;

	private:
		ISRSamplerState* m_pSamplerState;
	};

	SRSamplerState::SRSamplerState() :
		m_pSamplerState(nullptr)
	{}

	SRSamplerState::~SRSamplerState()
	{}

	bool SRSamplerState::Init(const SamplerDesc& desc)
	{
		SRSamplerDesc samplerDesc = {};
		if (desc.filterMode == SamplerFilterMode::RD_SAMPLER_FILTER_POINT)
		{
			samplerDesc.filter = SR_FILTER::POINT;
		}
		else if (desc.filterMode == SamplerFilterMode::RD_SAMPLER_FILTER_LINEAR)
		{
			samplerDesc.filter = SR_FILTER::BILINEAR;
		}

		if (!g_pSRDevice->CreateSamplerState(&samplerDesc, &m_pSamplerState))
		{
			return false;
		}

		return true;
	}

	void SRSamplerState::Release()
	{
		if (m_pSamplerState)
		{
			m_pSamplerState->Release();
			m_pSamplerState = nullptr;
		}
	}

	void SRSamplerState::SetToPixelShader(uint32_t startSlot)
	{
		g_pSRImmediateContext->PSSetSampler(startSlot, &m_pSamplerState);
	}

	//==================================================
	//		SamplerState Manager
	//==================================================

	class SRSamplerStateManager : public ISamplerStateManager
	{
	public:
		SRSamplerStateManager() = default;
		virtual ~SRSamplerStateManager() = default;

		virtual ISamplerState* CreateSamplerState() override;
		virtual void			ReleaseSamplerState(ISamplerState* pSampler) override;
	};

	SRSamplerStateManager g_SRSamplerStateManager;
	ISamplerStateManager* g_pISamplerStateManager = &g_SRSamplerStateManager;

	ISamplerState* SRSamplerStateManager::CreateSamplerState()
	{
		ISamplerState* pSampler = new SRSamplerState();

		return pSampler;
	}

	void SRSamplerStateManager::ReleaseSamplerState(ISamplerState* pSampler)
	{
		if (pSampler)
		{
			delete pSampler;
			pSampler = nullptr;
		}
	}
}// RenderDog