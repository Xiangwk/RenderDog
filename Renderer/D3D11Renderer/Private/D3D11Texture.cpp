////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: D3D11Texture.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Texture.h"
#include "D3D11Renderer.h"

#include <d3d11.h>
#include <DirectXTex.h>

namespace RenderDog
{
	//==================================================
	//		Texture2D
	//==================================================

	class D3D11Texture2D : public ITexture2D
	{
	public:
		D3D11Texture2D(const TextureDesc& desc);
		virtual ~D3D11Texture2D();

		virtual bool				LoadFromFile(const std::wstring& filePath) override;

		virtual void				Release() override;

		virtual void*				GetRenderTargetView() override { return (void*)m_pRTV; }
		virtual void*				GetDepthStencilView() override { return (void*)m_pDSV; }
		virtual void*				GetShaderResourceView() override { return (void*)m_pSRV; }
		

	private:
		ID3D11Texture2D*			m_pTexture2D;
		ID3D11RenderTargetView*		m_pRTV;
		ID3D11DepthStencilView*		m_pDSV;
		ID3D11ShaderResourceView*	m_pSRV;
	};

	D3D11Texture2D::D3D11Texture2D(const TextureDesc& desc) :
		m_pTexture2D(nullptr),
		m_pRTV(nullptr),
		m_pDSV(nullptr),
		m_pSRV(nullptr)
	{
		/*
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = desc.width;
		texDesc.Height = desc.height;
		texDesc.MipLevels = desc.mipLevels;
		texDesc.ArraySize = desc.arraySize;
		texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = desc.isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;
		if (FAILED(g_pD3D11Device->CreateTexture2D(&texDesc, nullptr, &m_pTexture2D)))
		{
			return;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = 0;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		if (FAILED(g_pD3D11Device->CreateDepthStencilView(m_pShadowDepthTexture, &dsvDesc, &m_pShadowDepthDSV)))
		{
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		if (FAILED(g_pD3D11Device->CreateShaderResourceView(m_pShadowDepthTexture, &srvDesc, &m_pShadowDepthSRV)))
		{
			return false;
		}

		return true;
		*/
	}

	D3D11Texture2D::~D3D11Texture2D()
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

		if (m_pDSV)
		{
			m_pDSV->Release();
			m_pDSV = nullptr;
		}

		if (m_pSRV)
		{
			m_pSRV->Release();
			m_pSRV = nullptr;
		}
	}

	bool D3D11Texture2D::LoadFromFile(const std::wstring& filePath)
	{
		DirectX::TexMetadata MetaData = DirectX::TexMetadata();
		DirectX::ScratchImage Image = DirectX::ScratchImage();

		HRESULT hr = 0;
		uint32_t pos = (uint32_t)filePath.find('.');
		std::wstring fileExt = filePath.substr(pos);
		if (fileExt == L".dds")
		{
			hr = DirectX::GetMetadataFromDDSFile(filePath.c_str(), 0, MetaData);
			hr = DirectX::LoadFromDDSFile(filePath.c_str(), 0, &MetaData, Image);
		}
		else if (fileExt == L".tga")
		{
			hr = DirectX::GetMetadataFromTGAFile(filePath.c_str(), MetaData);
			hr = DirectX::LoadFromTGAFile(filePath.c_str(), &MetaData, Image);
		}
		
		if (hr != S_OK)
		{
			return false;
		}

		hr = DirectX::CreateShaderResourceView(g_pD3D11Device, Image.GetImages(), MetaData.mipLevels, MetaData, &m_pSRV);
		if (hr != S_OK)
		{
			return false;
		}

		return true;
	}

	void D3D11Texture2D::Release()
	{
		g_pITextureManager->ReleaseTexture(this);
	}

	//==================================================
	//		Texture Manager
	//==================================================

	class D3D11TextureManager : public ITextureManager
	{
	public:
		D3D11TextureManager() = default;
		virtual ~D3D11TextureManager() = default;

		virtual ITexture2D* CreateTexture2D(const TextureDesc& desc) override;
		virtual void		ReleaseTexture(ITexture* pTexture) override;
	};

	D3D11TextureManager	g_D3D11TextureManager;
	ITextureManager*	g_pITextureManager = &g_D3D11TextureManager;

	ITexture2D* D3D11TextureManager::CreateTexture2D(const TextureDesc& desc)
	{
		ITexture2D* pTexture = new D3D11Texture2D(desc);
		pTexture->AddRef();

		return pTexture;
	}

	void D3D11TextureManager::ReleaseTexture(ITexture* pTexture)
	{
		pTexture->SubRef();
	}

	//==================================================
	//		SamplerState
	//==================================================

	class D3D11SamplerState : public ISamplerState
	{
	public:
		D3D11SamplerState();
		virtual ~D3D11SamplerState();

		virtual bool			Init(const SamplerDesc& desc) override;
		virtual void			Release() override;

		virtual void			SetToPixelShader(uint32_t startSlot) override;

	private:
		ID3D11SamplerState*		m_pSamplerState;
	};

	D3D11SamplerState::D3D11SamplerState() :
		m_pSamplerState(nullptr)
	{}

	D3D11SamplerState::~D3D11SamplerState()
	{}

	bool D3D11SamplerState::Init(const SamplerDesc& desc)
	{
		D3D11_SAMPLER_DESC samplerDesc = {};
		if (desc.filterMode == SAMPLER_FILTER::POINT)
		{
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		}
		else if (desc.filterMode == SAMPLER_FILTER::LINEAR)
		{
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		}

		if (desc.addressMode == SAMPLER_ADDRESS::WRAP)
		{
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		}
		else if (desc.addressMode == SAMPLER_ADDRESS::CLAMP)
		{
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

			samplerDesc.BorderColor[0] = 0.0f;
			samplerDesc.BorderColor[1] = 0.0f;
			samplerDesc.BorderColor[2] = 0.0f;
			samplerDesc.BorderColor[3] = 0.0f;
		}
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		if (FAILED(g_pD3D11Device->CreateSamplerState(&samplerDesc, &m_pSamplerState)))
		{
			return false;
		}

		return true;
	}

	void D3D11SamplerState::Release()
	{
		if (m_pSamplerState)
		{
			m_pSamplerState->Release();
			m_pSamplerState = nullptr;
		}
	}

	void D3D11SamplerState::SetToPixelShader(uint32_t startSlot)
	{
		g_pD3D11ImmediateContext->PSSetSamplers(startSlot, 1, &m_pSamplerState);
	}

	//==================================================
	//		SamplerState Manager
	//==================================================

	class D3D11SamplerStateManager : public ISamplerStateManager
	{
	public:
		D3D11SamplerStateManager() = default;
		virtual ~D3D11SamplerStateManager() = default;

		virtual ISamplerState*	CreateSamplerState() override;
		virtual void			ReleaseSamplerState(ISamplerState* pSampler) override;
	};

	D3D11SamplerStateManager	g_D3D11SamplerStateManager;
	ISamplerStateManager*		g_pISamplerStateManager = &g_D3D11SamplerStateManager;

	ISamplerState* D3D11SamplerStateManager::CreateSamplerState()
	{
		ISamplerState* pSampler = new D3D11SamplerState();

		return pSampler;
	}

	void D3D11SamplerStateManager::ReleaseSamplerState(ISamplerState* pSampler)
	{
		if (pSampler)
		{
			delete pSampler;
			pSampler = nullptr;
		}
	}
	
}// namespace RenderDog