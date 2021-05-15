#include "ShaderResourceView.h"
#include "Utility.h"

#if	USE_FREE_IMAGE
#include "FreeImage.h"
#endif

#if USE_STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

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
#if USE_FREE_IMAGE
		FREE_IMAGE_FORMAT freeImageFormat = FIF_UNKNOWN;
		freeImageFormat = FreeImage_GetFileType(strFileName, 0);
		if (freeImageFormat == FIF_UNKNOWN)
		{
			freeImageFormat = FreeImage_GetFIFFromFilename(strFileName);
		}

		if ((freeImageFormat != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(freeImageFormat))
		{
			FIBITMAP* freeImageBitMap = FreeImage_Load(freeImageFormat, strFileName, 0);
			FIBITMAP* p32BitMap = FreeImage_ConvertTo32Bits(freeImageBitMap);
			FreeImage_Unload(freeImageBitMap);

			m_nWidth = FreeImage_GetWidth(p32BitMap);
			m_nHeight = FreeImage_GetHeight(p32BitMap);

			uint32_t nSize = m_nWidth * m_nHeight;
			m_pView = new uint32_t[nSize];
			if (!m_pView)
			{
				return false;
			}

			for (uint32_t row = 0; row < m_nHeight; ++row)
			{
				for (uint32_t col = 0; col < m_nWidth; ++col)
				{
					RGBQUAD color;
					FreeImage_GetPixelColor(p32BitMap, row, col, &color);

					float r = color.rgbRed / 255.0f;
					float g = color.rgbGreen / 255.0f;
					float b = color.rgbBlue / 255.0f;
					float a = 1.0f;

					float fColor[4] = { r, g, b, a };
					uint32_t pixelColor = ConvertFloatColorToUInt32(fColor);
					m_pView[row * m_nWidth + col] = pixelColor;
				}
			}

			FreeImage_Unload(p32BitMap);
		}
		else
		{
			return true;
		}

#elif USE_STB_IMAGE
		
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

#else
		//使用程序生成的棋盘格纹理
		m_pView = new uint32_t[256 * 256];
		m_nWidth = 256;
		m_nHeight = 256;
		if (!m_pView)
		{
			return false;
		}

		for (int j = 0; j < 256; j++)
		{
			for (int i = 0; i < 256; i++)
			{
				int x = i / 32, y = j / 32;
				m_pView[j * 256 + i] = ((x + y) & 1) ? 0xffffff : 0x3fbcef;
			}
		}
#endif

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