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
	enum class TEXTURE_BIND_FLAG
	{
		RENDER_TARGET,
		SHADER_RESOURCE,
		DEPTH_STENCIL
	};

	enum class TEXTURE_FORMAT
	{
		R8G8B8A8_UNORM
	};

	struct TextureDesc
	{
		TEXTURE_BIND_FLAG		bindFlag;
		TEXTURE_FORMAT			format;
		uint32_t				width;
		uint32_t				height;
		uint32_t				mipLevels;
		bool					isDynamic;
	};

	class ITexture
	{
	protected:
		virtual ~ITexture() = default;

	public:
		virtual void			Release() = 0;
	};

	class ITexture2D : public ITexture
	{
	protected:
		virtual ~ITexture2D() = default;
		
	public:
		virtual bool			LoadFromFile(const std::wstring& filePath) = 0;

		virtual void*			GetShaderResourceView() = 0;
		virtual void*			GetRenderTargetView() = 0;
		virtual void*			GetDepthStencilView() = 0;
	};


	//==================================================
	//		Texture Manager
	//==================================================
	class ITextureManager
	{
	public:
		virtual ~ITextureManager() = default;

		virtual ITexture2D*		CreateTexture2D(const TextureDesc& desc) = 0;
	};

	extern ITextureManager* g_pITextureManager;


	//==================================================
	//		SamplerState
	//==================================================
	enum class SAMPLER_FILTER
	{
		POINT,
		LINEAR
	};

	enum class SAMPLER_ADDRESS
	{
		WRAP,
		CLAMP
	};

	struct SamplerDesc
	{
		SAMPLER_FILTER			filterMode;
		SAMPLER_ADDRESS			addressMode;

		SamplerDesc() :
			filterMode(SAMPLER_FILTER::POINT),
			addressMode(SAMPLER_ADDRESS::WRAP)
		{}
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
