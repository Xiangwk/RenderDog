///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Device.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include <Windows.h>
#include <cstdint>

namespace RenderDog
{
	class Texture2D;
	class RenderTargetView;
	struct RenderTargetDesc;

	class Device
	{
	public:
		Device() = default;
		~Device() = default;

		bool CreateRenderTargetView(Texture2D* pTexture, const RenderTargetDesc* pDesc, RenderTargetView** ppRenderTarget);
	};

	class DeviceContext
	{
	public:
		DeviceContext() :
			m_pFrameBuffer(nullptr),
			m_nWidth(0),
			m_nHeight(0)
		{}

		DeviceContext(uint32_t width, uint32_t height) :
			m_pFrameBuffer(nullptr),
			m_nWidth(width),
			m_nHeight(height)
		{}

		~DeviceContext()
		{}

		void OMSetRenderTarget(RenderTargetView* pRenderTarget);
		void ClearRenderTarget(RenderTargetView* pRenderTarget, const float* ClearColor);
		void Draw();

	private:
		void DrawLineWithDDA(float fPos1X, float fPos1Y, float fPos2X, float fPos2Y, const float* lineColor);

	private:
		uint32_t*	m_pFrameBuffer;
		uint32_t	m_nWidth;
		uint32_t	m_nHeight;
	};

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
		uint32_t*		m_pBackBuffer;
		uint32_t		m_nWidth;
		uint32_t		m_nHeight;

		HWND			m_hWnd;
		HDC				m_hWndDC;
		HBITMAP			m_hBitMap;
		HBITMAP			m_hOldBitMap;
	};

	bool CreateDeviceAndSwapChain(Device** pDevice, DeviceContext** pDeviceContext, SwapChain** ppSwapChain, const SwapChainDesc* pSwapChainDesc);
}