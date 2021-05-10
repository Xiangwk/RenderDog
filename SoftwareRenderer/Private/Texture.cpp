#include "Texture.h"

namespace RenderDog
{
	Texture2D::Texture2D() :
		m_pData(nullptr),
		m_nWidth(0),
		m_nHeight(0)
	{}

	Texture2D::Texture2D(uint32_t width, uint32_t height, TextureFormat format) :
		m_pData(nullptr),
		m_nWidth(width),
		m_nHeight(height)
	{
		if (format == TF_UINT32)
		{
			uint32_t* pData = new uint32_t[width * height];
			for (uint32_t i = 0; i < width * height; ++i)
			{
				pData[i] = 0;
			}

			m_pData = pData;
		} 
		else if(format == TF_FLOAT32)
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
			m_pData = nullptr;
		}
	}
}

