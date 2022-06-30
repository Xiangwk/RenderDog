////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Texture.h
//Written by Xiang Weikang
//Desc: Texture Interface
////////////////////////////////////////

#pragma once

#include <string>

namespace RenderDog
{
	//==================================================
	//		Texture
	//==================================================
	enum class TEXTURE_FORMAT
	{
		UNKNOWN,
		R8G8B8A8_UNORM,
		R24G8_TYPELESS
	};

	struct TextureDesc
	{
		std::wstring			name;
		TEXTURE_FORMAT			format;
		uint32_t				width;
		uint32_t				height;
		uint32_t				mipLevels;
		bool					isDynamic;
		bool					isDepthTexture;

		TextureDesc() :
			name(L""),
			format(TEXTURE_FORMAT::UNKNOWN),
			width(0),
			height(0),
			mipLevels(0),
			isDynamic(false),
			isDepthTexture(false)
		{}
	};

	class ITexture
	{
	protected:
		virtual ~ITexture() = default;

	public:
		virtual void				Release() = 0;

		virtual const std::wstring& GetName() const = 0;
	};

	class ITexture2D : public ITexture
	{
	protected:
		virtual ~ITexture2D() = default;
		
	public:
		
		virtual void*				GetShaderResourceView() = 0;
		virtual void*				GetRenderTargetView() = 0;
		virtual void*				GetDepthStencilView() = 0;
	};


	//==================================================
	//		Texture Manager
	//==================================================
	class ITextureManager
	{
	public:
		virtual ~ITextureManager() = default;

		virtual	ITexture2D*			CreateTexture2D(const std::wstring& filePath) = 0;

		virtual ITexture2D*			GetTexture2D(const TextureDesc& desc) = 0;
	};

	extern ITextureManager* g_pITextureManager;


	//==================================================
	//		SamplerState
	//==================================================
	enum class SAMPLER_FILTER
	{
		POINT,
		LINEAR,
		COMPARISON_LINEAR
	};

	enum class SAMPLER_ADDRESS
	{
		WRAP,
		CLAMP,
		BORDER
	};

	struct SamplerDesc
	{
		SAMPLER_FILTER			filterMode;
		SAMPLER_ADDRESS			addressMode;
		float					borderColor[4];

		SamplerDesc() :
			filterMode(SAMPLER_FILTER::POINT),
			addressMode(SAMPLER_ADDRESS::WRAP)
		{
			borderColor[0] = 0.0f;
			borderColor[1] = 0.0f;
			borderColor[2] = 0.0f;
			borderColor[3] = 0.0f;
		}
	};

	class ISamplerState
	{
	protected:
		virtual ~ISamplerState() = default;

	public:
		virtual void			Release() = 0;

		virtual void			SetToPixelShader(uint32_t startSlot) = 0;
	};

	//==================================================
	//		SamplerState Manager
	//==================================================
	class ISamplerStateManager
	{
	public:
		virtual ~ISamplerStateManager() = default;

		virtual ISamplerState*	CreateSamplerState(const SamplerDesc& desc) = 0;
	};

	extern ISamplerStateManager* g_pISamplerStateManager;

}// namespace RenderDog
