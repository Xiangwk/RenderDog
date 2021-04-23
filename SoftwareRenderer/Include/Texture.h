///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Texture.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include <cstdint>

namespace RenderDog
{
	class Texture2D
	{
	public:
		Texture2D() :
			m_pData(nullptr),
			m_nWidth(0),
			m_nHeight(0)
		{}

		~Texture2D()
		{}

		uint32_t*& GetData() { return m_pData; }
		const uint32_t* GetData() const { return m_pData; }

		void SetWidth(uint32_t width) { m_nWidth = width; }
		void SetHeight(uint32_t Height) { m_nHeight = Height; }

		uint32_t GetWidth() const { return m_nWidth; }
		uint32_t GetHeight() const { return m_nHeight; }

	private:
		uint32_t*	m_pData;
		uint32_t	m_nWidth;
		uint32_t	m_nHeight;
	};
}