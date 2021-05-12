#include "ShaderResourceView.h"

#include "DirectXTex.h"

namespace RenderDog
{
	ShaderResourceView::ShaderResourceView() :
		m_pView(nullptr),
		m_nWidth(0),
		m_nHeight(0)
	{}

	ShaderResourceView::~ShaderResourceView()
	{}

	bool ShaderResourceView::LoadFromFile(const wchar_t* strFileName)
	{
		/*DirectX::TexMetadata MetaData = DirectX::TexMetadata();
		DirectX::ScratchImage DDSImage = DirectX::ScratchImage();

		HRESULT hr = 0;
		hr = DirectX::GetMetadataFromDDSFile(strFileName, 0, MetaData);
		if (hr != S_OK)
		{
			return false;
		}
		hr = DirectX::LoadFromDDSFile(strFileName, 0, &MetaData, DDSImage);
		if (hr != S_OK)
		{
			return false;
		}

		m_nWidth = (uint32_t)MetaData.width;
		m_nHeight = (uint32_t)MetaData.height;

		m_pView = new uint8_t[m_nWidth * m_nHeight];
		memcpy(m_pView, DDSImage.GetImage(0, 0, 0)->pixels, m_nWidth * m_nHeight);*/

		m_pView = new uint32_t[256 * 256];
		for (int j = 0; j < 256; j++)
		{
			for (int i = 0; i < 256; i++)
			{
				int x = i / 32, y = j / 32;
				m_pView[j * 256 + i] = ((x + y) & 1) ? 0xffffff : 0x3fbcef;
			}
		}

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