////////////////////////////////////////
//RenderDog <·,·>
//FileName: D3D11Texture.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Texture.h"
#include "RefCntObject.h"
#include "D3D11Renderer.h"

#include <d3d11.h>
#include <DirectXTex.h>
#include <unordered_map>

namespace std
{
	template<>
	struct hash<RenderDog::SamplerDesc>
	{
		size_t operator()(const RenderDog::SamplerDesc& desc) const
		{
			return hash<string>()(desc.name)
				^ hash<uint32_t>()(uint32_t(desc.filterMode))
				^ hash<uint32_t>()(uint32_t(desc.addressMode))
				^ hash<float>()(desc.borderColor[0])
				^ hash<float>()(desc.borderColor[1])
				^ hash<float>()(desc.borderColor[2])
				^ hash<float>()(desc.borderColor[3]);
		}
	};
}

namespace RenderDog
{
	//==================================================
	//		Texture2D
	//==================================================
	class D3D11Texture2D : public ITexture2D, public RefCntObject
	{
	public:
		D3D11Texture2D();
		explicit D3D11Texture2D(const TextureDesc& desc);
		virtual ~D3D11Texture2D();

		virtual void				Release() override;

		virtual const std::wstring& GetName() const override { return m_Name; }

		virtual void*				GetRenderTargetView() override { return (void*)m_pRTV; }
		virtual void*				GetDepthStencilView() override { return (void*)m_pDSV; }
		virtual void*				GetShaderResourceView() override { return (void*)m_pSRV; }

		bool						LoadFromFile(const std::wstring& filePath);
		
	private:
		std::wstring				m_Name;

		ID3D11Texture2D*			m_pTexture2D;
		ID3D11RenderTargetView*		m_pRTV;
		ID3D11DepthStencilView*		m_pDSV;
		ID3D11ShaderResourceView*	m_pSRV;
	};

	//==================================================
	//		Texture Manager
	//==================================================
	class D3D11TextureManager : public ITextureManager
	{
	private:
		typedef std::unordered_map<std::wstring, ITexture*> TextureMap;

	public:
		D3D11TextureManager() = default;
		virtual ~D3D11TextureManager() = default;

		virtual ITexture2D*			CreateTexture2D(const std::wstring& filePath) override;
		virtual ITexture2D*			GetTexture2D(const TextureDesc& desc) override;

		void						ReleaseTexture2D(D3D11Texture2D* pTexture);

	private:
		TextureMap					m_TextureMap;
	};

	D3D11TextureManager	g_D3D11TextureManager;
	ITextureManager*	g_pITextureManager = &g_D3D11TextureManager;

	//==================================================
	//		Function Implementation
	//==================================================
	D3D11Texture2D::D3D11Texture2D() :
		RefCntObject(),
		m_Name(L""),
		m_pTexture2D(nullptr),
		m_pRTV(nullptr),
		m_pDSV(nullptr),
		m_pSRV(nullptr)
	{}

	D3D11Texture2D::D3D11Texture2D(const TextureDesc& desc) :
		RefCntObject(),
		m_Name(desc.name),
		m_pTexture2D(nullptr),
		m_pRTV(nullptr),
		m_pDSV(nullptr),
		m_pSRV(nullptr)
	{
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = desc.width;
		texDesc.Height = desc.height;
		texDesc.MipLevels = desc.mipLevels;
		texDesc.ArraySize = 1;
		switch (desc.format)
		{
		case TEXTURE_FORMAT::R24G8_TYPELESS:
			texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			break;
		default:
			texDesc.Format = DXGI_FORMAT_UNKNOWN;
			break;
		}
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = desc.isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = (desc.isDepthTexture ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET) | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;
		if (FAILED(g_pD3D11Device->CreateTexture2D(&texDesc, nullptr, &m_pTexture2D)))
		{
			return;
		}

		if (desc.isDepthTexture)
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			dsvDesc.Flags = 0;
			dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = 0;
			if (FAILED(g_pD3D11Device->CreateDepthStencilView(m_pTexture2D, &dsvDesc, &m_pDSV)))
			{
				return;
			}
		}
		else
		{
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
			rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0;
			if (FAILED(g_pD3D11Device->CreateRenderTargetView(m_pTexture2D, &rtvDesc, &m_pRTV)))
			{
				return;
			}
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		switch (desc.format)
		{
		case TEXTURE_FORMAT::R24G8_TYPELESS:
			srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			break;
		default:
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			break;
		}
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		if (FAILED(g_pD3D11Device->CreateShaderResourceView(m_pTexture2D, &srvDesc, &m_pSRV)))
		{
			return;
		}
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

		bool isCubeTexture = MetaData.IsCubemap();

		hr = DirectX::CreateShaderResourceView(g_pD3D11Device, Image.GetImages(), MetaData.arraySize * MetaData.mipLevels, MetaData, &m_pSRV);
		if (hr != S_OK)
		{
			return false;
		}

		m_Name = filePath;

		return true;
	}

	void D3D11Texture2D::Release()
	{
		g_D3D11TextureManager.ReleaseTexture2D(this);
	}

