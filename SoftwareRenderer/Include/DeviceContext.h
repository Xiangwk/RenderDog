#pragma once

#pragma once

#include <cstdint>

namespace RenderDog
{
	class VertexBuffer;
	class IndexBuffer;
	class VertexShader;
	class PixelShader;
	class Matrix4x4;
	class RenderTargetView;

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

		void OMSetRenderTarget(RenderTargetView* pRenderTarget);
		void ClearRenderTarget(RenderTargetView* pRenderTarget, const float* ClearColor);
		void Draw();
		void DrawIndex(uint32_t nIndexNum);

		bool CheckDrawPixelTiwce();

	private:
		void DrawLineWithDDA(float fPos1X, float fPos1Y, float fPos2X, float fPos2Y, const float* lineColor);
		void DrawTriangleWithLine(const Vertex& v0, const Vertex& v1, const Vertex& v2);
		void DrawTriangleWithFlat(const Vertex& v0, const Vertex& v1, const Vertex& v2);

		void SortTriangleVertsByYGrow(Vertex& v0, Vertex& v1, Vertex& v2);
		void SortScanlineVertsByXGrow(Vertex& v0, Vertex& v1);

		void DrawTopTriangle(Vertex& v0, Vertex& v1, Vertex& v2, float fDeltaXLeft, float fDeltaXRight);
		void DrawBottomTriangle(Vertex& v0, Vertex& v1, Vertex& v2, float fDeltaXLeft, float fDeltaXRight);

		void SliceTriangleToUpAndBottom(const Vertex& v0, const Vertex& v1, const Vertex& v2, Vertex& vNew);

		inline uint32_t ConvertFloatColorToUInt(const float* color);

	private:
		uint32_t*			m_pFrameBuffer;
		uint32_t*			m_pDebugBuffer;  //检查是否有重复绘制的像素
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