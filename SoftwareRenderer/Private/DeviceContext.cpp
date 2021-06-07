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
#include <cassert>

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
		m_pSRV(nullptr),
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
		m_vClipOutputVerts.clear();
		m_vAssembledVerts.clear();
		m_vClippingVerts.clear();

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

	void DeviceContext::IASetVertexBuffer(VertexBuffer* pVB)
	{
		if (m_pVB != pVB)
		{
			if (m_pVSOutputs)
			{
				delete[] m_pVSOutputs;
				m_pVSOutputs = nullptr;
			}

			uint32_t nVertexNum = pVB->GetNum();
			m_pVSOutputs = new VSOutputVertex[nVertexNum];

			m_pVB = pVB;
		}
	}
	void DeviceContext::IASetIndexBuffer(IndexBuffer* pIB)
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

	void DeviceContext::ClearRenderTarget(RenderTargetView* pRenderTarget, const Vector4& clearColor)
	{
		Vector4 ARGB = ConvertRGBAColorToARGBColor(clearColor);
		uint32_t nClearColor = ConvertColorToUInt32(ARGB);

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
		Vector4 ClearColor = { 1.0f, 0.0f, 0.0f, 1.0f };

		DrawLineWithDDA(100, 100, 100, 100, ClearColor);
	}

	void DeviceContext::DrawIndex(uint32_t nIndexNum)
	{
		const Vertex* pVerts = m_pVB->GetData();
		//Local Space to Clip Space
		for (uint32_t i = 0; i < m_pVB->GetNum(); ++i)
		{
			const Vertex& vert = pVerts[i];
			
			m_pVSOutputs[i] = m_pVS->VSMain(vert, *m_pWorldMat, *m_pViewMat, *m_pProjMat);
		}

		ShapeAssemble(nIndexNum);

		//Clip
		ClipTrianglesInClipSpace();

		//ClipSpace to ScreenSpace
		for (uint32_t i = 0; i < m_vClipOutputVerts.size(); ++i)
		{
			VSOutputVertex& vsOutput = m_vClipOutputVerts[i];
			Vector4 vScreenPos(vsOutput.SVPosition.x, vsOutput.SVPosition.y, vsOutput.SVPosition.z, 1.0f);
			vScreenPos = vScreenPos * (*m_pViewportMat);
			vsOutput.SVPosition.x = vScreenPos.x;
			vsOutput.SVPosition.y = vScreenPos.y;
			vsOutput.SVPosition.z = vScreenPos.z;
		}

		Rasterization();
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
	void DeviceContext::DrawLineWithDDA(float fPos1X, float fPos1Y, float fPos2X, float fPos2Y, const Vector4& lineColor)
	{
		uint32_t nClearColor = ConvertColorToUInt32(lineColor);

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
			for (int xStep = (int)xStart; xStep < (int)xEnd; ++xStep)
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
			for (int yStep = (int)yStart; yStep < (int)yEnd; ++yStep)
			{
				x += 1.0f / k;
				int xStep = (int)x;
				m_pFrameBuffer[xStep + yStep * m_nWidth] = nClearColor;
			}
		}
	}

	void DeviceContext::DrawTriangleWithLine(const VSOutputVertex& v0, const VSOutputVertex& v1, const VSOutputVertex& v2)
	{
		Vector4 lineColor = { v0.Color.x, v0.Color.y, v0.Color.z, 1.0f };
		DrawLineWithDDA(v0.SVPosition.x, v0.SVPosition.y, v1.SVPosition.x, v1.SVPosition.y, lineColor);
		DrawLineWithDDA(v1.SVPosition.x, v1.SVPosition.y, v2.SVPosition.x, v2.SVPosition.y, lineColor);
		DrawLineWithDDA(v2.SVPosition.x, v2.SVPosition.y, v0.SVPosition.x, v0.SVPosition.y, lineColor);
	}

	void DeviceContext::DrawTriangleWithFlat(const VSOutputVertex& v0, const VSOutputVertex& v1, const VSOutputVertex& v2)
	{
		if (floatEqual(v0.SVPosition.y, v1.SVPosition.y, fEpsilon) && floatEqual(v0.SVPosition.y, v2.SVPosition.y, fEpsilon) ||
			floatEqual(v0.SVPosition.x, v1.SVPosition.x, fEpsilon) && floatEqual(v0.SVPosition.x, v2.SVPosition.x, fEpsilon))
		{
			return;
		}

		VSOutputVertex vert0(v0);
		VSOutputVertex vert1(v1);
		VSOutputVertex vert2(v2);
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
			VSOutputVertex vertNew;
			SliceTriangleToUpAndBottom(vert0, vert1, vert2, vertNew);

			DrawBottomTriangle(vert0, vert1, vertNew);
			DrawTopTriangle(vert1, vertNew, vert2);
		}
	}

	void DeviceContext::SortTriangleVertsByYGrow(VSOutputVertex& v0, VSOutputVertex& v1, VSOutputVertex& v2)
	{
		if (v1.SVPosition.y <= v0.SVPosition.y)
		{
			VSOutputVertex vTemp = v0;
			v0 = v1;
			v1 = vTemp;
		}

		if (v2.SVPosition.y <= v0.SVPosition.y)
		{
			VSOutputVertex vTemp = v0;
			v0 = v2;
			v2 = vTemp;
		}

		if (v2.SVPosition.y <= v1.SVPosition.y)
		{
			VSOutputVertex vTemp = v1;
			v1 = v2;
			v2 = vTemp;
		}
	}

	void DeviceContext::SortScanlineVertsByXGrow(VSOutputVertex& v0, VSOutputVertex& v1)
	{
		if (v1.SVPosition.x <= v0.SVPosition.x)
		{
			VSOutputVertex vTemp = v0;
			v0 = v1;
			v1 = vTemp;
		}
	}

	void DeviceContext::DrawTopTriangle(VSOutputVertex& v0, VSOutputVertex& v1, VSOutputVertex& v2)
	{
		SortScanlineVertsByXGrow(v0, v1);

		float fYStart = std::ceilf(v0.SVPosition.y);
		float fYEnd = std::ceilf(v2.SVPosition.y);
		float fDeltaY = v2.SVPosition.y - v0.SVPosition.y;

		for (uint32_t i = (uint32_t)fYStart; i < (uint32_t)fYEnd; ++i)
		{
			float fLerpFactorY = (i - v0.SVPosition.y) / fDeltaY;

			VSOutputVertex vStart;
			LerpVertexParamsInScreen(v0, v2, vStart, fLerpFactorY);
			VSOutputVertex vEnd;
			LerpVertexParamsInScreen(v1, v2, vEnd, fLerpFactorY);

			float fXStart = std::ceil(vStart.SVPosition.x);
			float fXEnd = std::ceil(vEnd.SVPosition.x);

			float fDeltaX = vEnd.SVPosition.x - vStart.SVPosition.x;
			for (uint32_t j = (uint32_t)fXStart; j < (uint32_t)fXEnd; ++j)
			{
				float fLerpFactorX = (j - vStart.SVPosition.x) / fDeltaX;

				VSOutputVertex vCurr;
				LerpVertexParamsInScreen(vStart, vEnd, vCurr, fLerpFactorX);

				float fPixelDepth = m_pDepthBuffer[j + i * m_nWidth];
				if (vCurr.SVPosition.z <= fPixelDepth)
				{
					Vector4 color = m_pPS->PSMain(vCurr, m_pSRV);
					Vector4 ARGB = ConvertRGBAColorToARGBColor(color);
					m_pFrameBuffer[j + i * m_nWidth] = ConvertColorToUInt32(ARGB);

					m_pDepthBuffer[j + i * m_nWidth] = vCurr.SVPosition.z;
				}

#if DEBUG_RASTERIZATION
				m_pDebugBuffer[j + i * m_nWidth]++;
#endif
			}
		}
	}

	void DeviceContext::DrawBottomTriangle(VSOutputVertex& v0, VSOutputVertex& v1, VSOutputVertex& v2)
	{
		SortScanlineVertsByXGrow(v1, v2);

		float fYStart = std::ceilf(v0.SVPosition.y);
		float fYEnd = std::ceilf(v1.SVPosition.y);
		float fDeltaY = v1.SVPosition.y - v0.SVPosition.y;

		for (uint32_t i = (uint32_t)fYStart; i < (uint32_t)fYEnd; ++i)
		{
			float fLerpFactorY = (i - v0.SVPosition.y) / fDeltaY;

			VSOutputVertex vStart;
			LerpVertexParamsInScreen(v0, v1, vStart, fLerpFactorY);
			VSOutputVertex vEnd;
			LerpVertexParamsInScreen(v0, v2, vEnd, fLerpFactorY);

			float fXStart = std::ceil(vStart.SVPosition.x);
			float fXEnd = std::ceil(vEnd.SVPosition.x);

			float fDeltaX = vEnd.SVPosition.x - vStart.SVPosition.x;
			for (uint32_t j = (uint32_t)fXStart; j < (uint32_t)fXEnd; ++j)
			{
				float fLerpFactorX = (j - vStart.SVPosition.x) / fDeltaX;

				VSOutputVertex vCurr;
				LerpVertexParamsInScreen(vStart, vEnd, vCurr, fLerpFactorX);

				float fPixelDepth = m_pDepthBuffer[j + i * m_nWidth];
				if (vCurr.SVPosition.z <= fPixelDepth)
				{
					Vector4 color = m_pPS->PSMain(vCurr, m_pSRV);
					Vector4 ARGB = ConvertRGBAColorToARGBColor(color);
					m_pFrameBuffer[j + i * m_nWidth] = ConvertColorToUInt32(ARGB);

					m_pDepthBuffer[j + i * m_nWidth] = vCurr.SVPosition.z;
				}

#if DEBUG_RASTERIZATION
				m_pDebugBuffer[j + i * m_nWidth]++;
#endif
			}
		}
	}

	void DeviceContext::SliceTriangleToUpAndBottom(const VSOutputVertex& v0, const VSOutputVertex& v1, const VSOutputVertex& v2, VSOutputVertex& vNew)
	{
		float fLerpFactor = (v1.SVPosition.y - v0.SVPosition.y) / (v2.SVPosition.y - v0.SVPosition.y);
		
		LerpVertexParamsInScreen(v0, v2, vNew, fLerpFactor);
	}

	void DeviceContext::LerpVertexParamsInScreen(const VSOutputVertex& vStart, const VSOutputVertex& vEnd, VSOutputVertex& vNew, float fLerpFactor)
	{
		float fNewX = vStart.SVPosition.x + (vEnd.SVPosition.x - vStart.SVPosition.x) * fLerpFactor;
		float fNewY = vStart.SVPosition.y + (vEnd.SVPosition.y - vStart.SVPosition.y) * fLerpFactor;
		float fNewZ = vStart.SVPosition.z + (vEnd.SVPosition.z - vStart.SVPosition.z) * fLerpFactor;
		float fNewW = 1.0f / (1.0f / vStart.SVPosition.w + (1.0f / vEnd.SVPosition.w - 1.0f / vStart.SVPosition.w) * fLerpFactor);

		vNew.SVPosition = Vector4(fNewX, fNewY, fNewZ, fNewW);
		vNew.Color = fNewW * ((vStart.Color / vStart.SVPosition.w) * (1.0f - fLerpFactor) + (vEnd.Color / vEnd.SVPosition.w) * fLerpFactor);
		vNew.Normal = fNewW * ((vStart.Normal / vStart.SVPosition.w) * (1.0f - fLerpFactor) + (vEnd.Normal / vEnd.SVPosition.w) * fLerpFactor);
		vNew.Tangent = fNewW * ((vStart.Tangent / vStart.SVPosition.w) * (1.0f - fLerpFactor) + (vEnd.Tangent / vEnd.SVPosition.w) * fLerpFactor);
		vNew.Texcoord = fNewW * ((vStart.Texcoord / vStart.SVPosition.w) * (1.0f - fLerpFactor) + (vEnd.Texcoord / vEnd.SVPosition.w) * fLerpFactor);
	}

	void DeviceContext::LerpVertexParamsInClip(const VSOutputVertex& vStart, const VSOutputVertex& vEnd, VSOutputVertex& vNew, float fLerpFactor)
	{
		float fNewX = vStart.SVPosition.x + (vEnd.SVPosition.x - vStart.SVPosition.x) * fLerpFactor;
		float fNewY = vStart.SVPosition.y + (vEnd.SVPosition.y - vStart.SVPosition.y) * fLerpFactor;
		float fNewZ = vStart.SVPosition.z + (vEnd.SVPosition.z - vStart.SVPosition.z) * fLerpFactor;
		float fNewW = vStart.SVPosition.w + (vEnd.SVPosition.w - vStart.SVPosition.w) * fLerpFactor;

		vNew.SVPosition = Vector4(fNewX, fNewY, fNewZ, fNewW);
		vNew.Color = vStart.Color * (1.0f - fLerpFactor) + vEnd.Color * fLerpFactor;
		vNew.Normal = vStart.Normal * (1.0f - fLerpFactor) + vEnd.Normal * fLerpFactor;
		vNew.Tangent = vStart.Tangent * (1.0f - fLerpFactor) + vEnd.Tangent * fLerpFactor;
		vNew.Texcoord = vStart.Texcoord * (1.0f - fLerpFactor) + vEnd.Texcoord * fLerpFactor;
	}

	void DeviceContext::ClipTrianglesInClipSpace()
	{
		m_vClipOutputVerts.clear();
		if (m_vClipOutputVerts.capacity() < m_vAssembledVerts.capacity())
		{
			m_vClipOutputVerts.reserve(m_vAssembledVerts.capacity());
		}

		for (uint32_t i = 0; i < m_vAssembledVerts.size(); i += 3)
		{
			m_vClippingVerts.clear();

			const VSOutputVertex& vert0 = m_vAssembledVerts[i];
			const VSOutputVertex& vert1 = m_vAssembledVerts[i + 1];
			const VSOutputVertex& vert2 = m_vAssembledVerts[i + 2];

			m_vClippingVerts.push_back(vert0);
			m_vClippingVerts.push_back(vert1);
			m_vClippingVerts.push_back(vert2);

			ClipTriangleWithPlaneX(1);
			ClipTriangleWithPlaneX(-1);
			ClipTriangleWithPlaneY(1);
			ClipTriangleWithPlaneY(-1);
			ClipTriangleWithPlaneZeroZ();
			ClipTriangleWithPlanePositiveZ();

			for (uint32_t i = 0; i < m_vClippingVerts.size(); ++i)
			{
				m_vClipOutputVerts.push_back(m_vClippingVerts[i]);
			}
		}

		for (uint32_t i = 0; i < m_vClipOutputVerts.size(); ++i)
		{
			m_vClipOutputVerts[i].SVPosition.x /= m_vClipOutputVerts[i].SVPosition.w;
			m_vClipOutputVerts[i].SVPosition.y /= m_vClipOutputVerts[i].SVPosition.w;
			m_vClipOutputVerts[i].SVPosition.z /= m_vClipOutputVerts[i].SVPosition.w;
		}
	}

	void DeviceContext::ClipTriangleWithPlaneX(int nSign)
	{
		std::vector<VSOutputVertex> vCurrClipResultVerts = {};

		for (uint32_t i = 0; i < m_vClippingVerts.size(); i += 3)
		{
			int nOutOfClipPlaneNum = 0;

			VSOutputVertex& vert0 = m_vClippingVerts[i];
			VSOutputVertex& vert1 = m_vClippingVerts[i + 1];
			VSOutputVertex& vert2 = m_vClippingVerts[i + 2];

			nSign * vert0.SVPosition.x > vert0.SVPosition.w ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;
			nSign* vert1.SVPosition.x > vert1.SVPosition.w ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;
			nSign* vert2.SVPosition.x > vert2.SVPosition.w ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;

			if (nOutOfClipPlaneNum == 0)
			{
				vCurrClipResultVerts.push_back(vert0);
				vCurrClipResultVerts.push_back(vert1);
				vCurrClipResultVerts.push_back(vert2);

				continue;
			}
			else if(nOutOfClipPlaneNum == 3)
			{
				continue;
			}
			else if(nOutOfClipPlaneNum == 2)
			{
				if (nSign * vert0.SVPosition.x < vert0.SVPosition.w)
				{
					float fLerpFactor1 = GetClipLerpFactorX(vert0, vert1, nSign);
					float fLerpFactor2 = GetClipLerpFactorX(vert0, vert2, nSign);
					ClipTwoVertsInTriangle(vert0, vert1, vert2, fLerpFactor1, fLerpFactor2);
				}
				else if (nSign * vert1.SVPosition.x < vert1.SVPosition.w)
				{
					float fLerpFactor1 = GetClipLerpFactorX(vert1, vert2, nSign);
					float fLerpFactor2 = GetClipLerpFactorX(vert1, vert0, nSign);
					ClipTwoVertsInTriangle(vert1, vert2, vert0, fLerpFactor1, fLerpFactor2);
				}
				else
				{
					float fLerpFactor1 = GetClipLerpFactorX(vert2, vert0, nSign);
					float fLerpFactor2 = GetClipLerpFactorX(vert2, vert1, nSign);
					ClipTwoVertsInTriangle(vert2, vert0, vert1, fLerpFactor1, fLerpFactor2);
				}

				vCurrClipResultVerts.push_back(vert0);
				vCurrClipResultVerts.push_back(vert1);
				vCurrClipResultVerts.push_back(vert2);

				continue;
			}
			else
			{
				if (nSign * vert0.SVPosition.x > vert0.SVPosition.w)
				{
					float fLerpFactor1 = GetClipLerpFactorX(vert1, vert0, nSign);
					float fLerpFactor2 = GetClipLerpFactorX(vert2, vert0, nSign);
					ClipOneVertInTriangle(vert0, vert1, vert2, fLerpFactor1, fLerpFactor2, vCurrClipResultVerts);
				}
				else if (nSign * vert1.SVPosition.x > vert1.SVPosition.w)
				{
					float fLerpFactor1 = GetClipLerpFactorX(vert2, vert1, nSign);
					float fLerpFactor2 = GetClipLerpFactorX(vert0, vert1, nSign);
					ClipOneVertInTriangle(vert1, vert2, vert0, fLerpFactor1, fLerpFactor2, vCurrClipResultVerts);
				}
				else
				{
					float fLerpFactor1 = GetClipLerpFactorX(vert0, vert2, nSign);
					float fLerpFactor2 = GetClipLerpFactorX(vert1, vert2, nSign);
					ClipOneVertInTriangle(vert2, vert0, vert1, fLerpFactor1, fLerpFactor2, vCurrClipResultVerts);
				}

				vCurrClipResultVerts.push_back(vert0);
				vCurrClipResultVerts.push_back(vert1);
				vCurrClipResultVerts.push_back(vert2);

				continue;
			}
		}

		m_vClippingVerts.clear();
		for (uint32_t i = 0; i < vCurrClipResultVerts.size(); ++i)
		{
			m_vClippingVerts.push_back(vCurrClipResultVerts[i]);
		}
	}
	
	void DeviceContext::ClipTriangleWithPlaneY(int nSign)
	{
		std::vector<VSOutputVertex> vCurrClipResultVerts = {};

		for (uint32_t i = 0; i < m_vClippingVerts.size(); i += 3)
		{
			int nOutOfClipPlaneNum = 0;

			VSOutputVertex& vert0 = m_vClippingVerts[i];
			VSOutputVertex& vert1 = m_vClippingVerts[i + 1];
			VSOutputVertex& vert2 = m_vClippingVerts[i + 2];

			nSign * vert0.SVPosition.y > vert0.SVPosition.w ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;
			nSign * vert1.SVPosition.y > vert1.SVPosition.w ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;
			nSign * vert2.SVPosition.y > vert2.SVPosition.w ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;

			if (nOutOfClipPlaneNum == 0)
			{
				vCurrClipResultVerts.push_back(vert0);
				vCurrClipResultVerts.push_back(vert1);
				vCurrClipResultVerts.push_back(vert2);

				continue;
			}
			else if (nOutOfClipPlaneNum == 3)
			{
				continue;
			}
			else if (nOutOfClipPlaneNum == 2)
			{
				if (nSign * vert0.SVPosition.y < vert0.SVPosition.w)
				{
					float fLerpFactor1 = GetClipLerpFactorY(vert0, vert1, nSign);
					float fLerpFactor2 = GetClipLerpFactorY(vert0, vert2, nSign);
					ClipTwoVertsInTriangle(vert0, vert1, vert2, fLerpFactor1, fLerpFactor2);
				}
				else if (nSign * vert1.SVPosition.y < vert1.SVPosition.w)
				{
					float fLerpFactor1 = GetClipLerpFactorY(vert1, vert2, nSign);
					float fLerpFactor2 = GetClipLerpFactorY(vert1, vert0, nSign);
					ClipTwoVertsInTriangle(vert1, vert2, vert0, fLerpFactor1, fLerpFactor2);
				}
				else
				{
					float fLerpFactor1 = GetClipLerpFactorY(vert2, vert0, nSign);
					float fLerpFactor2 = GetClipLerpFactorY(vert2, vert1, nSign);
					ClipTwoVertsInTriangle(vert2, vert0, vert1, fLerpFactor1, fLerpFactor2);
				}

				vCurrClipResultVerts.push_back(vert0);
				vCurrClipResultVerts.push_back(vert1);
				vCurrClipResultVerts.push_back(vert2);

				continue;
			}
			else
			{
				if (nSign * vert0.SVPosition.y > vert0.SVPosition.w)
				{
					float fLerpFactor1 = GetClipLerpFactorY(vert1, vert0, nSign);
					float fLerpFactor2 = GetClipLerpFactorY(vert2, vert0, nSign);
					ClipOneVertInTriangle(vert0, vert1, vert2, fLerpFactor1, fLerpFactor2, vCurrClipResultVerts);
				}
				else if (nSign * vert1.SVPosition.y > vert1.SVPosition.w)
				{
					float fLerpFactor1 = GetClipLerpFactorY(vert2, vert1, nSign);
					float fLerpFactor2 = GetClipLerpFactorY(vert0, vert1, nSign);
					ClipOneVertInTriangle(vert1, vert2, vert0, fLerpFactor1, fLerpFactor2, vCurrClipResultVerts);
				}
				else
				{
					float fLerpFactor1 = GetClipLerpFactorY(vert0, vert2, nSign);
					float fLerpFactor2 = GetClipLerpFactorY(vert1, vert2, nSign);
					ClipOneVertInTriangle(vert2, vert0, vert1, fLerpFactor1, fLerpFactor2, vCurrClipResultVerts);
				}

				vCurrClipResultVerts.push_back(vert0);
				vCurrClipResultVerts.push_back(vert1);
				vCurrClipResultVerts.push_back(vert2);

				continue;
			}
		}

		m_vClippingVerts.clear();
		for (uint32_t i = 0; i < vCurrClipResultVerts.size(); ++i)
		{
			m_vClippingVerts.push_back(vCurrClipResultVerts[i]);
		}
	}

	void DeviceContext::ClipTriangleWithPlaneZeroZ()
	{
		std::vector<VSOutputVertex> vCurrClipResultVerts = {};

		for (uint32_t i = 0; i < m_vClippingVerts.size(); i += 3)
		{
			int nOutOfClipPlaneNum = 0;

			VSOutputVertex& vert0 = m_vClippingVerts[i];
			VSOutputVertex& vert1 = m_vClippingVerts[i + 1];
			VSOutputVertex& vert2 = m_vClippingVerts[i + 2];
			
			vert0.SVPosition.z < 0.0f ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;
			vert1.SVPosition.z < 0.0f ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;
			vert2.SVPosition.z < 0.0f ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;
			
			if (nOutOfClipPlaneNum == 0)
			{
				vCurrClipResultVerts.push_back(vert0);
				vCurrClipResultVerts.push_back(vert1);
				vCurrClipResultVerts.push_back(vert2);

				continue;
			}
			else if (nOutOfClipPlaneNum == 3)
			{
				continue;
			}
			else if (nOutOfClipPlaneNum == 2)
			{
				if (vert0.SVPosition.z > 0.0f)
				{
					float fLerpFactor1 = GetClipLerpFactorZeroZ(vert0, vert1);
					float fLerpFactor2 = GetClipLerpFactorZeroZ(vert0, vert2);
					ClipTwoVertsInTriangle(vert0, vert1, vert2, fLerpFactor1, fLerpFactor2);
				}
				else if (vert1.SVPosition.z > 0.0f)
				{
					float fLerpFactor1 = GetClipLerpFactorZeroZ(vert1, vert2);
					float fLerpFactor2 = GetClipLerpFactorZeroZ(vert1, vert0);
					ClipTwoVertsInTriangle(vert1, vert2, vert0, fLerpFactor1, fLerpFactor2);
				}
				else
				{
					float fLerpFactor1 = GetClipLerpFactorZeroZ(vert2, vert0);
					float fLerpFactor2 = GetClipLerpFactorZeroZ(vert2, vert1);
					ClipTwoVertsInTriangle(vert2, vert0, vert1, fLerpFactor1, fLerpFactor2);
				}

				vCurrClipResultVerts.push_back(vert0);
				vCurrClipResultVerts.push_back(vert1);
				vCurrClipResultVerts.push_back(vert2);

				continue;
			}
			else
			{
				if (vert0.SVPosition.z < 0.0f)
				{
					float fLerpFactor1 = GetClipLerpFactorZeroZ(vert1, vert0);
					float fLerpFactor2 = GetClipLerpFactorZeroZ(vert2, vert0);
					ClipOneVertInTriangle(vert0, vert1, vert2, fLerpFactor1, fLerpFactor2, vCurrClipResultVerts);
				}
				else if (vert1.SVPosition.z < 0.0f)
				{
					float fLerpFactor1 = GetClipLerpFactorZeroZ(vert2, vert1);
					float fLerpFactor2 = GetClipLerpFactorZeroZ(vert0, vert1);
					ClipOneVertInTriangle(vert1, vert2, vert0, fLerpFactor1, fLerpFactor2, vCurrClipResultVerts);
				}
				else
				{
					float fLerpFactor1 = GetClipLerpFactorZeroZ(vert0, vert2);
					float fLerpFactor2 = GetClipLerpFactorZeroZ(vert1, vert2);
					ClipOneVertInTriangle(vert2, vert0, vert1, fLerpFactor1, fLerpFactor2, vCurrClipResultVerts);
				}

				vCurrClipResultVerts.push_back(vert0);
				vCurrClipResultVerts.push_back(vert1);
				vCurrClipResultVerts.push_back(vert2);

				continue;
			}
		}

		m_vClippingVerts.clear();
		for (uint32_t i = 0; i < vCurrClipResultVerts.size(); ++i)
		{
			m_vClippingVerts.push_back(vCurrClipResultVerts[i]);
		}
	}

	void DeviceContext::ClipTriangleWithPlanePositiveZ()
	{
		std::vector<VSOutputVertex> vCurrClipResultVerts = {};

		for (uint32_t i = 0; i < m_vClippingVerts.size(); i += 3)
		{
			int nOutOfClipPlaneNum = 0;

			VSOutputVertex& vert0 = m_vClippingVerts[i];
			VSOutputVertex& vert1 = m_vClippingVerts[i + 1];
			VSOutputVertex& vert2 = m_vClippingVerts[i + 2];

			vert0.SVPosition.z > vert0.SVPosition.w ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;
			vert1.SVPosition.z > vert1.SVPosition.w ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;
			vert2.SVPosition.z > vert2.SVPosition.w ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;

			if (nOutOfClipPlaneNum == 0)
			{
				vCurrClipResultVerts.push_back(vert0);
				vCurrClipResultVerts.push_back(vert1);
				vCurrClipResultVerts.push_back(vert2);

				continue;
			}
			else if (nOutOfClipPlaneNum == 3)
			{
				continue;
			}
			else if (nOutOfClipPlaneNum == 2)
			{
				if (vert0.SVPosition.z < vert0.SVPosition.w)
				{
					float fLerpFactor1 = GetClipLerpFactorPositiveZ(vert0, vert1);
					float fLerpFactor2 = GetClipLerpFactorPositiveZ(vert0, vert2);
					ClipTwoVertsInTriangle(vert0, vert1, vert2, fLerpFactor1, fLerpFactor2);
				}
				else if (vert1.SVPosition.z < vert1.SVPosition.w)
				{
					float fLerpFactor1 = GetClipLerpFactorPositiveZ(vert1, vert2);
					float fLerpFactor2 = GetClipLerpFactorPositiveZ(vert1, vert0);
					ClipTwoVertsInTriangle(vert1, vert2, vert0, fLerpFactor1, fLerpFactor2);
				}
				else
				{
					float fLerpFactor1 = GetClipLerpFactorPositiveZ(vert2, vert0);
					float fLerpFactor2 = GetClipLerpFactorPositiveZ(vert2, vert1);
					ClipTwoVertsInTriangle(vert2, vert0, vert1, fLerpFactor1, fLerpFactor2);
				}

				vCurrClipResultVerts.push_back(vert0);
				vCurrClipResultVerts.push_back(vert1);
				vCurrClipResultVerts.push_back(vert2);

				continue;
			}
			else
			{
				if (vert0.SVPosition.z > vert0.SVPosition.w)
				{
					float fLerpFactor1 = GetClipLerpFactorPositiveZ(vert1, vert0);
					float fLerpFactor2 = GetClipLerpFactorPositiveZ(vert2, vert0);
					ClipOneVertInTriangle(vert0, vert1, vert2, fLerpFactor1, fLerpFactor2, vCurrClipResultVerts);
				}
				else if (vert1.SVPosition.z > vert1.SVPosition.w)
				{
					float fLerpFactor1 = GetClipLerpFactorPositiveZ(vert2, vert1);
					float fLerpFactor2 = GetClipLerpFactorPositiveZ(vert0, vert1);
					ClipOneVertInTriangle(vert1, vert2, vert0, fLerpFactor1, fLerpFactor2, vCurrClipResultVerts);
				}
				else
				{
					float fLerpFactor1 = GetClipLerpFactorPositiveZ(vert0, vert2);
					float fLerpFactor2 = GetClipLerpFactorPositiveZ(vert1, vert2);
					ClipOneVertInTriangle(vert2, vert0, vert1, fLerpFactor1, fLerpFactor2, vCurrClipResultVerts);
				}

				vCurrClipResultVerts.push_back(vert0);
				vCurrClipResultVerts.push_back(vert1);
				vCurrClipResultVerts.push_back(vert2);

				continue;
			}
		}

		m_vClippingVerts.clear();
		for (uint32_t i = 0; i < vCurrClipResultVerts.size(); ++i)
		{
			m_vClippingVerts.push_back(vCurrClipResultVerts[i]);
		}
	}

	void DeviceContext::ClipTwoVertsInTriangle(const VSOutputVertex& vertIn, VSOutputVertex& vertOut1, VSOutputVertex& vertOut2, float fLerpFactor1, float fLerpFactor2)
	{
		VSOutputVertex vertNew1;
		LerpVertexParamsInClip(vertIn, vertOut1, vertNew1, fLerpFactor1);

		VSOutputVertex vertNew2;
		LerpVertexParamsInClip(vertIn, vertOut2, vertNew2, fLerpFactor2);

		vertOut1 = vertNew1;
		vertOut2 = vertNew2;
	}

	void DeviceContext::ClipOneVertInTriangle(VSOutputVertex& vertOut, const VSOutputVertex& vertIn1, const VSOutputVertex& vertIn2, float fLerpFactor1, float fLerpFactor2, std::vector<VSOutputVertex>& vTempVerts)
	{
		VSOutputVertex vertNew1;
		LerpVertexParamsInClip(vertIn1, vertOut, vertNew1, fLerpFactor1);

		VSOutputVertex vertNew2;
		LerpVertexParamsInClip(vertIn2, vertOut, vertNew2, fLerpFactor2);

		vertOut = vertNew2;
		vTempVerts.push_back(vertNew2);
		vTempVerts.push_back(vertNew1);
		vTempVerts.push_back(vertIn1);
	}

	float DeviceContext::GetClipLerpFactorX(const VSOutputVertex& vert0, const VSOutputVertex& vert1, int fSign)
	{
		return (vert0.SVPosition.x - fSign * vert0.SVPosition.w) / (fSign * vert1.SVPosition.w - fSign * vert0.SVPosition.w - vert1.SVPosition.x + vert0.SVPosition.x);
	}

	float DeviceContext::GetClipLerpFactorY(const VSOutputVertex& vert0, const VSOutputVertex& vert1, int fSign)
	{
		return (vert0.SVPosition.y - fSign * vert0.SVPosition.w) / (fSign * vert1.SVPosition.w - fSign * vert0.SVPosition.w - vert1.SVPosition.y + vert0.SVPosition.y);
	}

	float DeviceContext::GetClipLerpFactorZeroZ(const VSOutputVertex& vert0, const VSOutputVertex& vert1)
	{
		return (-vert0.SVPosition.z / (vert1.SVPosition.z - vert0.SVPosition.z));
	}

	float DeviceContext::GetClipLerpFactorPositiveZ(const VSOutputVertex& vert0, const VSOutputVertex& vert1)
	{
		return (vert0.SVPosition.z - vert0.SVPosition.w) / (vert1.SVPosition.w - vert0.SVPosition.w - vert1.SVPosition.z + vert0.SVPosition.z);
	}

	void DeviceContext::ShapeAssemble(uint32_t nIndexNum)
	{
		m_vAssembledVerts.clear();
		if (m_vAssembledVerts.capacity() < nIndexNum)
		{
			m_vAssembledVerts.reserve(nIndexNum);
		}

		if (m_PriTopology == PrimitiveTopology::LINE_LIST || m_PriTopology == PrimitiveTopology::TRIANGLE_LIST)
		{
			const uint32_t* pIndice = m_pIB->GetData();
			for (uint32_t i = 0; i < nIndexNum; ++i)
			{
				const VSOutputVertex& vert0 = m_pVSOutputs[pIndice[i]];

				m_vAssembledVerts.push_back(vert0);
			}
		}
	}

	void DeviceContext::Rasterization()
	{
		for (uint32_t i = 0; i < m_vClipOutputVerts.size(); i += 3)
		{
			const VSOutputVertex& vert0 = m_vClipOutputVerts[i];
			const VSOutputVertex& vert1 = m_vClipOutputVerts[i + 1];
			const VSOutputVertex& vert2 = m_vClipOutputVerts[i + 2];

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
}