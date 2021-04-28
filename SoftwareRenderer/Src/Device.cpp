#include "Device.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "Buffer.h"
#include "Shader.h"
#include "Vertex.h"
#include "Viewport.h"
#include "Matrix.h"

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

	bool Device::CreateVertexBuffer(const VertexBufferDesc& vbDesc, VertexBuffer** ppVertexBuffer)
	{
		if (*ppVertexBuffer)
		{
			(*ppVertexBuffer)->Release();
			*ppVertexBuffer = nullptr;
		}

		VertexBuffer* pVB = new VertexBuffer(vbDesc);
		if (!pVB)
		{
			return false;
		}

		*ppVertexBuffer = pVB;
		
		return true;
	}

	bool Device::CreateIndexBuffer(const IndexBufferDesc& ibDesc, IndexBuffer** ppIndexBuffer)
	{
		if (*ppIndexBuffer)
		{
			(*ppIndexBuffer)->Release();
			*ppIndexBuffer = nullptr;
		}

		IndexBuffer* pIB = new IndexBuffer(ibDesc);
		if (!pIB)
		{
			return false;
		}

		*ppIndexBuffer = pIB;

		return true;
	}

	bool Device::CreateVertexShader(VertexShader** ppVertexShader)
	{
		if (*ppVertexShader)
		{
			delete *ppVertexShader;
		}

		VertexShader* pVS = new VertexShader();
		if (!pVS)
		{
			return false;
		}

		*ppVertexShader = pVS;

		return true;
	}

	bool Device::CreatePixelShader(PixelShader** ppPixelShader)
	{
		if (*ppPixelShader)
		{
			delete *ppPixelShader;
		}

		PixelShader* pPS = new PixelShader();
		if (!pPS)
		{
			return false;
		}

		*ppPixelShader = pPS;

		return true;
	}

	DeviceContext::~DeviceContext()
	{
		if (m_pVSOutputs)
		{
			delete[] m_pVSOutputs;
			m_pVSOutputs = nullptr;
		}

		if (m_pViewportMat)
		{
			delete m_pViewportMat;
			m_pViewportMat = nullptr;
		}
	}

	void DeviceContext::IASetVertexBuffer(const VertexBuffer* pVB)
	{
		if (m_pVB != pVB)
		{
			uint32_t nVertexNum = pVB->GetNum();
			m_pVSOutputs = new Vertex[nVertexNum];

			m_pVB = pVB;
		}
	}
	void DeviceContext::IASetIndexBuffer(const IndexBuffer* pIB)
	{
		if (m_pIB != pIB)
		{
			m_pIB = pIB;
		}
	}

	void DeviceContext::VSSetTransMats(const Matrix4x4* matWorld, const Matrix4x4* matView, const Matrix4x4* matProj)
	{
		m_pWorldMat = matWorld;
		m_pViewMat = matView;
		m_pProjMat = matProj;
	}

	void DeviceContext::RSSetViewport(const Viewport* pVP)
	{
		Matrix4x4 matViewport = pVP->GetViewportMatrix();
		m_pViewportMat = new Matrix4x4(matViewport);
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

	void DeviceContext::DrawIndex(uint32_t nIndexNum)
	{
		const Vertex* pVerts = m_pVB->GetData();
		//Local to CVV
		for (uint32_t i = 0; i < m_pVB->GetNum(); ++i)
		{
			const Vertex& vert = pVerts[i];
			//FIXME!!! 除以w不能在VS里做
			m_pVSOutputs[i] = m_pVS->VSMain(vert, *m_pWorldMat, *m_pViewMat, *m_pProjMat);
		}

		//Clip


		//Clip to Screen
		for (uint32_t i = 0; i < m_pVB->GetNum(); ++i)
		{
			Vertex& vert = m_pVSOutputs[i];
			Vector4 vScreenPos(vert.vPostion, 1.0f);
			vScreenPos = vScreenPos * (*m_pViewportMat);
			vert.vPostion = Vector3(vScreenPos.x, vScreenPos.y, vScreenPos.z);
		}

		const uint32_t* pIndice = m_pIB->GetData();
		for (uint32_t i = 0; i < nIndexNum; i += 3)
		{
			const Vertex& vert0 = m_pVSOutputs[pIndice[i]];
			const Vertex& vert1 = m_pVSOutputs[pIndice[i + 1]];
			const Vertex& vert2 = m_pVSOutputs[pIndice[i + 2]];

			if (m_PriTopology == PrimitiveTopology::LINE_LIST)
			{
				DrawTriangleWithLine(vert0, vert1, vert2);
			}
			else if(m_PriTopology == PrimitiveTopology::TRIANGLE_LIST)
			{
				DrawTriangleWithFlat(vert0, vert1, vert2);
			}
		}
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

	void DeviceContext::DrawTriangleWithLine(const Vertex& v0, const Vertex& v1, const Vertex& v2)
	{
		float lineColor[4] = { v0.vColor.x, v0.vColor.y, v0.vColor.z, 1.0f };
		DrawLineWithDDA(v0.vPostion.x, v0.vPostion.y, v1.vPostion.x, v1.vPostion.y, lineColor);
		DrawLineWithDDA(v1.vPostion.x, v1.vPostion.y, v2.vPostion.x, v2.vPostion.y, lineColor);
		DrawLineWithDDA(v2.vPostion.x, v2.vPostion.y, v0.vPostion.x, v0.vPostion.y, lineColor);
	}

	void DeviceContext::DrawTriangleWithFlat(const Vertex& v0, const Vertex& v1, const Vertex& v2)
	{
		Vertex vert0(v0);
		Vertex vert1(v1);
		Vertex vert2(v2);
		SortTriangleVertsByYGrow(vert0, vert1, vert2);

		if (std::abs(vert0.vPostion.y - vert1.vPostion.y) < 0.000001f)
		{
			DrawTopTriangle(vert0, vert1, vert2);
		}
		else if (std::abs(vert1.vPostion.y - vert2.vPostion.y) < 0.000001f)
		{
			DrawBottomTriangle(vert0, vert1, vert2);
		}
		else
		{
			RenderDog::Vertex vertNew;
			SliceTriangleToUpAndBottom(vert0, vert1, vert2, vertNew);

			DrawBottomTriangle(vert0, vert1, vertNew);
			DrawTopTriangle(vert1, vertNew, vert2);
		}
	}

	void DeviceContext::SortTriangleVertsByYGrow(Vertex& v0, Vertex& v1, Vertex& v2)
	{
		if (v1.vPostion.y < v0.vPostion.y)
		{
			Vertex vTemp = v0;
			v0 = v1;
			v1 = vTemp;
		}
		else if (v2.vPostion.y < v0.vPostion.y)
		{
			Vertex vTemp = v0;
			v0 = v2;
			v2 = vTemp;
		}

		if (v2.vPostion.y < v1.vPostion.y)
		{
			Vertex vTemp = v1;
			v1 = v2;
			v2 = vTemp;
		}
	}

	void DeviceContext::SortScanlineVertsByXGrow(Vertex& v0, Vertex& v1)
	{
		if (v1.vPostion.x < v0.vPostion.x)
		{
			Vertex vTemp = v0;
			v0 = v1;
			v1 = vTemp;
		}
	}

	void DeviceContext::DrawTopTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
	{

	}

	void DeviceContext::DrawBottomTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
	{

	}

	void DeviceContext::SliceTriangleToUpAndBottom(const Vertex& v0, const Vertex& v1, const Vertex& v2, Vertex& vNew)
	{
		float fLerpFactor = (v1.vPostion.y - v0.vPostion.y) / (v2.vPostion.y - v0.vPostion.y);

		float fNewX = v0.vPostion.x + fLerpFactor * (v2.vPostion.x - v0.vPostion.x);
		float fNewZ = v0.vPostion.z + fLerpFactor * (v2.vPostion.z - v0.vPostion.z);

		Vector3 vNewPos(fNewX, v1.vPostion.y, fNewZ);

		Vector3 vNewColor = v0.vColor + fLerpFactor * (v2.vColor - v0.vColor);

		vNew.vPostion = vNewPos;
		vNew.vColor = vNewColor;
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