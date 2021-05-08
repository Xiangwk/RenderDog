#pragma once

#pragma once

#include <cstdint>

#define DEBUG_RASTERIZATION 0

namespace RenderDog
{
	class VertexBuffer;
	class IndexBuffer;
	class VertexShader;
	class PixelShader;
	class Matrix4x4;
	class RenderTargetView;
	class DepthStencilView;

	struct Viewport;
	struct Vertex;

	enum class PrimitiveTopology
	{
		LINE_LIST,
		TRIANGLE_LIST
	};

	class DeviceContext
	{
	public:
		DeviceContext(uint32_t width, uint32_t height);
		~DeviceContext();

		void IASetVertexBuffer(const VertexBuffer* pVB);
		void IASetIndexBuffer(const IndexBuffer* pIB);
		void IASetPrimitiveTopology(PrimitiveTopology topology) { m_PriTopology = topology; }

		void VSSetShader(const VertexShader* pVS) { m_pVS = pVS; }
		void VSSetTransMats(const Matrix4x4* matWorld, const Matrix4x4* matView, const Matrix4x4* matProj);
		void PSSetShader(const PixelShader* pPS) { m_pPS = pPS; }

		void RSSetViewport(const Viewport* pVP);

		void OMSetRenderTarget(RenderTargetView* pRenderTarget, DepthStencilView* pDepthStencil);
		void ClearRenderTarget(RenderTargetView* pRenderTarget, const float* ClearColor);
		void ClearDepthStencil(DepthStencilView* pDepthStencil, float fDepth);
		void Draw();
		void DrawIndex(uint32_t nIndexNum);

#if DEBUG_RASTERIZATION
		bool CheckDrawPixelTiwce();
#endif

	private:
		void DrawLineWithDDA(float fPos1X, float fPos1Y, float fPos2X, float fPos2Y, const float* lineColor);
		void DrawTriangleWithLine(const Vertex& v0, const Vertex& v1, const Vertex& v2);
		void DrawTriangleWithFlat(const Vertex& v0, const Vertex& v1, const Vertex& v2);

		void SortTriangleVertsByYGrow(Vertex& v0, Vertex& v1, Vertex& v2);
		void SortScanlineVertsByXGrow(Vertex& v0, Vertex& v1);

		//平定三角形和平底三角形
		void DrawTopTriangle(Vertex& v0, Vertex& v1, Vertex& v2);
		void DrawBottomTriangle(Vertex& v0, Vertex& v1, Vertex& v2);

		void SliceTriangleToUpAndBottom(const Vertex& v0, const Vertex& v1, const Vertex& v2, Vertex& vNew);

		void LerpVertexParams(const Vertex& v0, const Vertex& v1, Vertex& vNew, float fLerpFactor);

		inline uint32_t ConvertFloatColorToUInt32(const float* color);

	private:
		uint32_t*			m_pFrameBuffer;
		float*				m_pDepthBuffer;
#if DEBUG_RASTERIZATION
		uint32_t*			m_pDebugBuffer;  //检查是否有重复绘制的像素
#endif
		uint32_t			m_nWidth;
		uint32_t			m_nHeight;

		const VertexBuffer* m_pVB;
		const IndexBuffer* m_pIB;

		const VertexShader* m_pVS;
		const PixelShader* m_pPS;

		const Matrix4x4* m_pWorldMat;
		const Matrix4x4* m_pViewMat;
		const Matrix4x4* m_pProjMat;

		Vertex* m_pVSOutputs;

		Matrix4x4* m_pViewportMat;

		PrimitiveTopology	m_PriTopology;
	};
}