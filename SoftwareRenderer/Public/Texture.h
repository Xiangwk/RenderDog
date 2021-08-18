///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Texture.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include <cstdint>

namespace RenderDog
{
	enum class RD_FORMAT
	{
		UNKNOWN = 0,
		R8G8B8A8_UNORM = 1,
		R32_FLOAT = 2
	};

	class Texture2D
	{
	public:
		Texture2D();

		Texture2D(uint32_t width, uint32_t height, RD_FORMAT format);

		~Texture2D();

		void Release();

		void*& GetData() { return m_pData; }
		const void* GetData() const { return m_pData; }

		void SetWidth(uint32_t width) { m_nWidth = width; }
		void SetHeight(uint32_t Height) { m_nHeight = Height; }

		uint32_t GetWidth() const { return m_nWidth; }
		uint32_t GetHeight() const { return m_nHeight; }

	private:
		void*			m_pData;
		uint32_t		m_nWidth;
		uint32_t		m_nHeight;

		RD_FORMAT		m_Format;
	};
}