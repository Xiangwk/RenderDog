#include "DeviceContext.h"
#include "Matrix.h"
#include "Vertex.h"
#include "Buffer.h"
#include "Viewport.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"
#include "Shader.h"
#include "Utility.h"

#include <iostream>

namespace RenderDog
{
	extern const float fEpsilon;

	DeviceContext::DeviceContext(uint32_t width, uint32_t height) :
		m_pFrameBuffer(nullptr),
		m_pDepthBuffer(nullptr),
#if DEBUG_RASTERIZATION
		m_pDebugBuffer(nullptr),
#endif
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
		m_pViewportMat(nullptr),
		m_PriTopology(PrimitiveTopology::TRIANGLE_LIST)
	{
#if DEBUG_RASTERIZATION
		uint32_t nBufferSize = m_nWidth * m_nHeight;
		m_pDebugBuffer = new uint32_t[nBufferSize];

		for (uint32_t i = 0; i < nBufferSize; ++i)
		{
			m_pDebugBuffer[i] = 0;
		}
#endif
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

#if DEBUG_RASTERIZATION
		if (m_pDebugBuffer)
		{
			delete[] m_pDebugBuffer;
			m_pDebugBuffer = nullptr;
		}
#endif
	}

	void DeviceContext::IASetVertexBuffer(const VertexBuffer* pVB)
	{
		if (m_pVB != pVB)
		{
			uint32_t nVertexNum = pVB->GetNum();
			m_pVSOutputs = new VSOutput[nVertexNum];

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

	void DeviceContext::OMSetRenderTarget(RenderTargetView* pRenderTarget, DepthStencilView* pDepthStencil)
	{
		m_pFrameBuffer = pRenderTarget->GetView();
		m_pDepthBuffer = pDepthStencil->GetView();
	}

	void DeviceContext::ClearRenderTarget(RenderTargetView* pRenderTarget, const float* ClearColor)
	{
		uint32_t nClearColor = 0x0;
		nClearColor = ConvertFloatColorToUInt32(ClearColor);

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

#if DEBUG_RASTERIZATION
		uint32_t nBufferSize = rtWidth * rtHeight;
		for (uint32_t i = 0; i < nBufferSize; ++i)
		{
			m_pDebugBuffer[i] = 0;
		}
#endif
	}

	void DeviceContext::ClearDepthStencil(DepthStencilView* pDepthStencil, float fDepth)
	{
		float* pDepth = pDepthStencil->GetView();
		uint32_t nWidth = pDepthStencil->GetWidth();
		uint32_t nHeight = pDepthStencil->GetHeight();
		for (uint32_t row = 0; row < nHeight; ++row)
		{
			for (uint32_t col = 0; col < nWidth; ++col)
			{
				uint32_t nIndex = row * nWidth + col;
				pDepth[nIndex] = fDepth;
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
			VSOutput& vsOutput = m_pVSOutputs[i];
			Vector4 vScreenPos(vsOutput.SVPosition.x, vsOutput.SVPosition.y, vsOutput.SVPosition.z, 1.0f);
			vScreenPos = vScreenPos * (*m_pViewportMat);
			vsOutput.SVPosition.x = vScreenPos.x;
			vsOutput.SVPosition.y = vScreenPos.y;
			vsOutput.SVPosition.z = vScreenPos.z;
		}

		const uint32_t* pIndice = m_pIB->GetData();
		for (uint32_t i = 0; i < nIndexNum; i += 3)
		{
			const VSOutput& vert0 = m_pVSOutputs[pIndice[i]];
			const VSOutput& vert1 = m_pVSOutputs[pIndice[i + 1]];
			const VSOutput& vert2 = m_pVSOutputs[pIndice[i + 2]];

			if (m_PriTopology == PrimitiveTopology::LINE_LIST)
			{
				DrawTriangleWithLine(vert0, vert1, vert2);
			}
			else if (m_PriTopology == PrimitiveTopology::TRIANGLE_LIST)
			{
				DrawTriangleWithFlat(vert0, vert1, vert2);
			}
		}
	}

#if DEBUG_RASTERIZATION
	bool DeviceContext::CheckDrawPixelTiwce()
	{
		for (uint32_t i = 0; i < m_nHeight; ++i)
		{
			for (uint32_t j = 0; j < m_nWidth; ++j)
			{
				uint32_t nDrawCnt = m_pDebugBuffer[i * m_nWidth + j];
				if (nDrawCnt > 1)
				{
					//std::cout << "Pixel: " << i << " " << j << " Draw " << nDrawCnt << std::endl;
					return true;
				}
			}
		}

		return false;
	}
#endif


	//------------------------------------------------------------------------------------------------------------------
	//Private Funtion
	//------------------------------------------------------------------------------------------------------------------
	void DeviceContext::DrawLineWithDDA(float fPos1X, float fPos1Y, float fPos2X, float fPos2Y, const float* lineColor)
	{
		uint32_t nClearColor = ConvertFloatColorToUInt32(lineColor);

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
			float xStart = fPos1X < fPos2X ? fPos1X : fPos2X;
			float xEnd = fPos1X < fPos2X ? fPos2X : fPos1X;
			float y = fPos1X < fPos2X ? fPos1Y : fPos2Y;
			for (int xStep = (int)xStart; xStep <= (int)xEnd; ++xStep)
			{
				y += k;
				int yStep = (int)y;
				m_pFrameBuffer[xStep + yStep * m_nWidth] = nClearColor;
			}
		}
		else
		{
			float yStart = fPos1Y < fPos2Y ? fPos1Y : fPos2Y;
			float yEnd = fPos1Y < fPos2Y ? fPos2Y : fPos1Y;
			float x = fPos1Y < fPos2Y ? fPos1X : fPos2X;
			for (int yStep = (int)yStart; yStep <= (int)yEnd; ++yStep)
			{
				x += 1.0f / k;
				int xStep = (int)x;
				m_pFrameBuffer[xStep + yStep * m_nWidth] = nClearColor;
			}
		}
	}

	void DeviceContext::DrawTriangleWithLine(const VSOutput& v0, const VSOutput& v1, const VSOutput& v2)
	{
		float lineColor[4] = { v0.Color.x, v0.Color.y, v0.Color.z, 1.0f };
		DrawLineWithDDA(v0.SVPosition.x, v0.SVPosition.y, v1.SVPosition.x, v1.SVPosition.y, lineColor);
		DrawLineWithDDA(v1.SVPosition.x, v1.SVPosition.y, v2.SVPosition.x, v2.SVPosition.y, lineColor);
		DrawLineWithDDA(v2.SVPosition.x, v2.SVPosition.y, v0.SVPosition.x, v0.SVPosition.y, lineColor);
	}

	void DeviceContext::DrawTriangleWithFlat(const VSOutput& v0, const VSOutput& v1, const VSOutput& v2)
	{
		if (floatEqual(v0.SVPosition.y, v1.SVPosition.y, fEpsilon) && floatEqual(v0.SVPosition.y, v2.SVPosition.y, fEpsilon) ||
			floatEqual(v0.SVPosition.x, v1.SVPosition.x, fEpsilon) && floatEqual(v0.SVPosition.x, v2.SVPosition.x, fEpsilon))
		{
			return;
		}

		VSOutput vert0(v0);
		VSOutput vert1(v1);
		VSOutput vert2(v2);
		SortTriangleVertsByYGrow(vert0, vert1, vert2);

		if (floatEqual(vert0.SVPosition.y, vert1.SVPosition.y, fEpsilon))
		{
			DrawTopTriangle(vert0, vert1, vert2);
		}
		else if (floatEqual(vert1.SVPosition.y, vert2.SVPosition.y, fEpsilon))
		{
			DrawBottomTriangle(vert0, vert1, vert2);
		}
		else
		{
			VSOutput vertNew;
			SliceTriangleToUpAndBottom(vert0, vert1, vert2, vertNew);

			DrawBottomTriangle(vert0, vert1, vertNew);
			DrawTopTriangle(vert1, vertNew, vert2);
		}
	}

	void DeviceContext::SortTriangleVertsByYGrow(VSOutput& v0, VSOutput& v1, VSOutput& v2)
	{
		if ((uint32_t)v1.SVPosition.y <= (uint32_t)v0.SVPosition.y)
		{
			VSOutput vTemp = v0;
			v0 = v1;
			v1 = vTemp;
		}

		if ((uint32_t)v2.SVPosition.y <= (uint32_t)v0.SVPosition.y)
		{
			VSOutput vTemp = v0;
			v0 = v2;
			v2 = vTemp;
		}

		if ((uint32_t)v2.SVPosition.y <= (uint32_t)v1.SVPosition.y)
		{
			VSOutput vTemp = v1;
			v1 = v2;
			v2 = vTemp;
		}
	}

	void DeviceContext::SortScanlineVertsByXGrow(VSOutput& v0, VSOutput& v1)
	{
		if ((uint32_t)v1.SVPosition.x <= (uint32_t)v0.SVPosition.x)
		{
			VSOutput vTemp = v0;
			v0 = v1;
			v1 = vTemp;
		}
	}

	void DeviceContext::DrawTopTriangle(VSOutput& v0, VSOutput& v1, VSOutput& v2)
	{
		SortScanlineVertsByXGrow(v0, v1);

		float fYStart = std::ceilf(v0.SVPosition.y);
		float fYEnd = std::ceilf(v2.SVPosition.y);
		float fDeltaY = v2.SVPosition.y - v0.SVPosition.y;

		for (uint32_t i = (uint32_t)fYStart; i < (uint32_t)fYEnd; ++i)
		{
			float fLerpFactorY = (i - v0.SVPosition.y) / fDeltaY;

			VSOutput vStart;
			LerpVertexParams(v0, v2, vStart, fLerpFactorY);
			VSOutput vEnd;
			LerpVertexParams(v1, v2, vEnd, fLerpFactorY);

			float fXStart = std::ceil(vStart.SVPosition.x);
			float fXEnd = std::ceil(vEnd.SVPosition.x);

			float fDeltaX = vEnd.SVPosition.x - vStart.SVPosition.x;
			for (uint32_t j = (uint32_t)fXStart; j < (uint32_t)fXEnd; ++j)
			{
				float fLerpFactorX = (j - vStart.SVPosition.x) / fDeltaX;

				VSOutput vCurr;
				LerpVertexParams(vStart, vEnd, vCurr, fLerpFactorX);

				float fPixelDepth = m_pDepthBuffer[j + i * m_nWidth];
				if (vCurr.SVPosition.z <= fPixelDepth)
				{
					/*Vector3 PSResult = m_pPS->PSMain(vCurr, m_pSRV);
					float pixelColor[4] = { PSResult.x, PSResult.y, PSResult.z, 1.0f };
					m_pFrameBuffer[j + i * m_nWidth] = ConvertFloatColorToUInt32(pixelColor);*/
					m_pFrameBuffer[j + i * m_nWidth] = m_pPS->PSMain(vCurr, m_pSRV);

					m_pDepthBuffer[j + i * m_nWidth] = vCurr.SVPosition.z;
				}

#if DEBUG_RASTERIZATION
				m_pDebugBuffer[j + i * m_nWidth]++;
#endif
			}
		}
	}

	void DeviceContext::DrawBottomTriangle(VSOutput& v0, VSOutput& v1, VSOutput& v2)
	{
		SortScanlineVertsByXGrow(v1, v2);

		float fYStart = std::ceilf(v0.SVPosition.y);
		float fYEnd = std::ceilf(v1.SVPosition.y);
		float fDeltaY = v1.SVPosition.y - v0.SVPosition.y;

		for (uint32_t i = (uint32_t)fYStart; i < (uint32_t)fYEnd; ++i)
		{
			float fLerpFactorY = (i - v0.SVPosition.y) / fDeltaY;

			VSOutput vStart;
			LerpVertexParams(v0, v1, vStart, fLerpFactorY);
			VSOutput vEnd;
			LerpVertexParams(v0, v2, vEnd, fLerpFactorY);

			float fXStart = std::ceil(vStart.SVPosition.x);
			float fXEnd = std::ceil(vEnd.SVPosition.x);

			float fDeltaX = vEnd.SVPosition.x - vStart.SVPosition.x;
			for (uint32_t j = (uint32_t)fXStart; j < (uint32_t)fXEnd; ++j)
			{
				float fLerpFactorX = (j - vStart.SVPosition.x) / fDeltaX;

				VSOutput vCurr;
				LerpVertexParams(vStart, vEnd, vCurr, fLerpFactorX);

				float fPixelDepth = m_pDepthBuffer[j + i * m_nWidth];
				if (vCurr.SVPosition.z <= fPixelDepth)
				{
					/*Vector3 PSResult = m_pPS->PSMain(vCurr, m_pSRV);
					float pixelColor[4] = { PSResult.x, PSResult.y, PSResult.z, 1.0f };
					m_pFrameBuffer[j + i * m_nWidth] = ConvertFloatColorToUInt32(pixelColor);*/
					m_pFrameBuffer[j + i * m_nWidth] = m_pPS->PSMain(vCurr, m_pSRV);

					m_pDepthBuffer[j + i * m_nWidth] = vCurr.SVPosition.z;
				}

#if DEBUG_RASTERIZATION
				m_pDebugBuffer[j + i * m_nWidth]++;
#endif
			}
		}
	}

	void DeviceContext::SliceTriangleToUpAndBottom(const VSOutput& v0, const VSOutput& v1, const VSOutput& v2, VSOutput& vNew)
	{
		float fLerpFactor = (v1.SVPosition.y - v0.SVPosition.y) / (v2.SVPosition.y - v0.SVPosition.y);
		
		LerpVertexParams(v0, v2, vNew, fLerpFactor);
	}

	void DeviceContext::LerpVertexParams(const VSOutput& v0, const VSOutput& v1, VSOutput& vNew, float fLerpFactor)
	{
		float fNewX = v0.SVPosition.x + (v1.SVPosition.x - v0.SVPosition.x) * fLerpFactor;
		float fNewY = v0.SVPosition.y + (v1.SVPosition.y - v0.SVPosition.y) * fLerpFactor;
		float fNewZ = v0.SVPosition.z + (v1.SVPosition.z - v0.SVPosition.z) * fLerpFactor;
		float fNewW = v0.SVPosition.w + (v1.SVPosition.w - v0.SVPosition.w) * fLerpFactor;

		vNew.SVPosition = Vector4(fNewX, fNewY, fNewZ, fNewW);
		vNew.Color = (1.0f / fNewW) * ((v0.Color * v0.SVPosition.w) * (1.0f - fLerpFactor) + (v1.Color * v1.SVPosition.w) * fLerpFactor);
		vNew.UV = (1.0f / fNewW) * ((v0.UV * v0.SVPosition.w) * (1.0f - fLerpFactor) + (v1.UV * v1.SVPosition.w) * fLerpFactor);
	}
}