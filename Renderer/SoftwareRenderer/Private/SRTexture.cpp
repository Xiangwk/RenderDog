////////////////////////////////////////
//RenderDog <·,·>
//FileName: SRTexture.cpp
//Written by Xiang Weikang
//Desc: Software Renderer Texture
////////////////////////////////////////

#include "Texture.h"
#include "RefCntObject.h"
#include "Vector.h"
#include "SoftwareRender3D.h"
#include "SoftwareRenderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "StbImage/stb_image.h"

#include <unordered_map>

namespace RenderDog
{
	//==================================================
	//		Texture2D
	//==================================================
	class SRTexture2D : public ITexture2D, public RefCntObject
	{
	public:
		SRTexture2D();
		explicit SRTexture2D(const TextureDesc& desc);
		virtual ~SRTexture2D();
		
		virtual void				Release() override;

		virtual const std::wstring& GetName() const override { return m_Name; }

		virtual void*				GetRenderTargetView() override { return (void*)m_pRTV; }
		virtual void*				GetDepthStencilView() override { return (void*)m_pDSV; }
		virtual void*				GetShaderResourceView() override { return (void*)m_pSRV; }

		bool						LoadFromFile(const std::wstring& filePath);

	private:
		std::wstring				m_Name;

		ISRTexture2D*				m_pTexture2D;
		ISRRenderTargetView*		m_pRTV;
		ISRDepthStencilView*		m_pDSV;
		ISRShaderResourceView*		m_pSRV;
	};

	//==================================================
	//		Texture Manager
	//==================================================
	class SRTextureManager : public ITextureManager
	{
	private:
		typedef std::unordered_map<std::wstring, ITexture*> TextureMap;

	public:
		SRTextureManager() = default;
		virtual ~SRTextureManager() = default;

		virtual ITexture2D*			CreateTexture2D(const std::wstring& filePath) override;
		virtual ITexture2D*			GetTexture2D(const TextureDesc& desc) override;

		void						ReleaseTexture2D(SRTexture2D* pTexture);

	private:
		TextureMap					m_TextureMap;
	};

	SRTextureManager g_SRTextureManager;
	ITextureManager* g_pITextureManager = &g_SRTextureManager;


