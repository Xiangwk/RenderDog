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
	class VertexBuffer;
	class IndexBuffer;
	class VertexShader;
	class PixelShader;
	class Matrix4x4;
	struct Viewport;
	struct RenderTargetDesc;
	struct VertexBufferDesc;
	struct IndexBufferDesc;
	struct Vertex;

	class Device
	{
	public:
		Device() = default;
		~Device() = default;

		bool CreateRenderTargetView(Texture2D* pTexture, const RenderTargetDesc* pDesc, RenderTargetView** ppRenderTarget);
		bool CreateVertexBuffer(const VertexBufferDesc& vbDesc, VertexBuffer** ppVertexBuffer);
		bool CreateIndexBuffer(const IndexBufferDesc& ibDesc, IndexBuffer** ppIndexBuffer);
		bool CreateVertexShader(VertexShader** ppVertexShader);
		bool CreatePixelShader(PixelShader** ppPixelShader);
	};

	class DeviceContext
	{
	public:
		DeviceContext() :
			m_pFrameBuffer(nullptr),
			m_nWidth(0),
			m_nHeight(0),
			m_pVB(nullptr),
			m_pIB(nullptr),
			m_pVS(nullptr),
			m_pPS(nullptr),
			m_pWorldMat(nullptr),
			m_pViewMat(nullptr),
			m_pProjMat(nullptr),
			m_pVSOutputs(nullptr),
			m_pViewportMat(nullptr)
		{}

		DeviceContext(uint32_t width, uint32_t height) :
			m_pFrameBuffer(nullptr),
			m_nWidth(width),
			m_nHeight(height),
			m_pVB(nullptr),
			m_pIB(nullptr),
			m_pVS(nullptr),
			m_pPS(nullptr),
			m_pWorldMat(nullptr),
			m_pViewMat(nullptr),
			m_pProjMat(nullptr),
			m_pVSOutputs(nullptr),
			m_pViewportMat(nullptr)
		{}

		~DeviceContext();

		void IASetVertexBuffer(const VertexBuffer* pVB);
		void IASetIndexBuffer(const IndexBuffer* pIB);

		void VSSetShader(const VertexShader* pVS) { m_pVS = pVS; }
		void VSSetTransMats(const Matrix4x4* matWorld, const Matrix4x4* matView, const Matrix4x4* matProj);
		void PSSetShader(const PixelShader* pPS) { m_pPS = pPS; }

		void RSSetViewport(const Viewport* pVP);

		void OMSetRenderTarget(RenderTargetView* pRenderTarget);
		void ClearRenderTarget(RenderTargetView* pRenderTarget, const float* ClearColor);
		void Draw();
		void DrawIndex(uint32_t nIndexNum);

	private:
		void DrawLineWithDDA(float fPos1X, float fPos1Y, float fPos2X, float fPos2Y, const float* lineColor);

	private:
		uint32_t*			m_pFrameBuffer;
		uint32_t			m_nWidth;
		uint32_t			m_nHeight;

		const VertexBuffer*	m_pVB;
		const IndexBuffer*	m_pIB;

		const VertexShader* m_pVS;
		const PixelShader*	m_pPS;

		const Matrix4x4*	m_pWorldMat;
		const Matrix4x4*	m_pViewMat;
		const Matrix4x4*	m_pProjMat;

		Vertex*				m_pVSOutputs;

		Matrix4x4*			m_pViewportMat;
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