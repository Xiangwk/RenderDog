#include "Texture.h"

namespace RenderDog
{
	Texture2D::Texture2D() :
		m_pData(nullptr),
		m_nWidth(0),
		m_nHeight(0),
		m_Format(RD_FORMAT::UNKNOWN)
	{}

	Texture2D::Texture2D(uint32_t width, uint32_t height, RD_FORMAT format) :
		m_pData(nullptr),
		m_nWidth(width),
		m_nHeight(height)
	{
		m_Format = format;

		if (m_Format == RD_FORMAT::R8G8B8A8_UNORM)
		{
			uint32_t* pData = new uint32_t[width * height];
			for (uint32_t i = 0; i < width * height; ++i)
			{
				pData[i] = 0;
			}

			m_pData = pData;
		} 
		else if(m_Format == RD_FORMAT::R32_FLOAT)
		{
			float* pData = new float[width * height];
			for (uint32_t i = 0; i < width * height; ++i)
			{
				pData[i] = 0.0f;
			}

			m_pData = pData;
		}
	}

	Texture2D::~Texture2D()
	{}

	void Texture2D::Release()
	{	
		if (m_pData)
		{
			delete[] m_pData;
		}
	}
}