	//==================================================
	//		Function Implementation
	//==================================================
	SRTexture2D::SRTexture2D() :
		RefCntObject(),
		m_Name(L""),
		m_pTexture2D(nullptr),
		m_pRTV(nullptr),
		m_pDSV(nullptr),
		m_pSRV(nullptr)
	{}

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
		int len = WideCharToMultiByte(CP_ACP, 0, filePath.c_str(), (int)filePath.size(), nullptr, 0, nullptr, nullptr);
		char* buffer = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, filePath.c_str(), (int)filePath.size(), buffer, len, nullptr, nullptr);
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
		initData.pSysMem		= pColor;
		initData.sysMemPitch	= nTexWidth * nTexHeight * sizeof(Vector4);
		if (!g_pSRDevice->CreateTexture2D(&tex2DDesc, &initData, &m_pTexture2D))
		{
			return false;
		}

		SRShaderResourceViewDesc srvDesc;
		srvDesc.format			= tex2DDesc.format;
		srvDesc.viewDimension	= SR_SRV_DIMENSION::TEXTURE2D;
		if (!g_pSRDevice->CreateShaderResourceView(m_pTexture2D, &srvDesc, &m_pSRV))
		{
			return false;
		}

		delete[] pColor;

		m_Name = filePath;

		return true;
	}

	void SRTexture2D::Release()
	{
		g_SRTextureManager.ReleaseTexture2D(this);
	}
	
	ITexture2D* SRTextureManager::CreateTexture2D(const std::wstring& filePath)
	{
		SRTexture2D* pTexture = nullptr;

		auto texture = m_TextureMap.find(filePath);
		if (texture != m_TextureMap.end())
		{
			//NOTE!!! 这里用强转是否合适？
			pTexture = (SRTexture2D*)(texture->second);
			pTexture->AddRef();
		}
		else
		{
			pTexture = new SRTexture2D();
			if (pTexture)
			{
				pTexture->LoadFromFile(filePath);
			}

			m_TextureMap.insert({ filePath, pTexture });
		}

		return pTexture;
	}

	ITexture2D* SRTextureManager::GetTexture2D(const TextureDesc& desc)
	{
		SRTexture2D* pTexture = nullptr;

		auto texture = m_TextureMap.find(desc.name);
		if (texture != m_TextureMap.end())
		{
			//NOTE!!! 这里用强转是否合适？
			pTexture = (SRTexture2D*)(texture->second);
			pTexture->AddRef();
		}
		else
		{
			pTexture = new SRTexture2D(desc);
		}

		return pTexture;
	}

	void SRTextureManager::ReleaseTexture2D(SRTexture2D* pTexture)
	{
		if (pTexture)
		{
			std::wstring texName = pTexture->GetName();
			if (pTexture->SubRef() == 0)
			{
				m_TextureMap.erase(texName);
			}
		}
	}

	//==================================================
	//		SamplerState
	//==================================================
	class SRSamplerState : public ISamplerState
	{
	public:
		SRSamplerState();
		SRSamplerState(const SamplerDesc& desc);
		virtual ~SRSamplerState();

		virtual void					Release() override;

		virtual const SamplerDesc&		GetDesc() const override { return m_Desc; }
		
		virtual void					SetToVertexShader(uint32_t startSlot) override;
		virtual void					SetToPixelShader(uint32_t startSlot) override;

	private:
		ISRSamplerState*				m_pSamplerState;

		SamplerDesc						m_Desc;
	};

	//==================================================
	//		SamplerState Manager
	//==================================================
	class SRSamplerStateManager : public ISamplerStateManager
	{
	public:
		SRSamplerStateManager() = default;
		virtual ~SRSamplerStateManager() = default;

		virtual ISamplerState*	CreateSamplerState(const SamplerDesc& desc) override;

		void					ReleaseSamplerState(SRSamplerState* pSampler);
	};

	SRSamplerStateManager	g_SRSamplerStateManager;
	ISamplerStateManager*	g_pISamplerStateManager = &g_SRSamplerStateManager;


	//==================================================
	//		Function Implementation
	//==================================================
	SRSamplerState::SRSamplerState() :
		m_pSamplerState(nullptr),
		m_Desc()
	{}

	SRSamplerState::SRSamplerState(const SamplerDesc& desc):
		m_pSamplerState(nullptr),
		m_Desc(desc)
	{
		SRSamplerDesc samplerDesc = {};
		if (desc.filterMode == SAMPLER_FILTER::POINT)
		{
			samplerDesc.filter = SR_FILTER::POINT;
		}
		else if (desc.filterMode == SAMPLER_FILTER::LINEAR)
		{
			samplerDesc.filter = SR_FILTER::BILINEAR;
		}

		g_pSRDevice->CreateSamplerState(&samplerDesc, &m_pSamplerState);
	}

	SRSamplerState::~SRSamplerState()
	{}

	void SRSamplerState::Release()
	{
		if (m_pSamplerState)
		{
			m_pSamplerState->Release();
			m_pSamplerState = nullptr;
		}
	}

	void SRSamplerState::SetToVertexShader(uint32_t startSlot)
	{
		return;
	}

	void SRSamplerState::SetToPixelShader(uint32_t startSlot)
	{
		g_pSRImmediateContext->PSSetSampler(startSlot, &m_pSamplerState);
	}

	

	ISamplerState* SRSamplerStateManager::CreateSamplerState(const SamplerDesc& desc)
	{
		ISamplerState* pSampler = new SRSamplerState(desc);

		return pSampler;
	}

	void SRSamplerStateManager::ReleaseSamplerState(SRSamplerState* pSampler)
	{
		if (pSampler)
		{
			delete pSampler;
			pSampler = nullptr;
		}
	}
}// RenderDog