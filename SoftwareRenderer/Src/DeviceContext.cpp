#include "DeviceContext.h"
#include "Matrix.h"
#include "Vertex.h"
#include "Buffer.h"
#include "Viewport.h"
#include "RenderTargetView.h"
#include "Shader.h"
#include "Utility.h"

namespace RenderDog
{
	extern const float fEpsilon;

	DeviceContext::DeviceContext(uint32_t width, uint32_t height) :
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
		m_pViewportMat(nullptr),
		m_PriTopology(PrimitiveTopology::TRIANGLE_LIST)
	{}

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
			else if (m_PriTopology == PrimitiveTopology::TRIANGLE_LIST)
			{
				DrawTriangleWithFlat(vert0, vert1, vert2);
			}
		}
	}


	//------------------------------------------------------------------------------------------------------------------
	//Private Funtion
	//------------------------------------------------------------------------------------------------------------------
	void DeviceContext::DrawLineWithDDA(float fPos1X, float fPos1Y, float fPos2X, float fPos2Y, const float* lineColor)
	{
		uint32_t nClearColor = ConvertFloatColorToUInt(lineColor);

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

	void DeviceContext::DrawTriangleWithLine(const Vertex& v0, const Vertex& v1, const Vertex& v2)
	{
		float lineColor[4] = { v0.vColor.x, v0.vColor.y, v0.vColor.z, 1.0f };
		DrawLineWithDDA(v0.vPostion.x, v0.vPostion.y, v1.vPostion.x, v1.vPostion.y, lineColor);
		DrawLineWithDDA(v1.vPostion.x, v1.vPostion.y, v2.vPostion.x, v2.vPostion.y, lineColor);
		DrawLineWithDDA(v2.vPostion.x, v2.vPostion.y, v0.vPostion.x, v0.vPostion.y, lineColor);
	}

	void DeviceContext::DrawTriangleWithFlat(const Vertex& v0, const Vertex& v1, const Vertex& v2)
	{
		if (floatEqual(v0.vPostion.y, v1.vPostion.y, fEpsilon) && floatEqual(v0.vPostion.y, v2.vPostion.y, fEpsilon) ||
			floatEqual(v0.vPostion.x, v1.vPostion.x, fEpsilon) && floatEqual(v0.vPostion.x, v2.vPostion.x, fEpsilon))
		{
			return;
		}

		Vertex vert0(v0);
		Vertex vert1(v1);
		Vertex vert2(v2);
		SortTriangleVertsByYGrow(vert0, vert1, vert2);

		if (floatEqual(vert0.vPostion.y, vert1.vPostion.y, fEpsilon))
		{
			DrawTopTriangle(vert0, vert1, vert2);
		}
		else if (floatEqual(vert1.vPostion.y, vert2.vPostion.y, fEpsilon))
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
		if ((uint32_t)v1.vPostion.y <= (uint32_t)v0.vPostion.y)
		{
			Vertex vTemp = v0;
			v0 = v1;
			v1 = vTemp;
		}

		if ((uint32_t)v2.vPostion.y <= (uint32_t)v0.vPostion.y)
		{
			Vertex vTemp = v0;
			v0 = v2;
			v2 = vTemp;
		}

		if ((uint32_t)v2.vPostion.y <= (uint32_t)v1.vPostion.y)
		{
			Vertex vTemp = v1;
			v1 = v2;
			v2 = vTemp;
		}
	}

	void DeviceContext::SortScanlineVertsByXGrow(Vertex& v0, Vertex& v1)
	{
		if ((uint32_t)v1.vPostion.x <= (uint32_t)v0.vPostion.x)
		{
			Vertex vTemp = v0;
			v0 = v1;
			v1 = vTemp;
		}
	}

	void DeviceContext::DrawTopTriangle(Vertex& v0, Vertex& v1, Vertex& v2)
	{
		SortScanlineVertsByXGrow(v0, v1);

		float fDeltaY = v2.vPostion.y - v0.vPostion.y;
		float fDeltaXLeft = (v2.vPostion.x - v0.vPostion.x) / fDeltaY;
		float fDeltaXRight = (v2.vPostion.x - v1.vPostion.x) / fDeltaY;

		float fYStart = std::ceilf(v0.vPostion.y - 0.5f);
		float fYEnd = std::ceilf(v2.vPostion.y - 0.5f);
		for (uint32_t i = (uint32_t)fYStart; i < (uint32_t)fYEnd; ++i)
		{
			float fYStep = (float)i - fYStart;
			float fXStart = std::ceilf(v0.vPostion.x + fYStep * fDeltaXLeft - 0.5f);
			float fXEnd = std::ceilf(v1.vPostion.x + fYStep * fDeltaXRight - 0.5f);
			for (uint32_t j = (uint32_t)fXStart; j < (uint32_t)fXEnd; ++j)
			{
				Vector3 vColor = Vector3(1.0f, 0.0f, 0.0f);
				float pixelColor[4] = { vColor.x, vColor.y, vColor.z, 1.0f };
				m_pFrameBuffer[j + i * m_nWidth] = ConvertFloatColorToUInt(pixelColor);
			}
		}
	}

	void DeviceContext::DrawBottomTriangle(Vertex& v0, Vertex& v1, Vertex& v2)
	{
		SortScanlineVertsByXGrow(v1, v2);

		float fDeltaY = v2.vPostion.y - v0.vPostion.y;
		float fDeltaXLeft = (v1.vPostion.x - v0.vPostion.x) / fDeltaY;
		float fDeltaXRight = (v2.vPostion.x - v0.vPostion.x) / fDeltaY;

		float fYStart = std::ceilf(v0.vPostion.y - 0.5f);
		float fYEnd = std::ceilf(v2.vPostion.y - 0.5f);
		for (uint32_t i = (uint32_t)fYStart; i < (uint32_t)fYEnd; ++i)
		{
			float fYStep = (float)i - fYStart;
			float fXStart = std::ceilf(v0.vPostion.x + fYStep * fDeltaXLeft - 0.5f);
			float fXEnd = std::ceilf(v0.vPostion.x + fYStep * fDeltaXRight - 0.5f);
			for (uint32_t j = (uint32_t)fXStart; j < (uint32_t)fXEnd; ++j)
			{
				Vector3 vColor = Vector3(1.0f, 0.0f, 0.0f);
				float pixelColor[4] = { vColor.x, vColor.y, vColor.z, 1.0f };
				m_pFrameBuffer[j + i * m_nWidth] = ConvertFloatColorToUInt(pixelColor);
			}
		}
	}

	void DeviceContext::SliceTriangleToUpAndBottom(const Vertex& v0, const Vertex& v1, const Vertex& v2, Vertex& vNew)
	{
		float fLerpFactor = (v1.vPostion.y - v0.vPostion.y) / (v2.vPostion.y - v0.vPostion.y);

		float fNewX = v0.vPostion.x + fLerpFactor * (v2.vPostion.x - v0.vPostion.x);
		float fInvNewZ = 1.0f / v0.vPostion.z + fLerpFactor * (1.0f / v2.vPostion.z - 1.0f / v0.vPostion.z);
		float fNewZ = 1.0f / fInvNewZ;

		Vector3 vNewPos(fNewX, v1.vPostion.y, fNewZ);

		Vector3 vNewColor = v0.vColor + fLerpFactor * (v2.vColor - v0.vColor);

		vNew.vPostion = vNewPos;
		vNew.vColor = vNewColor;
	}

	inline uint32_t DeviceContext::ConvertFloatColorToUInt(const float* color)
	{
		return (uint32_t)(255 * color[0]) << 16 | (uint32_t)(255 * color[1]) << 8 | (uint32_t)(255 * color[2]);
	}

}