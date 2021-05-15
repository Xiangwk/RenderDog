#include "ShaderResourceView.h"
#include "Utility.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
		m_pView = new uint32_t[nSize];
		if (!m_pView)
		{
			return false;
		}

		unsigned char* pSrc = pTexData;
		for (uint32_t row = 0; row < m_nHeight; ++row)
		{
			pSrc = pTexData + row * 4 * m_nWidth;
			for (uint32_t col = 0; col < m_nWidth; ++col)
			{
				float r = pSrc[0] / 255.0f;
				float g = pSrc[1] / 255.0f;
				float b = pSrc[2] / 255.0f;
				float a = pSrc[3] / 255.0f;

				float fColor[4] = { r, g, b, a };
				uint32_t pixelColor = ConvertFloatColorToUInt32(fColor);
				m_pView[row * m_nWidth + col] = pixelColor;

				pSrc += 4;
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