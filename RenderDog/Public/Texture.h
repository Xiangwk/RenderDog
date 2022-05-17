////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Texture.h
//Written by Xiang Weikang
//Desc: Texture Interface
////////////////////////////////////////

#pragma once

#include "RefCntObject.h"

#include <string>

namespace RenderDog
{
	//==================================================
	//		Texture
	//==================================================

	enum class TextureBindFlag
	{
		RD_BIND_RENDER_TARGET,
		RD_BIND_SHADER_RESOURCE,
		RD_BIND_DEPTH_STENCIL
	};

	enum class TextureFormat
	{
		RD_FORMAT_R8G8B8A8_UNORM
	};

	struct TextureDesc
	{
		TextureBindFlag			bindFlag;
		TextureFormat			format;
		uint32_t				width;
		uint32_t				height;
		uint32_t				mipLevels;
		bool					isDynamic;
	};

	class ITexture : public RefCntObject
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
	};


	//==================================================
	//		Texture Manager
	//==================================================
	class ITextureManager
	{
	public:
		virtual ~ITextureManager() = default;

		virtual ITexture2D*		CreateTexture2D(const TextureDesc& desc) = 0;
		virtual void			ReleaseTexture(ITexture* pTexture) = 0;
	};

	extern ITextureManager* g_pITextureManager;


	//==================================================
	//		SamplerState
	//==================================================

	enum class SamplerFilterMode
	{
		RD_SAMPLER_FILTER_POINT,
		RD_SAMPLER_FILTER_LINEAR
	};

	enum class SamplerAddressMode
	{
		RD_SAMPLER_ADDRESS_WRAP,
		RD_SAMPLER_ADDRESS_CLAMP
	};

	struct SamplerDesc
	{
		SamplerFilterMode		filterMode;
		SamplerAddressMode		addressMode;
	};

	class ISamplerState
	{
	public:
		virtual ~ISamplerState() = default;

		virtual bool			Init(const SamplerDesc& desc) = 0;
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

		virtual ISamplerState*	CreateSamplerState() = 0;
		virtual void			ReleaseSamplerState(ISamplerState* pSampler) = 0;
	};

	extern ISamplerStateManager* g_pISamplerStateManager;

}// namespace RenderDog
