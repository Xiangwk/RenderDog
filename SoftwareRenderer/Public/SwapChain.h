#pragma once

#include <windows.h>
#include <cstdint>

namespace RenderDog
{
	class Texture2D;

	struct SwapChainDesc
	{
		uint32_t	nWidth;
		uint32_t	nHeight;
		HWND		hOutputWindow;
	};

	class SwapChain
	{
	public:
		SwapChain() :
			m_pBackBuffer(nullptr),
			m_nWidth(0),
			m_nHeight(0),
			m_hWnd(nullptr),
			m_hWndDC(nullptr),
			m_hBitMap(nullptr),
			m_hOldBitMap(nullptr)
		{}

		~SwapChain();

		SwapChain(const SwapChainDesc* pDesc);

		bool GetBuffer(Texture2D** ppSurface);

		void Release();

		void Present();

	private:
		uint32_t* m_pBackBuffer;
		uint32_t		m_nWidth;
		uint32_t		m_nHeight;

		HWND			m_hWnd;
		HDC				m_hWndDC;
		HBITMAP			m_hBitMap;
		HBITMAP			m_hOldBitMap;
	};
}