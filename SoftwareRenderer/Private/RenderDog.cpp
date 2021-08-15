#include "RenderDog.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"
#include "Buffer.h"
#include "Shader.h"
#include "Vertex.h"
#include "Matrix.h"
#include "Viewport.h"
#include "Utility.h"

#include <vector>

namespace RenderDog
{
#pragma region Device
	class Device : public IDevice
	{
	public:
		Device() = default;
		~Device() = default;

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;

		virtual bool CreateTexture2D(const Texture2DDesc* pDesc, Texture2D** ppTexture) override;
		virtual bool CreateRenderTargetView(Texture2D* pTexture, const RenderTargetDesc* pDesc, RenderTargetView** ppRenderTarget) override;
		virtual bool CreateDepthStencilView(Texture2D* pTexture, DepthStencilView** ppDepthStencil) override;
		virtual bool CreateVertexBuffer(const VertexBufferDesc& vbDesc, VertexBuffer** ppVertexBuffer) override;
		virtual bool CreateIndexBuffer(const IndexBufferDesc& ibDesc, IndexBuffer** ppIndexBuffer) override;
		virtual bool CreateVertexShader(VertexShader** ppVertexShader) override;
		virtual bool CreatePixelShader(PixelShader** ppPixelShader) override;

		virtual void AddRef() override {}
		virtual void Release() override { delete this; }
	};


	bool Device::CreateTexture2D(const Texture2DDesc* pDesc, Texture2D** ppTexture)
	{
		Texture2D* pTex = new Texture2D(pDesc->width, pDesc->height, pDesc->format);
		if (!pTex)
		{
			return false;
		}

		*ppTexture = pTex;

		return true;
	}

	bool Device::CreateRenderTargetView(Texture2D* pTexture, const RenderTargetDesc* pDesc, RenderTargetView** ppRenderTarget)
	{
		if (!pDesc)
		{
			RenderTargetView* pRT = new RenderTargetView();
			if (!pRT)
			{
				return false;
			}

			*ppRenderTarget = pRT;

			pRT->GetView() = (uint32_t*)pTexture->GetDataUint32();
			pRT->SetWidth(pTexture->GetWidth());
			pRT->SetHeight(pTexture->GetHeight());
		}
		else
		{
			//ToDo: Create RenderTargetView by Desc
		}

		return true;
	}

	bool Device::CreateDepthStencilView(Texture2D* pTexture, DepthStencilView** ppDepthStencil)
	{
		DepthStencilView* pDS = new DepthStencilView();
		if (!pDS)
		{
			return false;
		}

		*ppDepthStencil = pDS;

		pDS->GetView() = (float*)pTexture->GetDataFloat32();
		pDS->SetWidth(pTexture->GetWidth());
		pDS->SetHeight(pTexture->GetHeight());

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
			delete* ppVertexShader;
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
			delete* ppPixelShader;
		}

		PixelShader* pPS = new PixelShader();
		if (!pPS)
		{
			return false;
		}

		*ppPixelShader = pPS;

		return true;
	}
#pragma endregion Device

