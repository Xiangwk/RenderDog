#include "Device.h"

namespace RenderDog
{
	bool Device::CreateRenderTarget(const RenderTargetDesc& Desc, uint32_t**& pRenderTarget)
	{
		int needSize = sizeof(void*) * (Desc.height * 2 + 1024) + Desc.width * Desc.height * 8;
		char* ptr = (char*)malloc(needSize + 64);
		pRenderTarget = (uint32_t**)ptr;

		char* pFrameBuf = (char*)Desc.pFrameBuf;
		for (uint32_t i = 0; i < Desc.height; ++i)
		{
			pRenderTarget[i] = (uint32_t*)(pFrameBuf + Desc.width * 4 * i);
		}

		return true;
	}

	void DeviceContext::OMSetRenderTarget(uint32_t* pRenderTarget)
	{
		m_frameBuffer = pRenderTarget;
	}

	void DeviceContext::ClearRenderTarget(uint32_t** pRenderTarget, const float* pClearColor)
	{
		uint32_t nClearColor = 0x0;
		nClearColor = (uint32_t)(255 * pClearColor[0]) << 16 |
			(uint32_t)(255 * pClearColor[1]) << 8 |
			(uint32_t)(255 * pClearColor[2]);

		for (uint32_t i = 0; i < m_FrameBufferHeight; ++i)
		{
			for (uint32_t j = 0; j < m_FrameBufferWidth; ++j)
			{
				pRenderTarget[i][j] = nClearColor;
			}
		}
	}

	void DeviceContext::Draw()
	{
		return;
	}

	SwapChain::SwapChain(const SwapChainDesc& desc):
		m_hWnd(desc.hOutputWindow),
		m_Width(desc.nWidth),
		m_Height(desc.nHeight)
	{
		HDC hDC = GetDC(m_hWnd);
		m_hWndDC = CreateCompatibleDC(hDC);
		ReleaseDC(m_hWnd, hDC);

		void* pTempBitMapBuffer;
		BITMAPINFO BitMapInfo = { { sizeof(BITMAPINFOHEADER), (int)desc.nWidth, -(int)desc.nHeight, 1, 32, BI_RGB,
			desc.nWidth* desc.nHeight * 4, 0, 0, 0, 0 } };
		m_hBitMap = CreateDIBSection(m_hWndDC, &BitMapInfo, DIB_RGB_COLORS, &pTempBitMapBuffer, 0, 0);
		if (m_hBitMap)
		{
			m_hOldBitMap = (HBITMAP)SelectObject(m_hWndDC, m_hBitMap);
		}
		else
		{
			m_hOldBitMap = nullptr;
		}

		m_pBackBuffer = (unsigned char*)pTempBitMapBuffer;

		memset(m_pBackBuffer, 0, m_Width * m_Height * 4);
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
		BitBlt(hDC, 0, 0, m_Width, m_Height, m_hWndDC, 0, 0, SRCCOPY);
		ReleaseDC(m_hWnd, hDC);
	}

	bool CreateDeviceAndSwapChain(Device** pDevice, DeviceContext** pDeviceContext, SwapChain** pSwapChain, const SwapChainDesc& swapChainDesc)
	{
		*pDevice = new Device;
		if (!pDevice)
		{
			return false;
		}

		*pDeviceContext = new DeviceContext(swapChainDesc.nWidth, swapChainDesc.nHeight);
		if (!pDeviceContext)
		{
			return false;
		}

		*pSwapChain = new SwapChain(swapChainDesc);
		if (!pSwapChain)
		{
			return false;
		}

		return true;
	}

	void ReleaseDevice(Device* pDevice, DeviceContext* pDeviceContext)
	{
		if (pDevice)
		{
			delete pDevice;
			pDevice = nullptr;
		}

		if (pDeviceContext)
		{
			delete pDeviceContext;
			pDeviceContext = nullptr;
		}
	}
}