#include "Device.h"
#include "Texture.h"
#include "RenderTargetView.h"

#include <cmath>

namespace RenderDog
{
	bool Device::CreateRenderTargetView(Texture2D* pTexture, const RenderTargetDesc* pDesc, RenderTargetView** ppRenderTarget)
	{
		if (!pDesc)
		{
			RenderTargetView* pRT = new RenderTargetView();
			*ppRenderTarget = pRT;

			pRT->GetView() = pTexture->GetData();
			pRT->SetWidth(pTexture->GetWidth());
			pRT->SetHeight(pTexture->GetHeight());
		}
		else
		{
			//ToDo: Create RenderTargetView by Desc
		}

		return true;
	}

	void DeviceContext::OMSetRenderTarget(RenderTargetView* pRenderTarget)
	{
		m_pFrameBuffer = pRenderTarget->GetView();
	}

	void DeviceContext::ClearRenderTarget(RenderTargetView* pRenderTarget, const float* ClearColor)
	{
		uint32_t nClearColor = 0x0;
		nClearColor = (uint32_t)(255 * ClearColor[0]) << 16 | (uint32_t)(255 * ClearColor[1]) << 8 | (uint32_t)(255 * ClearColor[2]);

		uint32_t rtWidth = pRenderTarget->GetWidth();
		uint32_t rtHeight = pRenderTarget->GetHeight();
		uint32_t* pRT = pRenderTarget->GetView();
		for (uint32_t row = 0; row < rtHeight; ++row)
		{
			for (uint32_t col = 0; col < rtWidth; ++col)
			{
				uint32_t nIndex = row * rtWidth + col;
				pRT[nIndex] = nClearColor;
			}
		}
	}

	void DeviceContext::Draw()
	{
		float ClearColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
		
		DrawLineWithDDA(100, 100, 100, 100, ClearColor);
	}

	void DeviceContext::DrawLineWithDDA(float fPos1X, float fPos1Y, float fPos2X, float fPos2Y, const float* lineColor)
	{
		uint32_t nClearColor = (uint32_t)(255 * lineColor[0]) << 16 | (uint32_t)(255 * lineColor[1]) << 8 | (uint32_t)(255 * lineColor[2]);

		float DeltaX = fPos2X - fPos1X;
		float DeltaY = fPos2Y - fPos1Y;
		
		if (std::abs(DeltaX - 0.0f) < 0.000001f)
		{
			float yStart = std::fmin(fPos1Y, fPos2Y);
			float yEnd = std::fmax(fPos1Y, fPos2Y);
			for (int yStep = (int)yStart; yStep <= (int)yEnd; ++yStep)
			{
				m_pFrameBuffer[(int)fPos1X + yStep * m_nWidth] = nClearColor;
			}
			return;
		}
		
		float k = DeltaY / DeltaX;
		if (std::abs(k) <= 1.0f)
		{
			float xStart	= fPos1X < fPos2X ? fPos1X : fPos2X;
			float xEnd		= fPos1X < fPos2X ? fPos2X : fPos1X;
			float y			= fPos1X < fPos2X ? fPos1Y : fPos2Y;
			for (int xStep = (int)xStart; xStep <= (int)xEnd; ++xStep)
			{
				y += k;
				int yStep = (int)y;
				m_pFrameBuffer[xStep + yStep * m_nWidth] = nClearColor;
			}
		}
		else
		{
			float yStart	= fPos1Y < fPos2Y ? fPos1Y : fPos2Y;
			float yEnd		= fPos1Y < fPos2Y ? fPos2Y : fPos1Y;
			float x			= fPos1Y < fPos2Y ? fPos1X : fPos2X;
			for (int yStep = (int)yStart; yStep <= (int)yEnd; ++yStep)
			{
				x += 1.0f / k;
				int xStep = (int)x;
				m_pFrameBuffer[xStep + yStep * m_nWidth] = nClearColor;
			}
		}
	}

	SwapChain::SwapChain(const SwapChainDesc* pDesc):
		m_hWnd(pDesc->hOutputWindow),
		m_nWidth(pDesc->nWidth),
		m_nHeight(pDesc->nHeight)
	{
		HDC hDC = GetDC(m_hWnd);
		m_hWndDC = CreateCompatibleDC(hDC);
		ReleaseDC(m_hWnd, hDC);

		void* pTempBitMapBuffer;
		BITMAPINFO BitMapInfo = 
		{ 
			{ sizeof(BITMAPINFOHEADER), (int)pDesc->nWidth, -(int)pDesc->nHeight, 1, 32, BI_RGB, pDesc->nWidth * pDesc->nHeight * 4, 0, 0, 0, 0 }
		};
		m_hBitMap = CreateDIBSection(m_hWndDC, &BitMapInfo, DIB_RGB_COLORS, &pTempBitMapBuffer, 0, 0);
		if (m_hBitMap)
		{
			m_hOldBitMap = (HBITMAP)SelectObject(m_hWndDC, m_hBitMap);
		}
		else
		{
			m_hOldBitMap = nullptr;
		}

		m_pBackBuffer = (uint32_t*)pTempBitMapBuffer;

		memset(m_pBackBuffer, 0, (size_t)m_nWidth * (size_t)m_nHeight * 4);
	}

	SwapChain::~SwapChain()
	{
		m_pBackBuffer = nullptr;
	}

	void SwapChain::Release()
	{
		if (m_hWndDC)
		{
			if (m_hOldBitMap)
			{
				SelectObject(m_hWndDC, m_hOldBitMap);
				m_hOldBitMap = nullptr;
			}
			DeleteDC(m_hWndDC);
			m_hWndDC = nullptr;
		}

		if (m_hBitMap)
		{
			DeleteObject(m_hBitMap);
			m_hBitMap = nullptr;
		}

		if (m_hWnd)
		{
			CloseWindow(m_hWnd);
			m_hWnd = nullptr;
		}
	}

	void SwapChain::Present()
	{
		HDC hDC = GetDC(m_hWnd);
		BitBlt(hDC, 0, 0, m_nWidth, m_nHeight, m_hWndDC, 0, 0, SRCCOPY);
		ReleaseDC(m_hWnd, hDC);
	}

	bool SwapChain::GetBuffer(Texture2D** ppSurface)
	{
		Texture2D* pTex = new Texture2D();
		*ppSurface = pTex;

		pTex->GetData() = m_pBackBuffer;
		pTex->SetWidth(m_nWidth);
		pTex->SetHeight(m_nHeight);

		return true;
	}

	bool CreateDeviceAndSwapChain(Device** pDevice, DeviceContext** pDeviceContext, SwapChain** pSwapChain, const SwapChainDesc* pSwapChainDesc)
	{
		*pDevice = new Device;
		if (!pDevice)
		{
			return false;
		}

		*pDeviceContext = new DeviceContext(pSwapChainDesc->nWidth, pSwapChainDesc->nHeight);
		if (!pDeviceContext)
		{
			return false;
		}

		*pSwapChain = new SwapChain(pSwapChainDesc);
		if (!pSwapChain)
		{
			return false;
		}

		return true;
	}
}