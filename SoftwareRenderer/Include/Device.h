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
	struct RenderTargetDesc
	{
		uint32_t	width;
		uint32_t	height;
		void*		pFrameBuf;
	};

	class Device
	{
	public:
		Device() = default;
		~Device() = default;

		bool CreateRenderTarget(const RenderTargetDesc& Desc, uint32_t**& pRenderTarget);
	};

	class DeviceContext
	{
	public:
		DeviceContext(uint32_t width, uint32_t height) :
			m_frameBuffer(nullptr),
			m_FrameBufferWidth(width),
			m_FrameBufferHeight(height)
		{}

		~DeviceContext()
		{}

		void OMSetRenderTarget(uint32_t* pRenderTarget);
		void ClearRenderTarget(uint32_t** pRenderTarget, const float* pClearColor);
		void Draw();

	private:
		uint32_t*	m_frameBuffer;
		uint32_t	m_FrameBufferWidth;
		uint32_t	m_FrameBufferHeight;
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
			m_Width(0),
			m_Height(0),
			m_hWnd(nullptr),
			m_hWndDC(nullptr),
			m_hBitMap(nullptr),
			m_hOldBitMap(nullptr)
		{}

		~SwapChain();

		SwapChain(const SwapChainDesc& desc);

		unsigned char* GetBackBuffer() { return m_pBackBuffer; }

		void Release();

		void Present();

	private:
		unsigned char*	m_pBackBuffer;
		uint32_t		m_Width;
		uint32_t		m_Height;

		HWND			m_hWnd;
		HDC				m_hWndDC;
		HBITMAP			m_hBitMap;
		HBITMAP			m_hOldBitMap;
	};

	bool CreateDeviceAndSwapChain(Device** pDevice, DeviceContext** pDeviceContext, SwapChain** pSwapChain, const SwapChainDesc& swapChainDesc);
	void ReleaseDevice(Device* pDevice, DeviceContext* pDeviceContext);
}