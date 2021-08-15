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

		uint32_t*& GetDataUint32() { return m_pDataUint32; }
		const uint32_t* GetDataUint32() const { return m_pDataUint32; }

		float*& GetDataFloat32() { return m_pDataFloat32; }
		const float* GetDataFloat32() const { return m_pDataFloat32; }

		void SetWidth(uint32_t width) { m_nWidth = width; }
		void SetHeight(uint32_t Height) { m_nHeight = Height; }

		uint32_t GetWidth() const { return m_nWidth; }
		uint32_t GetHeight() const { return m_nHeight; }

	private:
		uint32_t*		m_pDataUint32;
		float*			m_pDataFloat32;
		uint32_t		m_nWidth;
		uint32_t		m_nHeight;

		RD_FORMAT		m_Format;
	};
}