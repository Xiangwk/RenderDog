#include "Texture.h"

namespace RenderDog
{
	Texture2D::Texture2D() :
		m_pDataUint32(nullptr),
		m_pDataFloat32(nullptr),
		m_nWidth(0),
		m_nHeight(0)
	{}

	Texture2D::Texture2D(uint32_t width, uint32_t height, TextureFormat format) :
		m_pDataUint32(nullptr),
		m_pDataFloat32(nullptr),
		m_nWidth(width),
		m_nHeight(height)
	{
		m_Format = format;

		if (m_Format == TF_UINT32)
		{
			uint32_t* pData = new uint32_t[width * height];
			for (uint32_t i = 0; i < width * height; ++i)
			{
				pData[i] = 0;
			}

			m_pDataUint32 = pData;
		} 
		else if(m_Format == TF_FLOAT32)
		{
			float* pData = new float[width * height];
			for (uint32_t i = 0; i < width * height; ++i)
			{
				pData[i] = 0.0f;
			}

			m_pDataFloat32 = pData;
		}
	}

	Texture2D::~Texture2D()
	{}

	void Texture2D::Release()
	{	
		if (m_pDataUint32 && m_Format == TF_UINT32)
		{
			delete[] m_pDataUint32;
			m_pDataUint32 = nullptr;
		}
		else if (m_pDataFloat32 && m_Format == TF_FLOAT32)
		{
			delete[] m_pDataFloat32;
			m_pDataFloat32 = nullptr;
		}
	}
}