	ITexture2D* D3D11TextureManager::CreateTexture2D(const std::wstring& filePath)
	{
		D3D11Texture2D* pTexture = nullptr;

		auto texture = m_TextureMap.find(filePath);
		if (texture != m_TextureMap.end())
		{
			//NOTE!!! 这里用强转是否合适？
			pTexture = (D3D11Texture2D*)(texture->second);
			pTexture->AddRef();
		}
		else
		{
			pTexture = new D3D11Texture2D();
			if (pTexture)
			{
				if (!pTexture->LoadFromFile(filePath))
				{
					return nullptr;
				}
			}

			m_TextureMap.insert({ filePath, pTexture });
		}

		return pTexture;
	}

	ITexture2D* D3D11TextureManager::GetTexture2D(const TextureDesc& desc)
	{
		D3D11Texture2D* pTexture = nullptr;

		auto texture = m_TextureMap.find(desc.name);
		if (texture != m_TextureMap.end())
		{
			//NOTE!!! 这里用强转是否合适？
			pTexture = (D3D11Texture2D*)(texture->second);
			pTexture->AddRef();
		}
		else
		{
			pTexture = new D3D11Texture2D(desc);
		}

		return pTexture;
	}

	void D3D11TextureManager::ReleaseTexture2D(D3D11Texture2D* pTexture)
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
	class D3D11SamplerState : public ISamplerState, public RefCntObject
	{
	public:
		D3D11SamplerState();
		D3D11SamplerState(const SamplerDesc& desc);
		virtual ~D3D11SamplerState();

		virtual void					Release() override;

		virtual const SamplerDesc&		GetDesc() const override;

		virtual void					SetToVertexShader(uint32_t startSlot) override;
		virtual void					SetToPixelShader(uint32_t startSlot) override;

	private:
		ID3D11SamplerState*				m_pSamplerState;
		SamplerDesc						m_Desc;
	};

	//==================================================
	//		SamplerState Manager
	//==================================================
	class D3D11SamplerStateManager : public ISamplerStateManager
	{
	private:
		typedef std::unordered_map<SamplerDesc, ISamplerState*> SamplerMap;

	public:
		D3D11SamplerStateManager() = default;
		virtual ~D3D11SamplerStateManager() = default;

		virtual ISamplerState*	CreateSamplerState(const SamplerDesc& desc) override;

		void					ReleaseSamplerState(D3D11SamplerState* pSampler);

	private:
		SamplerMap				m_SamplerMap;
	};

	D3D11SamplerStateManager	g_D3D11SamplerStateManager;
	ISamplerStateManager*		g_pISamplerStateManager = &g_D3D11SamplerStateManager;


	//==================================================
	//		Function Implementation
	//==================================================
	D3D11SamplerState::D3D11SamplerState():
		m_pSamplerState(nullptr),
		m_Desc()
	{}

	D3D11SamplerState::D3D11SamplerState(const SamplerDesc& desc):
		m_pSamplerState(nullptr),
		m_Desc(desc)
	{
		D3D11_SAMPLER_DESC samplerDesc = {};
		if (desc.filterMode == SAMPLER_FILTER::POINT)
		{
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		}
		else if (desc.filterMode == SAMPLER_FILTER::LINEAR)
		{
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		}
		else if (desc.filterMode == SAMPLER_FILTER::COMPARISON_LINEAR)
		{
			samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
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
		}
		else if (desc.addressMode == SAMPLER_ADDRESS::BORDER)
		{
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;

			samplerDesc.BorderColor[0] = desc.borderColor[0];
			samplerDesc.BorderColor[1] = desc.borderColor[1];
			samplerDesc.BorderColor[2] = desc.borderColor[2];
			samplerDesc.BorderColor[3] = desc.borderColor[3];
		}
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		g_pD3D11Device->CreateSamplerState(&samplerDesc, &m_pSamplerState);
	}


	D3D11SamplerState::~D3D11SamplerState()
	{
		if (m_pSamplerState)
		{
			m_pSamplerState->Release();
			m_pSamplerState = nullptr;
		}
	}

	void D3D11SamplerState::Release()
	{
		g_D3D11SamplerStateManager.ReleaseSamplerState(this);
	}

	const SamplerDesc& D3D11SamplerState::GetDesc() const
	{
		return m_Desc;
	}

	void D3D11SamplerState::SetToVertexShader(uint32_t startSlot)
	{
		g_pD3D11ImmediateContext->VSSetSamplers(startSlot, 1, &m_pSamplerState);
	}

	void D3D11SamplerState::SetToPixelShader(uint32_t startSlot)
	{
		g_pD3D11ImmediateContext->PSSetSamplers(startSlot, 1, &m_pSamplerState);
	}

	ISamplerState* D3D11SamplerStateManager::CreateSamplerState(const SamplerDesc& desc)
	{
		D3D11SamplerState* pSampler = nullptr;

		auto iter = m_SamplerMap.find(desc);
		if (iter != m_SamplerMap.end())
		{
			pSampler = (D3D11SamplerState*)(iter->second);
			pSampler->AddRef();
		}
		else
		{
			pSampler = new D3D11SamplerState(desc);
			m_SamplerMap.insert({ desc, pSampler });
		}
		
		return pSampler;
	}

	void D3D11SamplerStateManager::ReleaseSamplerState(D3D11SamplerState* pSampler)
	{
		if (pSampler)
		{
			SamplerDesc samplerDesc = pSampler->GetDesc();
			if (pSampler->SubRef() == 0)
			{
				m_SamplerMap.erase(samplerDesc);
			}
		}
	}
	
}// namespace RenderDog