#pragma region DeviceContext
	class DeviceContext : public IDeviceContext
	{
	public:
		DeviceContext(uint32_t width, uint32_t height);
		~DeviceContext();

		DeviceContext(const DeviceContext&) = delete;
		DeviceContext& operator=(const DeviceContext&) = delete;

		virtual void AddRef() override {}
		virtual void Release() override { delete this; }

		virtual void IASetVertexBuffer(VertexBuffer* pVB) override;
		virtual void IASetIndexBuffer(IndexBuffer* pIB) override;
		virtual void IASetPrimitiveTopology(PrimitiveTopology topology) override { m_PriTopology = topology; }

		virtual void VSSetShader(VertexShader* pVS) override { m_pVS = pVS; }
		virtual void VSSetTransMats(const Matrix4x4* matWorld, const Matrix4x4* matView, const Matrix4x4* matProj) override;
		virtual void PSSetShader(PixelShader* pPS) override { m_pPS = pPS; }
		virtual void PSSetShaderResource(ShaderResourceView* const* pSRV) override { m_pSRV = *pSRV; }
		virtual void PSSetMainLight(DirectionalLight* pLight) override { m_pMainLight = pLight; }

		virtual void RSSetViewport(const Viewport* pVP) override;

		virtual void OMSetRenderTarget(RenderTargetView* pRenderTarget, DepthStencilView* pDepthStencil) override;
		virtual void ClearRenderTarget(RenderTargetView* pRenderTarget, const Vector4& clearColor) override;
		virtual void ClearDepthStencil(DepthStencilView* pDepthStencil, float fDepth) override;
		virtual void Draw() override;
		virtual void DrawIndex(uint32_t nIndexNum) override;

#if DEBUG_RASTERIZATION
		virtual bool CheckDrawPixelTwice() override;
#endif
		virtual void DrawLineWithDDA(float fPos1X, float fPos1Y, float fPos2X, float fPos2Y, const Vector4& lineColor) override;

	private:
		void DrawTriangleWithLine(const VSOutputVertex& v0, const VSOutputVertex& v1, const VSOutputVertex& v2);
		void DrawTriangleWithFlat(const VSOutputVertex& v0, const VSOutputVertex& v1, const VSOutputVertex& v2);

		void SortTriangleVertsByYGrow(VSOutputVertex& v0, VSOutputVertex& v1, VSOutputVertex& v2);
		void SortScanlineVertsByXGrow(VSOutputVertex& v0, VSOutputVertex& v1);

		//平顶三角形和平底三角形
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

		void BackFaceCulling();

		void Rasterization();

	private:
		uint32_t* m_pFrameBuffer;
		float* m_pDepthBuffer;
#if DEBUG_RASTERIZATION
		uint32_t* m_pDebugBuffer;  //检查是否有重复绘制的像素
#endif
		uint32_t					m_nWidth;
		uint32_t					m_nHeight;

		VertexBuffer* m_pVB;
		IndexBuffer* m_pIB;

		VertexShader* m_pVS;
		PixelShader* m_pPS;

		ShaderResourceView* m_pSRV;

		const Matrix4x4* m_pWorldMat;
		const Matrix4x4* m_pViewMat;
		const Matrix4x4* m_pProjMat;

		VSOutputVertex* m_pVSOutputs;
		std::vector<VSOutputVertex> m_vAssembledVerts;
		std::vector<VSOutputVertex> m_vBackFaceCulledVerts;
		std::vector<VSOutputVertex>	m_vClipOutputVerts;
		std::vector<VSOutputVertex> m_vClippingVerts;

		Matrix4x4* m_pViewportMat;

		PrimitiveTopology			m_PriTopology;

		DirectionalLight* m_pMainLight;
	};

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
		m_PriTopology(PrimitiveTopology::TRIANGLE_LIST),
		m_pMainLight(nullptr)
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

		BackFaceCulling();

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
	bool DeviceContext::CheckDrawPixelTwice()
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
					Vector4 color = m_pPS->PSMain(vCurr, m_pSRV, m_pMainLight);
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
					Vector4 color = m_pPS->PSMain(vCurr, m_pSRV, m_pMainLight);
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
		if (m_vClipOutputVerts.capacity() < m_vBackFaceCulledVerts.capacity())
		{
			m_vClipOutputVerts.reserve(m_vBackFaceCulledVerts.capacity());
		}

		for (uint32_t i = 0; i < m_vBackFaceCulledVerts.size(); i += 3)
		{
			m_vClippingVerts.clear();

			const VSOutputVertex& vert0 = m_vBackFaceCulledVerts[i];
			const VSOutputVertex& vert1 = m_vBackFaceCulledVerts[i + 1];
			const VSOutputVertex& vert2 = m_vBackFaceCulledVerts[i + 2];

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

			nSign* vert0.SVPosition.x > vert0.SVPosition.w ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;
			nSign* vert1.SVPosition.x > vert1.SVPosition.w ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;
			nSign* vert2.SVPosition.x > vert2.SVPosition.w ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;

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

			nSign* vert0.SVPosition.y > vert0.SVPosition.w ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;
			nSign* vert1.SVPosition.y > vert1.SVPosition.w ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;
			nSign* vert2.SVPosition.y > vert2.SVPosition.w ? ++nOutOfClipPlaneNum : nOutOfClipPlaneNum;

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

	void DeviceContext::BackFaceCulling()
	{
		m_vBackFaceCulledVerts.clear();
		if (m_vBackFaceCulledVerts.capacity() < m_vAssembledVerts.size())
		{
			m_vBackFaceCulledVerts.reserve(m_vAssembledVerts.size());
		}

		for (uint32_t i = 0; i < m_vAssembledVerts.size(); i += 3)
		{
			const Vector4& Pos1 = m_vAssembledVerts[i].SVPosition;
			const Vector4& Pos2 = m_vAssembledVerts[i + 1].SVPosition;
			const Vector4& Pos3 = m_vAssembledVerts[i + 2].SVPosition;

			//三角形三个顶点若逆时针环绕，则行列式的值为正数，顺时针为负数，等于0则退化为线段
			if (GetArea2(Vector3(Pos1.x, Pos1.y, Pos1.z), Vector3(Pos2.x, Pos2.y, Pos2.z), Vector3(Pos3.x, Pos3.y, Pos3.z)) < 0.0f)
			{
				m_vBackFaceCulledVerts.push_back(m_vAssembledVerts[i]);
				m_vBackFaceCulledVerts.push_back(m_vAssembledVerts[i + 1]);
				m_vBackFaceCulledVerts.push_back(m_vAssembledVerts[i + 2]);
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

#pragma endregion DeviceContext

#pragma region SwapChain
	class SwapChain : public ISwapChain
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

		~SwapChain() = default;

		SwapChain(const SwapChainDesc* pDesc);
		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;

		virtual bool GetBuffer(Texture2D** ppSurface) override;
		virtual void AddRef() override {}
		virtual void Release() override;
		virtual void Present() override;

	private:
		uint32_t*		m_pBackBuffer;
		uint32_t		m_nWidth;
		uint32_t		m_nHeight;

		HWND			m_hWnd;
		HDC				m_hWndDC;
		HBITMAP			m_hBitMap;
		HBITMAP			m_hOldBitMap;
	};

	SwapChain::SwapChain(const SwapChainDesc* pDesc) :
		m_hWnd(pDesc->OutputWindow),
		m_nWidth(pDesc->Width),
		m_nHeight(pDesc->Height)
	{
		HDC hDC = GetDC(m_hWnd);
		m_hWndDC = CreateCompatibleDC(hDC);
		ReleaseDC(m_hWnd, hDC);

		uint16_t bitCnt = 0;
		uint32_t imageSize = 0;
		switch (pDesc->Format)
		{
		case RD_FORMAT::R8G8B8A8_UNORM:
		{
			bitCnt = 32;
			imageSize = pDesc->Width * pDesc->Height * 4;

			break;
		}
		case RD_FORMAT::UNKNOWN:
		{
			bitCnt = 0;
			imageSize = 0;

			break;
		}
		default:
		{
			break;
		}
		}

		void* pTempBitMapBuffer = nullptr;
		BITMAPINFO BitMapInfo =
		{
			{ sizeof(BITMAPINFOHEADER), (int)pDesc->Width, -(int)pDesc->Height, 1, bitCnt, BI_RGB, imageSize, 0, 0, 0, 0 }
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

		m_pBackBuffer = nullptr;

		delete this;
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

		pTex->GetDataUint32() = m_pBackBuffer;
		pTex->SetWidth(m_nWidth);
		pTex->SetHeight(m_nHeight);

		return true;
	}
#pragma endregion SwapChain


	bool CreateDeviceAndSwapChain(IDevice** pDevice, IDeviceContext** pDeviceContext, ISwapChain** pSwapChain, const SwapChainDesc* pSwapChainDesc)
	{
		*pDevice = new Device;
		if (!pDevice)
		{
			return false;
		}
		(*pDevice)->AddRef();

		*pDeviceContext = new DeviceContext(pSwapChainDesc->Width, pSwapChainDesc->Height);
		if (!pDeviceContext)
		{
			return false;
		}
		(*pDeviceContext)->AddRef();

		*pSwapChain = new SwapChain(pSwapChainDesc);
		if (!pSwapChain)
		{
			return false;
		}
		(*pSwapChain)->AddRef();

		return true;
	}
}