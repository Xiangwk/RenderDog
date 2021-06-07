#include "ShaderResourceView.h"
#include "Vector.h"
#include "Utility.h"

#define STB_IMAGE_IMPLEMENTATION
#include "StbImage/stb_image.h"

namespace RenderDog
{
	ShaderResourceView::ShaderResourceView() :
		m_pView(nullptr),
		m_nWidth(0),
		m_nHeight(0)
	{}

	ShaderResourceView::~ShaderResourceView()
	{}

	bool ShaderResourceView::LoadFromFile(const char* strFileName)
	{		
		int nTexWidth = 0;
		int nTexHeight = 0;
		int nChannels = 0;
		unsigned char* pTexData = stbi_load(strFileName, &nTexWidth, &nTexHeight, &nChannels, 0);
		if (!pTexData)
		{
			return false;
		}

		m_nWidth = (uint32_t)nTexWidth;
		m_nHeight = (uint32_t)nTexHeight;

		uint32_t nSize = m_nWidth * m_nHeight;
		m_pView = new Vector4[nSize];
		if (!m_pView)
		{
			return false;
		}

		unsigned char* pSrc = pTexData;
		for (uint32_t row = 0; row < m_nHeight; ++row)
		{
			pSrc = pTexData + row * nChannels * m_nWidth;
			for (uint32_t col = 0; col < m_nWidth; ++col)
			{
				float r = pSrc[0] / 255.0f;
				float g = pSrc[1] / 255.0f;
				float b = pSrc[2] / 255.0f;
				float a = 0.0f;
				if (nChannels == 4)
				{
					a = pSrc[3] / 255.0f;
				}
				else
				{
					a = 1.0f;
				}

				Vector4 fColor = { r, g, b, a };
				m_pView[row * m_nWidth + col] = fColor;

				pSrc += nChannels;
			}
		}

		stbi_image_free(pTexData);

		return true;
	}

	void ShaderResourceView::Release()
	{
		if (m_pView)
		{
			delete[] m_pView;
			m_pView = nullptr;
		}
	}
}