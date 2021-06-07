#pragma once

#include <cstdint>
#include <vector>

#define DEBUG_RASTERIZATION 0

namespace RenderDog
{
	class VertexBuffer;
	class IndexBuffer;
	class VertexShader;
	class PixelShader;
	class Vector4;
	class Matrix4x4;
	class RenderTargetView;
	class DepthStencilView;
	class ShaderResourceView;

	struct Viewport;
	struct Vertex;
	struct VSOutputVertex;

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

		void IASetVertexBuffer(VertexBuffer* pVB);
		void IASetIndexBuffer(IndexBuffer* pIB);
		void IASetPrimitiveTopology(PrimitiveTopology topology) { m_PriTopology = topology; }

		void VSSetShader(VertexShader* pVS) { m_pVS = pVS; }
		void VSSetTransMats(const Matrix4x4* matWorld, const Matrix4x4* matView, const Matrix4x4* matProj);
		void PSSetShader(PixelShader* pPS) { m_pPS = pPS; }
		void PSSetShaderResource(ShaderResourceView* const* pSRV) { m_pSRV = *pSRV; }

		void RSSetViewport(const Viewport* pVP);

		void OMSetRenderTarget(RenderTargetView* pRenderTarget, DepthStencilView* pDepthStencil);
		void ClearRenderTarget(RenderTargetView* pRenderTarget, const Vector4& clearColor);
		void ClearDepthStencil(DepthStencilView* pDepthStencil, float fDepth);
		void Draw();
		void DrawIndex(uint32_t nIndexNum);

#if DEBUG_RASTERIZATION
		bool CheckDrawPixelTiwce();
#endif

	private:
		void DrawLineWithDDA(float fPos1X, float fPos1Y, float fPos2X, float fPos2Y, const Vector4& lineColor);
		void DrawTriangleWithLine(const VSOutputVertex& v0, const VSOutputVertex& v1, const VSOutputVertex& v2);
		void DrawTriangleWithFlat(const VSOutputVertex& v0, const VSOutputVertex& v1, const VSOutputVertex& v2);

		void SortTriangleVertsByYGrow(VSOutputVertex& v0, VSOutputVertex& v1, VSOutputVertex& v2);
		void SortScanlineVertsByXGrow(VSOutputVertex& v0, VSOutputVertex& v1);

		//平定三角形和平底三角形
		void DrawTopTriangle(VSOutputVertex& v0, VSOutputVertex& v1, VSOutputVertex& v2);
		void DrawBottomTriangle(VSOutputVertex& v0, VSOutputVertex& v1, VSOutputVertex& v2);

		void SliceTriangleToUpAndBottom(const VSOutputVertex& v0, const VSOutputVertex& v1, const VSOutputVertex& v2, VSOutputVertex& vNew);

		void LerpVertexParamsInScreen(const VSOutputVertex& vStart, const VSOutputVertex& vEnd, VSOutputVertex& vNew, float fLerpFactor);

		void LerpVertexParamsInClip(const VSOutputVertex& vStart, const VSOutputVertex& vEnd, VSOutputVertex& vNew, float fLerpFactor);

		void ClipTrianglesInClipSpace();

		void ClipTriangleWithPlaneX(int nSign); //fSign为+1或者-1
		void ClipTriangleWithPlaneY(int nSign); //fSign为+1或者-1
		void ClipTriangleWithPlaneZeroZ();
		void ClipTriangleWithPlanePositiveZ();

		void ClipTwoVertsInTriangle(const VSOutputVertex& vertIn, VSOutputVertex& vertOut1, VSOutputVertex& vertOut2, float fLerpFactor1, float fLerpFactor2);
		void ClipOneVertInTriangle(VSOutputVertex& vertOut, const VSOutputVertex& vertIn1, const VSOutputVertex& vertIn2, float fLerpFactor1, float fLerpFactor2, std::vector<VSOutputVertex>& vTempVerts);
		float GetClipLerpFactorX(const VSOutputVertex& vert0, const VSOutputVertex& vert1, int fSign);
		float GetClipLerpFactorY(const VSOutputVertex& vert0, const VSOutputVertex& vert1, int fSign);
		float GetClipLerpFactorZeroZ(const VSOutputVertex& vert0, const VSOutputVertex& vert1);
		float GetClipLerpFactorPositiveZ(const VSOutputVertex& vert0, const VSOutputVertex& vert1);

		void ShapeAssemble(uint32_t nIndexNum);

		void Rasterization();

	private:
		uint32_t*					m_pFrameBuffer;
		float*						m_pDepthBuffer;
#if DEBUG_RASTERIZATION
		uint32_t*					m_pDebugBuffer;  //检查是否有重复绘制的像素
#endif
		uint32_t					m_nWidth;
		uint32_t					m_nHeight;

		VertexBuffer*				m_pVB;
		IndexBuffer*				m_pIB;

		VertexShader*				m_pVS;
		PixelShader*				m_pPS;

		ShaderResourceView*			m_pSRV;

		const Matrix4x4*			m_pWorldMat;
		const Matrix4x4*			m_pViewMat;
		const Matrix4x4*			m_pProjMat;

		VSOutputVertex*				m_pVSOutputs;
		std::vector<VSOutputVertex> m_vAssembledVerts;
		std::vector<VSOutputVertex>	m_vClipOutputVerts;
		std::vector<VSOutputVertex> m_vClippingVerts;

		Matrix4x4*					m_pViewportMat;

		PrimitiveTopology			m_PriTopology;
	};
}