///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Texture.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include <cstdint>

namespace RenderDog
{
	enum TextureFormat
	{
		TF_UINT32,
		TF_FLOAT32
	};

	struct Texture2DDesc
	{
		uint32_t		width;
		uint32_t		height;
		TextureFormat	format;
	};

	class Texture2D
	{
	public:
		Texture2D();

		Texture2D(uint32_t width, uint32_t height, TextureFormat format);

		~Texture2D();

		void Release();

		void*& GetData() { return m_pData; }
		const void* GetData() const { return m_pData; }

		void SetWidth(uint32_t width) { m_nWidth = width; }
		void SetHeight(uint32_t Height) { m_nHeight = Height; }

		uint32_t GetWidth() const { return m_nWidth; }
		uint32_t GetHeight() const { return m_nHeight; }

	private:
		void*		m_pData;
		uint32_t	m_nWidth;
		uint32_t	m_nHeight;
	};
}