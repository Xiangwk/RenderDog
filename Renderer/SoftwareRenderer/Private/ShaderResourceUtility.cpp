#include "ShaderResourceUtility.h"

#include "SoftwareRenderer.h"
#include "Vector.h"

#define STB_IMAGE_IMPLEMENTATION
#include "StbImage/stb_image.h"

namespace RenderDog
{
	bool CreateShaderResourceViewFromFile(IDevice* pDevice, const char* strSrcFile, IShaderResourceView** ppShaderResourceView)
	{
		uint32_t nTexWidth = 0;
		uint32_t nTexHeight = 0;
		int nChannels = 0;
		unsigned char* pTexData = stbi_load(strSrcFile, (int*)&nTexWidth, (int*)&nTexHeight, &nChannels, 0);
		if (!pTexData)
		{
			return false;
		}

		ITexture2D* pTexture2D = nullptr;

		Texture2DDesc tex2DDesc;
		tex2DDesc.width = nTexWidth;
		tex2DDesc.height = nTexHeight;
		if (nChannels == 3 || nChannels == 4)
		{
			tex2DDesc.format = RD_FORMAT::R32G32B32A32_FLOAT;
		}

		Vector4* pColor = new Vector4[nTexWidth * nTexHeight];
		if (!pColor)
		{
			return false;
		}

		unsigned char* pSrc = pTexData;
		for (uint32_t row = 0; row < nTexHeight; ++row)
		{
			pSrc = pTexData + row * nChannels * nTexWidth;
			for (uint32_t col = 0; col < nTexWidth; ++col)
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
				pColor[row * nTexWidth + col] = fColor;

				pSrc += nChannels;
			}
		}

		stbi_image_free(pTexData);

		SubResourceData initData;
		initData.pSysMem = pColor;
		initData.sysMemPitch = nTexWidth * nTexHeight * sizeof(Vector4);
		if (!pDevice->CreateTexture2D(&tex2DDesc, &initData, &pTexture2D))
		{
			return false;
		}

		ShaderResourceViewDesc srvDesc;
		srvDesc.format = tex2DDesc.format;
		srvDesc.viewDimension = RD_SRV_DIMENSION::TEXTURE2D;
		if (!pDevice->CreateShaderResourceView(pTexture2D, &srvDesc, ppShaderResourceView))
		{
			return false;
		}

		pTexture2D->Release();

		delete[] pColor;

		return true;
	}
}