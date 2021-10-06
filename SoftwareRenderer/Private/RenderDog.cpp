#include "RenderDog.h"
#include "Vertex.h"
#include "Matrix.h"
#include "Utility.h"

#include <vector>

namespace RenderDog
{
	struct ShaderResourceTexture
	{
		Vector4* pColor;

		uint32_t width;
		uint32_t height;

		ShaderResourceTexture() :
			pColor(nullptr),
			width(0),
			height(0)
		{}
	};

#pragma region Shader
	struct VSOutputVertex
	{
		VSOutputVertex() = default;
		VSOutputVertex(const VSOutputVertex& v) = default;
		VSOutputVertex& operator=(const VSOutputVertex& v) = default;

		Vector4 SVPosition;
		Vector4 Color;
		Vector3 Normal;
		Vector4 Tangent;
		Vector2 Texcoord;
	};

	class VertexShader : public IVertexShader
	{
	public:
		VertexShader() = default;
		~VertexShader() = default;

		virtual void AddRef() override {}
		virtual void Release() override { delete this; }

		VSOutputVertex VSMain(const Vertex& inVertex, const Matrix4x4& matWorld, const Matrix4x4& matView, const Matrix4x4& matProj) const;
	};

	class PixelShader : public IPixelShader
	{
	private:
		struct MainLight
		{
			Vector3	direction;	//从光源发射光线的方向
			Vector3	color;
			float	luminance;

			MainLight() :
				direction(),
				color(),
				luminance(0)
			{}
		};
	public:
		PixelShader() :
			m_pMainLight(nullptr)
		{
			m_pMainLight = new MainLight;
		}
		~PixelShader()
		{}

		virtual void AddRef() override {}
		virtual void Release() override;

		void SetMainLight(const Vector3& direction, const Vector3& color, float luma);

		Vector4 PSMain(const VSOutputVertex& VSOutput, const ShaderResourceTexture* pSRTexture) const;

	private:
		Vector4 Sample(const ShaderResourceTexture* pSRTexture, const Vector2& vUV) const;

		Vector3 CalcPhongLighing(const MainLight* light, const Vector3& normal, const Vector3& faceColor) const;

	private:
		MainLight* m_pMainLight;
	};

	void PixelShader::Release()
	{ 
		if (m_pMainLight)
		{
			delete m_pMainLight;
			m_pMainLight = nullptr;
		}

		delete this; 
	}

	void PixelShader::SetMainLight(const Vector3& direction, const Vector3& color, float luma)
	{
		m_pMainLight->direction = direction;
		m_pMainLight->color = color;
		m_pMainLight->luminance = luma;
	}
#pragma endregion Shader

#pragma region Texture2D

	class Texture2D : public ITexture2D
	{
	public:
		Texture2D();
		~Texture2D();

		bool Init(const Texture2DDesc* pDesc, const SubResourceData* pInitData);
		void SetFormat(RD_FORMAT format) { m_Desc.format = format; }

		virtual void AddRef() override { ++m_RefCnt; }
		virtual void Release() override;

		virtual void GetType(RD_RESOURCE_DIMENSION* pResDimension) override { *pResDimension = RD_RESOURCE_DIMENSION::TEXTURE2D; }
		virtual void GetDesc(Texture2DDesc* pDesc) override { *pDesc = m_Desc; }

		void*& GetData() { return m_pData; }
		const void* GetData() const { return m_pData; }

		void SetWidth(uint32_t width) { m_Desc.width = width; }
		void SetHeight(uint32_t Height) { m_Desc.height = Height; }

		uint32_t GetWidth() const { return m_Desc.width; }
		uint32_t GetHeight() const { return m_Desc.height; }

	private:
		int						m_RefCnt;
		void*					m_pData;

		Texture2DDesc			m_Desc;
	};

	Texture2D::Texture2D() :
		m_RefCnt(0),
		m_pData(nullptr),
		m_Desc()
	{}

	bool Texture2D::Init(const Texture2DDesc* pDesc, const SubResourceData* pInitData)
	{
		m_Desc = *pDesc;
		uint32_t dataNum = pDesc->width * pDesc->height;

		if (pDesc->format == RD_FORMAT::R8G8B8A8_UNORM)
		{
			m_pData = new uint32_t[dataNum];
		}
		else if (pDesc->format == RD_FORMAT::R32_FLOAT)
		{
			m_pData = new float[dataNum];
		}
		else if (pDesc->format == RD_FORMAT::R32G32B32A32_FLOAT)
		{
			m_pData = new Vector4[dataNum];
		}
		else
		{
			m_pData = nullptr;
		}

		if (pInitData && m_pData)
		{
			memcpy(m_pData, pInitData->pSysMem, pInitData->sysMemPitch);
		}

		AddRef();

		return true;
	}

	Texture2D::~Texture2D()
	{
		if (m_pData)
		{
			delete[] m_pData;
			m_pData = nullptr;
		}
	}

	void Texture2D::Release()
	{
		--m_RefCnt;
		if (m_RefCnt == 0)
		{
			delete this;
		}
	}

#pragma endregion Texture2D


#pragma region View
	class RenderTargetView : public IRenderTargetView
	{
	public:
		RenderTargetView() :
			m_RefCnt(0),
			m_pViewResource(nullptr),
			m_Desc()
		{}
		~RenderTargetView()
		{}

		bool Init(IResource* pResource, const RenderTargetViewDesc* pDesc);

		virtual void AddRef() override { ++m_RefCnt; }
		virtual void Release() override;

		virtual void GetResource(IResource** ppResource) override { *ppResource = m_pViewResource; }

		virtual void GetDesc(RenderTargetViewDesc* pDesc) override { *pDesc = m_Desc; }

	private:
		int						m_RefCnt;

		IResource*				m_pViewResource;
		RenderTargetViewDesc	m_Desc;
	};

	bool RenderTargetView::Init(IResource* pResource, const RenderTargetViewDesc* pDesc)
	{
		m_pViewResource = pResource;
		m_pViewResource->AddRef();

		if (pDesc)
		{
			m_Desc = *pDesc;
		}

		AddRef();

		return true;
	}

	void RenderTargetView::Release()
	{
		m_pViewResource->Release();

		--m_RefCnt;
		if (m_RefCnt == 0)
		{
			delete this;
		}
	}

	class DepthStencilView : public IDepthStencilView
	{
	public:
		DepthStencilView() :
			m_RefCnt(0),
			m_pViewResource(nullptr),
			m_Desc()
		{}
		~DepthStencilView()
		{}

		bool Init(IResource* pResource, const DepthStencilViewDesc* pDesc);

		virtual void AddRef() override { ++m_RefCnt; }
		virtual void Release() override;

		virtual void GetResource(IResource** ppResource) override { *ppResource = m_pViewResource; }

		virtual void GetDesc(DepthStencilViewDesc* pDesc) override { *pDesc = m_Desc; }

	private:
		int						m_RefCnt;

		IResource*				m_pViewResource;
		DepthStencilViewDesc	m_Desc;
	};

	bool DepthStencilView::Init(IResource* pResource, const DepthStencilViewDesc* pDesc)
	{
		m_pViewResource = pResource;
		m_pViewResource->AddRef();

		if (pDesc)
		{
			m_Desc = *pDesc;
		}

		AddRef();

		return true;
	}

	void DepthStencilView::Release()
	{
		m_pViewResource->Release();

		--m_RefCnt;
		if (m_RefCnt == 0)
		{
			delete this;
		}
	}

	class ShaderResourceView : public IShaderResourceView
	{
	public:
		ShaderResourceView() :
			m_RefCnt(0),
			m_pViewResource(nullptr),
			m_Desc()
		{}

		~ShaderResourceView()
		{}

		bool Init(IResource* pResource, const ShaderResourceViewDesc* pDesc);

		virtual void AddRef() override { ++m_RefCnt; }
		virtual void Release() override;

		virtual void GetResource(IResource** ppResource) override { *ppResource = m_pViewResource; }

		virtual void GetDesc(ShaderResourceViewDesc* pDesc) override { *pDesc = m_Desc; }

	private:
		int						m_RefCnt;

		IResource*				m_pViewResource;
		ShaderResourceViewDesc	m_Desc;
	};

	bool ShaderResourceView::Init(IResource* pResource, const ShaderResourceViewDesc* pDesc)
	{
		m_pViewResource = pResource;

		m_pViewResource->AddRef();

		if (pDesc)
		{
			m_Desc = *pDesc;
		}

		AddRef();

		return true;
	}

	void ShaderResourceView::Release()
	{
		m_pViewResource->Release();

		--m_RefCnt;
		if (m_RefCnt == 0)
		{
			delete this;
		}
	}

#pragma endregion View

#pragma region Buffer
	class VertexBuffer : public IBuffer
	{
	public:
		VertexBuffer() :
			m_RefCnt(0),
			m_Desc(),
			m_pData(nullptr),
			m_nVertsNum(0)
		{}

		~VertexBuffer()
		{}

		bool Init(const BufferDesc* pDesc, const SubResourceData* pInitData);

		virtual void AddRef() override { ++m_RefCnt; }
		virtual void Release() override;

		virtual void GetType(RD_RESOURCE_DIMENSION* pResDimension) override { *pResDimension = RD_RESOURCE_DIMENSION::BUFFER; }

		virtual void GetDesc(BufferDesc* pDesc) override { *pDesc = m_Desc; }

		const Vertex* GetData() const { return m_pData; }
		const uint32_t GetNum() const { return m_nVertsNum; }

	private:
		int			m_RefCnt;
		BufferDesc	m_Desc;

		Vertex*		m_pData;
		uint32_t	m_nVertsNum;
	};

	bool VertexBuffer::Init(const BufferDesc* pDesc, const SubResourceData* pInitData)
	{
		if (!pDesc)
		{
			return false;
		}

		m_Desc = *pDesc;

		m_nVertsNum = pDesc->byteWidth / sizeof(Vertex);
		m_pData = new Vertex[m_nVertsNum];
		if (!m_pData)
		{
			return false;
		}

		if (pInitData)
		{
			memcpy(m_pData, pInitData->pSysMem, pInitData->sysMemPitch);
		}

		AddRef();

		return true;
	}

	void VertexBuffer::Release()
	{
		--m_RefCnt;
		if (m_RefCnt == 0 && m_pData)
		{
			delete[] m_pData;
			m_pData = nullptr;

			delete this;
		}
	}

	class IndexBuffer : public IBuffer
	{
	public:
		IndexBuffer() :
			m_RefCnt(0),
			m_Desc(),
			m_pData(nullptr)
		{}

		~IndexBuffer()
		{}

		bool Init(const BufferDesc* pDesc, const SubResourceData* pInitData);

		virtual void AddRef() override { ++m_RefCnt; }
		virtual void Release() override;

		virtual void GetType(RD_RESOURCE_DIMENSION* pResDimension) override { *pResDimension = RD_RESOURCE_DIMENSION::BUFFER; }

		virtual void GetDesc(BufferDesc* pDesc) override { *pDesc = m_Desc; }

		const uint32_t* GetData() const { return m_pData; }

	private:
		int			m_RefCnt;
		BufferDesc	m_Desc;

		uint32_t*	m_pData;
	};

	bool IndexBuffer::Init(const BufferDesc* pDesc, const SubResourceData* pInitData)
	{
		if (!pDesc)
		{
			return false;
		}

		m_Desc = *pDesc;

		uint32_t indexNum = pDesc->byteWidth / sizeof(uint32_t);
		m_pData = new uint32_t[indexNum];
		if (!m_pData)
		{
			return false;
		}

		if (pInitData)
		{
			memcpy(m_pData, pInitData->pSysMem, pInitData->sysMemPitch);
		}

		AddRef();

		return true;
	}

	void IndexBuffer::Release()
	{
		--m_RefCnt;
		if (m_RefCnt == 0 && m_pData)
		{
			delete[] m_pData;
			m_pData = nullptr;

			delete this;
		}
	}

	class ConstantBuffer : public IBuffer
	{
	public:
		ConstantBuffer() :
			m_RefCnt(0),
			m_Desc(),
			m_pData(nullptr)
		{}
		~ConstantBuffer()
		{}

		bool Init(const BufferDesc* pDesc, const SubResourceData* pInitData);

		virtual void AddRef() override { ++m_RefCnt; }
		virtual void Release() override;

		virtual void GetType(RD_RESOURCE_DIMENSION* pResDimension) override { *pResDimension = RD_RESOURCE_DIMENSION::BUFFER; }

		virtual void GetDesc(BufferDesc* pDesc) override { *pDesc = m_Desc; }

		const void* GetData() const { return m_pData; }
		void* GetData() { return m_pData; }

	private:
		int							m_RefCnt;

		BufferDesc					m_Desc;

		void*						m_pData;
	};

	bool ConstantBuffer::Init(const BufferDesc* pDesc, const SubResourceData* pInitData)
	{
		if (!pDesc)
		{
			return false;
		}

		m_pData = malloc(pDesc->byteWidth);
		if (!m_pData)
		{
			return false;
		}

		m_Desc = *pDesc;

		if (pInitData)
		{
			memcpy(m_pData, pInitData->pSysMem, pInitData->sysMemPitch);
		}

		AddRef();

		return true;
	}

	void ConstantBuffer::Release()
	{
		--m_RefCnt;
		if (m_RefCnt == 0)
		{
			free(m_pData);

			delete this;
		}
	}

#pragma endregion Buffer

#pragma region Device
	class Device : public IDevice
	{
	public:
		Device() = default;
		~Device() = default;

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;

		virtual bool CreateTexture2D(const Texture2DDesc* pDesc, const SubResourceData* pInitData, ITexture2D** ppTexture) override;
		virtual bool CreateRenderTargetView(IResource* pResource, const RenderTargetViewDesc* pDesc, IRenderTargetView** ppRenderTargetView) override;
		virtual bool CreateDepthStencilView(IResource* pResource, const DepthStencilViewDesc* pDesc, IDepthStencilView** ppDepthStencilView) override;
		virtual bool CreateShaderResourceView(IResource* pResource, const ShaderResourceViewDesc* pDesc, IShaderResourceView** ppShaderResourceView) override;
		virtual bool CreateBuffer(const BufferDesc* pDesc, const SubResourceData* pInitData, IBuffer** ppBuffer) override;
		virtual bool CreateVertexShader(IVertexShader** ppVertexShader) override;
		virtual bool CreatePixelShader(IPixelShader** ppPixelShader) override;

		virtual void AddRef() override {}
		virtual void Release() override { delete this; }

	private:
		bool CreateVertexBuffer(const BufferDesc* pDesc, const SubResourceData* pInitData, IBuffer** ppBuffer);
		bool CreateIndexBuffer(const BufferDesc* pDesc, const SubResourceData* pInitData, IBuffer** ppBuffer);
		bool CreateConstantBuffer(const BufferDesc* pDesc, const SubResourceData* pInitData, IBuffer** ppBuffer);
	};


	bool Device::CreateTexture2D(const Texture2DDesc* pDesc, const SubResourceData* pInitData, ITexture2D** ppTexture)
	{
		Texture2D* pTex = new Texture2D;
		if (!pTex)
		{
			return false;
		}

		if (!pTex->Init(pDesc, pInitData))
		{
			return false;
		}

		*ppTexture = pTex;

		return true;
	}

	bool Device::CreateRenderTargetView(IResource* pResource, const RenderTargetViewDesc* pDesc, IRenderTargetView** ppRenderTargetView)
	{
		RenderTargetView* pRTV = new RenderTargetView();
		if (!pRTV)
		{
			return false;
		}

		RD_RESOURCE_DIMENSION resDimension;
		pResource->GetType(&resDimension);
		if (resDimension == RD_RESOURCE_DIMENSION::TEXTURE2D)
		{
			RenderTargetViewDesc rtvDesc;
			rtvDesc.viewDimension = RD_RTV_DIMENSION::TEXTURE2D;
			pDesc = &rtvDesc;
		}

		if (!pRTV->Init(pResource, pDesc))
		{
			return false;
		}

		*ppRenderTargetView = pRTV;

		return true;
	}

	bool Device::CreateDepthStencilView(IResource* pResource, const DepthStencilViewDesc* pDesc, IDepthStencilView** ppDepthStencilView)
	{
		DepthStencilView* pDSV = new DepthStencilView();
		if (!pDSV)
		{
			return false;
		}

		if (!pDSV->Init(pResource, pDesc))
		{
			return false;
		}

		*ppDepthStencilView = pDSV;

		return true;
	}

	bool Device::CreateShaderResourceView(IResource* pResource, const ShaderResourceViewDesc* pDesc, IShaderResourceView** ppShaderResourceView)
	{
		ShaderResourceView* pSRV = new ShaderResourceView();
		if (!pSRV)
		{
			return false;
		}

		if (!pSRV->Init(pResource, pDesc))
		{
			return false;
		}

		*ppShaderResourceView = pSRV;

		return true;
	}

	bool Device::CreateBuffer(const BufferDesc* pDesc, const SubResourceData* pInitData, IBuffer** ppBuffer)
	{
		if (!pDesc)
		{
			return false;
		}

		switch (pDesc->bindFlag)
		{
		case RD_BIND_FLAG::BIND_VERTEX_BUFFER:
		{
			return CreateVertexBuffer(pDesc, pInitData, ppBuffer);
		}
		case RD_BIND_FLAG::BIND_INDEX_BUFFER:
		{
			return CreateIndexBuffer(pDesc, pInitData, ppBuffer);
		}
		case RD_BIND_FLAG::BIND_CONSTANT_BUFFER:
		{
			return CreateConstantBuffer(pDesc, pInitData, ppBuffer);
		}
		default:
			break;
		}
		
		return false;
	}

	bool Device::CreateVertexShader(IVertexShader** ppVertexShader)
	{
		VertexShader* pVS = new VertexShader();
		if (!pVS)
		{
			return false;
		}

		*ppVertexShader = pVS;

		return true;
	}

	bool Device::CreatePixelShader(IPixelShader** ppPixelShader)
	{
		PixelShader* pPS = new PixelShader();
		if (!pPS)
		{
			return false;
		}

		*ppPixelShader = pPS;

		return true;
	}

	bool Device::CreateVertexBuffer(const BufferDesc* pDesc, const SubResourceData* pInitData, IBuffer** ppBuffer)
	{
		VertexBuffer* pVB = new VertexBuffer();
		if (!pVB)
		{
			return false;
		}

		if (!pVB->Init(pDesc, pInitData))
		{
			return false;
		}

		*ppBuffer = pVB;

		return true;
	}

	bool Device::CreateIndexBuffer(const BufferDesc* pDesc, const SubResourceData* pInitData, IBuffer** ppBuffer)
	{
		IndexBuffer* pIB = new IndexBuffer();
		if (!pIB)
		{
			return false;
		}

		if (!pIB->Init(pDesc, pInitData))
		{
			return false;
		}

		*ppBuffer = pIB;

		return true;
	}

	bool Device::CreateConstantBuffer(const BufferDesc* pDesc, const SubResourceData* pInitData, IBuffer** ppBuffer)
	{
		ConstantBuffer* pCB = new ConstantBuffer();
		if (!pCB)
		{
			return false;
		}

		if (!pCB->Init(pDesc, pInitData))
		{
			return false;
		}

		*ppBuffer = pCB;

		return true;
	}
#pragma endregion Device

#pragma region DeviceContext
	class DeviceContext : public IDeviceContext
	{
	public:
		DeviceContext();
		~DeviceContext();

		DeviceContext(const DeviceContext&) = delete;
		DeviceContext& operator=(const DeviceContext&) = delete;

		bool Init(uint32_t width, uint32_t height);

		virtual void AddRef() override {}
		virtual void Release() override { delete this; }

		virtual void IASetVertexBuffer(IBuffer* pVB) override;
		virtual void IASetIndexBuffer(IBuffer* pIB) override;
		virtual void IASetPrimitiveTopology(RD_PRIMITIVE_TOPOLOGY topology) override { m_PriTopology = topology; }

		virtual	void UpdateSubresource(IResource* pDstResource, const void* pSrcData, uint32_t srcRowPitch, uint32_t srcDepthPitch) override;

		virtual void VSSetShader(IVertexShader* pVS) override { m_pVS = dynamic_cast<VertexShader*>(pVS); }
		virtual void VSSetConstantBuffer(uint32_t startSlot, IBuffer* const* ppConstantBuffer) override;
		virtual void PSSetConstantBuffer(uint32_t startSlot, IBuffer* const* ppConstantBuffer) override;
		virtual void PSSetShader(IPixelShader* pPS) override { m_pPS = dynamic_cast<PixelShader*>(pPS); }
		virtual void PSSetShaderResource(IShaderResourceView* const* ppShaderResourceView) override;

		virtual void RSSetViewport(const Viewport* pVP) override;

		virtual void OMSetRenderTarget(IRenderTargetView* pRenderTargetView, IDepthStencilView* pDepthStencilVew) override;
		virtual void ClearRenderTargetView(IRenderTargetView* pRenderTargetView, const float* clearColor) override;
		virtual void ClearDepthStencilView(IDepthStencilView* pDepthStencilView, float fDepth) override;
		virtual void Draw() override;
		virtual void DrawIndex(uint32_t nIndexNum) override;

#if DEBUG_RASTERIZATION
		virtual bool CheckDrawPixelTwice() override;
#endif
		virtual void DrawLineWithDDA(float fPos1X, float fPos1Y, float fPos2X, float fPos2Y, const float* lineColor) override;

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
		uint32_t*					m_pFrameBuffer;
		float*						m_pDepthBuffer;
#if DEBUG_RASTERIZATION
		uint32_t*					m_pDebugBuffer;  //检查是否有重复绘制的像素
#endif
		uint32_t					m_BackBufferWidth;
		uint32_t					m_BackBufferHeight;

		VertexBuffer*				m_pVB;
		IndexBuffer*				m_pIB;
		ConstantBuffer*				m_pCB[2];

		VertexShader*				m_pVS;
		PixelShader*				m_pPS;

		ShaderResourceTexture		m_SRTexture;

		std::vector<VSOutputVertex>	m_VSOutputs;
		std::vector<VSOutputVertex> m_AssembledVerts;
		std::vector<VSOutputVertex> m_BackFaceCulledVerts;
		std::vector<VSOutputVertex>	m_ClipOutputVerts;
		std::vector<VSOutputVertex> m_ClippingVerts;

		Matrix4x4					m_ViewportMatrix;

		RD_PRIMITIVE_TOPOLOGY		m_PriTopology;
	};

	DeviceContext::DeviceContext() :
		m_pFrameBuffer(nullptr),
		m_pDepthBuffer(nullptr),
#if DEBUG_RASTERIZATION
		m_pDebugBuffer(nullptr),
#endif
		m_BackBufferWidth(0),
		m_BackBufferHeight(0),
		m_pVB(nullptr),
		m_pIB(nullptr),
		m_pVS(nullptr),
		m_pPS(nullptr),
		m_SRTexture(),
		m_PriTopology(RD_PRIMITIVE_TOPOLOGY::TRIANGLE_LIST)
	{
		m_pCB[0] = nullptr;
		m_pCB[1] = nullptr;

		m_ViewportMatrix.Identity();
#if DEBUG_RASTERIZATION
		uint32_t nBufferSize = m_BackBufferWidth * m_BackBufferHeight;
		m_pDebugBuffer = new uint32_t[nBufferSize];

		for (uint32_t i = 0; i < nBufferSize; ++i)
		{
			m_pDebugBuffer[i] = 0;
		}
#endif
	}

	DeviceContext::~DeviceContext()
	{
		m_VSOutputs.clear();
		m_ClipOutputVerts.clear();
		m_AssembledVerts.clear();
		m_ClippingVerts.clear();

#if DEBUG_RASTERIZATION
		if (m_pDebugBuffer)
		{
			delete[] m_pDebugBuffer;
			m_pDebugBuffer = nullptr;
		}
#endif
	}

	bool DeviceContext::Init(uint32_t width, uint32_t height)
	{
		m_BackBufferWidth = width;
		m_BackBufferHeight = height;

		AddRef();

		return true;
	}

	void DeviceContext::IASetVertexBuffer(IBuffer* pVB)
	{
		if (m_pVB != pVB)
		{
			m_VSOutputs.clear();

			m_pVB = dynamic_cast<VertexBuffer*>(pVB);

			uint32_t nVertexNum = m_pVB->GetNum();
			m_VSOutputs.resize(nVertexNum);
		}
	}
	void DeviceContext::IASetIndexBuffer(IBuffer* pIB)
	{
		if (m_pIB != pIB)
		{
			m_pIB = dynamic_cast<IndexBuffer*>(pIB);
		}
	}

	void DeviceContext::UpdateSubresource(IResource* pDstResource, const void* pSrcData, uint32_t srcRowPitch, uint32_t srcDepthPitch)
	{
		RD_RESOURCE_DIMENSION resDimension;
		pDstResource->GetType(&resDimension);

		switch (resDimension)
		{
		case RenderDog::RD_RESOURCE_DIMENSION::UNKNOWN:
		{
			return;
			break;
		}
		case RenderDog::RD_RESOURCE_DIMENSION::BUFFER:
		{
			IBuffer* pBuffer = dynamic_cast<IBuffer*>(pDstResource);
			BufferDesc bufferDesc;
			pBuffer->GetDesc(&bufferDesc);
			if (bufferDesc.bindFlag == RD_BIND_FLAG::BIND_CONSTANT_BUFFER)
			{
				ConstantBuffer* pCB = dynamic_cast<ConstantBuffer*>(pBuffer);
				memcpy(pCB->GetData(), pSrcData, bufferDesc.byteWidth);
			}
			
			break;
		}
		case RenderDog::RD_RESOURCE_DIMENSION::TEXTURE2D:
		{
			break;
		}
		default:
			break;
		}
	}

	void DeviceContext::VSSetConstantBuffer(uint32_t startSlot, IBuffer* const* ppConstantBuffer)
	{
		m_pCB[startSlot] = dynamic_cast<ConstantBuffer*>(*ppConstantBuffer);
	}

	void DeviceContext::PSSetConstantBuffer(uint32_t startSlot, IBuffer* const* ppConstantBuffer)
	{
		m_pCB[startSlot] = dynamic_cast<ConstantBuffer*>(*ppConstantBuffer);

		float* pData = static_cast<float*>(m_pCB[startSlot]->GetData());
		Vector3 mainLightDir(pData[0], pData[1], pData[2]);
		Vector3 mainLightColor(pData[3], pData[4], pData[5]);
		float mainLightLuma = pData[6];

		m_pPS->SetMainLight(mainLightDir, mainLightColor, mainLightLuma);
	}

	void DeviceContext::PSSetShaderResource(IShaderResourceView* const* ppShaderResourceView)
	{ 
		IResource* pRes = nullptr;
		(*ppShaderResourceView)->GetResource(&pRes);

		ShaderResourceViewDesc srvDesc;
		(*ppShaderResourceView)->GetDesc(&srvDesc);

		switch (srvDesc.viewDimension)
		{
		case RD_SRV_DIMENSION::TEXTURE2D:
		{
			Texture2D* pTex2D = dynamic_cast<Texture2D*>(pRes);
			Texture2DDesc texDesc;
			pTex2D->GetDesc(&texDesc);

			m_SRTexture.pColor = static_cast<Vector4*>(pTex2D->GetData());
			m_SRTexture.width = texDesc.width;
			m_SRTexture.height = texDesc.height;
		}
		default:
			break;
		}
	}

	void DeviceContext::RSSetViewport(const Viewport* pViewport)
	{
		m_ViewportMatrix(0, 0) = (pViewport->width - 1) / 2.0f;
		m_ViewportMatrix(3, 0) = (pViewport->width - 1) / 2.0f + pViewport->topLeftX;
		m_ViewportMatrix(1, 1) = -(pViewport->height - 1) / 2.0f;
		m_ViewportMatrix(3, 1) = (pViewport->height - 1) / 2.0f + pViewport->topLeftY;
		m_ViewportMatrix(2, 2) = (pViewport->maxDepth - pViewport->minDepth);
		m_ViewportMatrix(3, 2) = pViewport->minDepth;
	}

	void DeviceContext::OMSetRenderTarget(IRenderTargetView* pRenderTargetView, IDepthStencilView* pDepthStencilView)
	{
		IResource* pTex = nullptr;
		pRenderTargetView->GetResource(&pTex);
		
		RenderTargetViewDesc rtvDesc;
		pRenderTargetView->GetDesc(&rtvDesc);

		switch (rtvDesc.viewDimension)
		{
		case RD_RTV_DIMENSION::UNKNOWN:
		{
			m_pFrameBuffer = nullptr;
			break;
		}
		case RD_RTV_DIMENSION::BUFFER:
		{
			break;
		}
		case RD_RTV_DIMENSION::TEXTURE2D:
		{
			Texture2D* pTex2D = dynamic_cast<Texture2D*>(pTex);

			//TODO: use format to determine m_pFrameBuffer's type;
			m_pFrameBuffer = (uint32_t*)pTex2D->GetData();
			Texture2DDesc desc;
			pTex2D->GetDesc(&desc);

			m_BackBufferWidth = desc.width;
			m_BackBufferHeight = desc.height;

			break;
		}
		default:
			break;
		}
		
		
		pDepthStencilView->GetResource(&pTex);
		
		DepthStencilViewDesc dsvDesc;
		pDepthStencilView->GetDesc(&dsvDesc);

		switch (dsvDesc.viewDimension)
		{
		case RD_DSV_DIMENSION::UNKNOWN:
		{
			m_pDepthBuffer = nullptr;
			break;
		}
		case RD_DSV_DIMENSION::TEXTURE2D:
		{
			Texture2D* pTex2D = dynamic_cast<Texture2D*>(pTex);

			//TODO: use format to determine m_pFrameBuffer's type;
			m_pDepthBuffer = (float*)pTex2D->GetData();

			break;
		}
		default:
			break;
		}
	}

	void DeviceContext::ClearRenderTargetView(IRenderTargetView* pRenderTargetView, const float* clearColor)
	{
		Vector4 colorVector(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
		Vector4 ARGB = ConvertRGBAColorToARGBColor(colorVector);
		uint32_t nClearColor = ConvertColorToUInt32(ARGB);

		IResource* pTex = nullptr;
		pRenderTargetView->GetResource(&pTex);

		RenderTargetViewDesc rtvDesc;
		pRenderTargetView->GetDesc(&rtvDesc);
		
		if(rtvDesc.viewDimension == RD_RTV_DIMENSION::TEXTURE2D)
		{
			Texture2D* pTex2D = dynamic_cast<Texture2D*>(pTex);

			Texture2DDesc texDesc;
			pTex2D->GetDesc(&texDesc);

			uint32_t rtWidth = texDesc.width;
			uint32_t rtHeight = texDesc.height;
			if (texDesc.format == RD_FORMAT::R8G8B8A8_UNORM)
			{
				uint32_t* pRT = static_cast<uint32_t*>(pTex2D->GetData());
				for (uint32_t row = 0; row < rtHeight; ++row)
				{
					for (uint32_t col = 0; col < rtWidth; ++col)
					{
						uint32_t nIndex = row * rtWidth + col;
						pRT[nIndex] = nClearColor;
					}
				}
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

	void DeviceContext::ClearDepthStencilView(IDepthStencilView* pDepthStencil, float fDepth)
	{
		IResource* pTex = nullptr;
		pDepthStencil->GetResource(&pTex);

		DepthStencilViewDesc dsvDesc;
		pDepthStencil->GetDesc(&dsvDesc);

		if (dsvDesc.viewDimension == RD_DSV_DIMENSION::TEXTURE2D)
		{
			Texture2D* pTex2D = dynamic_cast<Texture2D*>(pTex);
			Texture2DDesc texDesc;
			pTex2D->GetDesc(&texDesc);

			if (texDesc.format == RD_FORMAT::R32_FLOAT)
			{
				float* pDepth = static_cast<float*>(pTex2D->GetData());
				uint32_t nWidth = texDesc.width;
				uint32_t nHeight = texDesc.height;
				for (uint32_t row = 0; row < nHeight; ++row)
				{
					for (uint32_t col = 0; col < nWidth; ++col)
					{
						uint32_t nIndex = row * nWidth + col;
						pDepth[nIndex] = fDepth;
					}
				}
			}
		}
	}

	void DeviceContext::Draw()
	{
		float clearColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

		DrawLineWithDDA(100, 100, 100, 100, clearColor);
	}

	void DeviceContext::DrawIndex(uint32_t nIndexNum)
	{
		const Vertex* pVerts = m_pVB->GetData();
		//Local Space to Clip Space
		for (uint32_t i = 0; i < m_pVB->GetNum(); ++i)
		{
			const Vertex& vert = pVerts[i];

			Matrix4x4* pWorldMatrix = (Matrix4x4*)m_pCB[0]->GetData();
			Matrix4x4* pViewMatrix = (Matrix4x4*)m_pCB[0]->GetData() + 1;
			Matrix4x4* pProjMatrix = (Matrix4x4*)m_pCB[0]->GetData() + 2;
			m_VSOutputs[i] = m_pVS->VSMain(vert, *pWorldMatrix, *pViewMatrix, *pProjMatrix);
		}

		ShapeAssemble(nIndexNum);

		BackFaceCulling();

		ClipTrianglesInClipSpace();

		for (uint32_t i = 0; i < m_ClipOutputVerts.size(); ++i)
		{
			VSOutputVertex& vsOutput = m_ClipOutputVerts[i];
			Vector4 vScreenPos(vsOutput.SVPosition.x, vsOutput.SVPosition.y, vsOutput.SVPosition.z, 1.0f);
			vScreenPos = vScreenPos * m_ViewportMatrix;
			vsOutput.SVPosition.x = vScreenPos.x;
			vsOutput.SVPosition.y = vScreenPos.y;
			vsOutput.SVPosition.z = vScreenPos.z;
		}

		Rasterization();
	}

#if DEBUG_RASTERIZATION
	bool DeviceContext::CheckDrawPixelTwice()
	{
		for (uint32_t i = 0; i < m_BackBufferHeight; ++i)
		{
			for (uint32_t j = 0; j < m_BackBufferWidth; ++j)
			{
				uint32_t nDrawCnt = m_pDebugBuffer[i * m_BackBufferWidth + j];
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
		Vector4 lineColorVector(lineColor[0], lineColor[1], lineColor[2], lineColor[3]);
		uint32_t nClearColor = ConvertColorToUInt32(lineColorVector);

		float DeltaX = fPos2X - fPos1X;
		float DeltaY = fPos2Y - fPos1Y;

		if (std::abs(DeltaX - 0.0f) < 0.000001f)
		{
			float yStart = std::fmin(fPos1Y, fPos2Y);
			float yEnd = std::fmax(fPos1Y, fPos2Y);
			for (int yStep = (int)yStart; yStep <= (int)yEnd; ++yStep)
			{
				m_pFrameBuffer[(int)fPos1X + yStep * m_BackBufferWidth] = nClearColor;
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
				m_pFrameBuffer[xStep + yStep * m_BackBufferWidth] = nClearColor;
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
				m_pFrameBuffer[xStep + yStep * m_BackBufferWidth] = nClearColor;
			}
		}
	}

	void DeviceContext::DrawTriangleWithLine(const VSOutputVertex& v0, const VSOutputVertex& v1, const VSOutputVertex& v2)
	{
		float lineColor[4] = { v0.Color.x, v0.Color.y, v0.Color.z, 1.0f };
		DrawLineWithDDA(v0.SVPosition.x, v0.SVPosition.y, v1.SVPosition.x, v1.SVPosition.y, lineColor);
		DrawLineWithDDA(v1.SVPosition.x, v1.SVPosition.y, v2.SVPosition.x, v2.SVPosition.y, lineColor);
		DrawLineWithDDA(v2.SVPosition.x, v2.SVPosition.y, v0.SVPosition.x, v0.SVPosition.y, lineColor);
	}

	void DeviceContext::DrawTriangleWithFlat(const VSOutputVertex& v0, const VSOutputVertex& v1, const VSOutputVertex& v2)
	{
		if (FloatEqual(v0.SVPosition.y, v1.SVPosition.y, RD_FLT_EPSILON) && FloatEqual(v0.SVPosition.y, v2.SVPosition.y, RD_FLT_EPSILON) ||
			FloatEqual(v0.SVPosition.x, v1.SVPosition.x, RD_FLT_EPSILON) && FloatEqual(v0.SVPosition.x, v2.SVPosition.x, RD_FLT_EPSILON))
		{
			return;
		}

		VSOutputVertex vert0(v0);
		VSOutputVertex vert1(v1);
		VSOutputVertex vert2(v2);
		SortTriangleVertsByYGrow(vert0, vert1, vert2);

		if (FloatEqual(vert0.SVPosition.y, vert1.SVPosition.y, RD_FLT_EPSILON))
		{
			DrawTopTriangle(vert0, vert1, vert2);
		}
		else if (FloatEqual(vert1.SVPosition.y, vert2.SVPosition.y, RD_FLT_EPSILON))
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

				float fPixelDepth = m_pDepthBuffer[j + i * m_BackBufferWidth];
				if (vCurr.SVPosition.z <= fPixelDepth)
				{
					Vector4 color = m_pPS->PSMain(vCurr, &m_SRTexture);
					Vector4 ARGB = ConvertRGBAColorToARGBColor(color);
					m_pFrameBuffer[j + i * m_BackBufferWidth] = ConvertColorToUInt32(ARGB);

					m_pDepthBuffer[j + i * m_BackBufferWidth] = vCurr.SVPosition.z;
				}

#if DEBUG_RASTERIZATION
				m_pDebugBuffer[j + i * m_BackBufferWidth]++;
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

				float fPixelDepth = m_pDepthBuffer[j + i * m_BackBufferWidth];
				if (vCurr.SVPosition.z <= fPixelDepth)
				{
					Vector4 color = m_pPS->PSMain(vCurr, &m_SRTexture);
					Vector4 ARGB = ConvertRGBAColorToARGBColor(color);
					m_pFrameBuffer[j + i * m_BackBufferWidth] = ConvertColorToUInt32(ARGB);

					m_pDepthBuffer[j + i * m_BackBufferWidth] = vCurr.SVPosition.z;
				}

#if DEBUG_RASTERIZATION
				m_pDebugBuffer[j + i * m_BackBufferWidth]++;
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
		m_ClipOutputVerts.clear();
		if (m_ClipOutputVerts.capacity() < m_BackFaceCulledVerts.capacity())
		{
			m_ClipOutputVerts.reserve(m_BackFaceCulledVerts.capacity());
		}

		for (uint32_t i = 0; i < m_BackFaceCulledVerts.size(); i += 3)
		{
			m_ClippingVerts.clear();

			const VSOutputVertex& vert0 = m_BackFaceCulledVerts[i];
			const VSOutputVertex& vert1 = m_BackFaceCulledVerts[i + 1];
			const VSOutputVertex& vert2 = m_BackFaceCulledVerts[i + 2];

			m_ClippingVerts.push_back(vert0);
			m_ClippingVerts.push_back(vert1);
			m_ClippingVerts.push_back(vert2);

			ClipTriangleWithPlaneX(1);
			ClipTriangleWithPlaneX(-1);
			ClipTriangleWithPlaneY(1);
			ClipTriangleWithPlaneY(-1);
			ClipTriangleWithPlaneZeroZ();
			ClipTriangleWithPlanePositiveZ();

			for (uint32_t i = 0; i < m_ClippingVerts.size(); ++i)
			{
				m_ClipOutputVerts.push_back(m_ClippingVerts[i]);
			}
		}

		for (uint32_t i = 0; i < m_ClipOutputVerts.size(); ++i)
		{
			m_ClipOutputVerts[i].SVPosition.x /= m_ClipOutputVerts[i].SVPosition.w;
			m_ClipOutputVerts[i].SVPosition.y /= m_ClipOutputVerts[i].SVPosition.w;
			m_ClipOutputVerts[i].SVPosition.z /= m_ClipOutputVerts[i].SVPosition.w;
		}
	}

	void DeviceContext::ClipTriangleWithPlaneX(int nSign)
	{
		std::vector<VSOutputVertex> vCurrClipResultVerts = {};

		for (uint32_t i = 0; i < m_ClippingVerts.size(); i += 3)
		{
			int nOutOfClipPlaneNum = 0;

			VSOutputVertex& vert0 = m_ClippingVerts[i];
			VSOutputVertex& vert1 = m_ClippingVerts[i + 1];
			VSOutputVertex& vert2 = m_ClippingVerts[i + 2];

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

		m_ClippingVerts.clear();
		for (uint32_t i = 0; i < vCurrClipResultVerts.size(); ++i)
		{
			m_ClippingVerts.push_back(vCurrClipResultVerts[i]);
		}
	}

	void DeviceContext::ClipTriangleWithPlaneY(int nSign)
	{
		std::vector<VSOutputVertex> vCurrClipResultVerts = {};

		for (uint32_t i = 0; i < m_ClippingVerts.size(); i += 3)
		{
			int nOutOfClipPlaneNum = 0;

			VSOutputVertex& vert0 = m_ClippingVerts[i];
			VSOutputVertex& vert1 = m_ClippingVerts[i + 1];
			VSOutputVertex& vert2 = m_ClippingVerts[i + 2];

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

		m_ClippingVerts.clear();
		for (uint32_t i = 0; i < vCurrClipResultVerts.size(); ++i)
		{
			m_ClippingVerts.push_back(vCurrClipResultVerts[i]);
		}
	}

	void DeviceContext::ClipTriangleWithPlaneZeroZ()
	{
		std::vector<VSOutputVertex> vCurrClipResultVerts = {};

		for (uint32_t i = 0; i < m_ClippingVerts.size(); i += 3)
		{
			int nOutOfClipPlaneNum = 0;

			VSOutputVertex& vert0 = m_ClippingVerts[i];
			VSOutputVertex& vert1 = m_ClippingVerts[i + 1];
			VSOutputVertex& vert2 = m_ClippingVerts[i + 2];

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

		m_ClippingVerts.clear();
		for (uint32_t i = 0; i < vCurrClipResultVerts.size(); ++i)
		{
			m_ClippingVerts.push_back(vCurrClipResultVerts[i]);
		}
	}

	void DeviceContext::ClipTriangleWithPlanePositiveZ()
	{
		std::vector<VSOutputVertex> vCurrClipResultVerts = {};

		for (uint32_t i = 0; i < m_ClippingVerts.size(); i += 3)
		{
			int nOutOfClipPlaneNum = 0;

			VSOutputVertex& vert0 = m_ClippingVerts[i];
			VSOutputVertex& vert1 = m_ClippingVerts[i + 1];
			VSOutputVertex& vert2 = m_ClippingVerts[i + 2];

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

		m_ClippingVerts.clear();
		for (uint32_t i = 0; i < vCurrClipResultVerts.size(); ++i)
		{
			m_ClippingVerts.push_back(vCurrClipResultVerts[i]);
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
		m_AssembledVerts.clear();
		if (m_AssembledVerts.capacity() < nIndexNum)
		{
			m_AssembledVerts.reserve(nIndexNum);
		}

		if (m_PriTopology == RD_PRIMITIVE_TOPOLOGY::LINE_LIST || m_PriTopology == RD_PRIMITIVE_TOPOLOGY::TRIANGLE_LIST)
		{
			const uint32_t* pIndice = m_pIB->GetData();
			for (uint32_t i = 0; i < nIndexNum; ++i)
			{
				const VSOutputVertex& vert0 = m_VSOutputs[pIndice[i]];

				m_AssembledVerts.push_back(vert0);
			}
		}
	}

	void DeviceContext::BackFaceCulling()
	{
		m_BackFaceCulledVerts.clear();
		if (m_BackFaceCulledVerts.capacity() < m_AssembledVerts.size())
		{
			m_BackFaceCulledVerts.reserve(m_AssembledVerts.size());
		}

		for (uint32_t i = 0; i < m_AssembledVerts.size(); i += 3)
		{
			const Vector4& Pos1 = m_AssembledVerts[i].SVPosition;
			const Vector4& Pos2 = m_AssembledVerts[i + 1].SVPosition;
			const Vector4& Pos3 = m_AssembledVerts[i + 2].SVPosition;

			//三角形三个顶点若逆时针环绕，则行列式的值为正数，顺时针为负数，等于0则退化为线段
			if (GetArea2(Vector3(Pos1.x, Pos1.y, Pos1.z), Vector3(Pos2.x, Pos2.y, Pos2.z), Vector3(Pos3.x, Pos3.y, Pos3.z)) < 0.0f)
			{
				m_BackFaceCulledVerts.push_back(m_AssembledVerts[i]);
				m_BackFaceCulledVerts.push_back(m_AssembledVerts[i + 1]);
				m_BackFaceCulledVerts.push_back(m_AssembledVerts[i + 2]);
			}
		}
	}

	void DeviceContext::Rasterization()
	{
		for (uint32_t i = 0; i < m_ClipOutputVerts.size(); i += 3)
		{
			const VSOutputVertex& vert0 = m_ClipOutputVerts[i];
			const VSOutputVertex& vert1 = m_ClipOutputVerts[i + 1];
			const VSOutputVertex& vert2 = m_ClipOutputVerts[i + 2];

			if (m_PriTopology == RD_PRIMITIVE_TOPOLOGY::LINE_LIST)
			{
				DrawTriangleWithLine(vert0, vert1, vert2);
			}
			else if (m_PriTopology == RD_PRIMITIVE_TOPOLOGY::TRIANGLE_LIST)
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
			m_Desc(),
			m_hWndDC(nullptr),
			m_hBitMap(nullptr),
			m_hOldBitMap(nullptr)
		{}

		~SwapChain() = default;

		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;

		bool Init(const SwapChainDesc* pDesc);
		
		virtual void AddRef() override {}
		virtual void Release() override;

		virtual bool GetBuffer(void** ppSurface) override;
		virtual void GetDesc(SwapChainDesc* pDesc) override;
		virtual void Present() override;

	private:
		Texture2D*		m_pBackBuffer;
		
		SwapChainDesc   m_Desc;

		HDC				m_hWndDC;
		HBITMAP			m_hBitMap;
		HBITMAP			m_hOldBitMap;
	};

	bool SwapChain::Init(const SwapChainDesc* pDesc)
	{
		if (!pDesc)
		{
			return false;
		}

		m_Desc = *pDesc;

		m_pBackBuffer = new Texture2D();
		if (!m_pBackBuffer)
		{
			return false;
		}

		Texture2DDesc texDesc;
		texDesc.width = m_Desc.width;
		texDesc.height = m_Desc.height;
		texDesc.format = RD_FORMAT::UNKNOWN;   //这里不传入pDesc->format是为了Init Texture2D时避免分配内存，SwapChain的backbuffer的内存由CreateDIBSection来分配；
		if (!m_pBackBuffer->Init(&texDesc, nullptr))
		{
			return false;
		}

		void*& pTempBitMapBuffer = m_pBackBuffer->GetData();

		HDC hDC = GetDC(m_Desc.hOutputWindow);
		m_hWndDC = CreateCompatibleDC(hDC);
		ReleaseDC(m_Desc.hOutputWindow, hDC);

		uint16_t bitCnt = 0;
		uint32_t imageSize = 0;
		switch (pDesc->format)
		{
		case RD_FORMAT::R8G8B8A8_UNORM:
		{
			bitCnt = 32;
			imageSize = pDesc->width * pDesc->height * 4;

			m_pBackBuffer->SetFormat(RD_FORMAT::R8G8B8A8_UNORM);

			break;
		}
		case RD_FORMAT::UNKNOWN:
		{
			bitCnt = 0;
			imageSize = 0;

			return false;
			break;
		}
		default:
		{
			break;
		}
		}

		
		BITMAPINFO BitMapInfo =
		{
			{ sizeof(BITMAPINFOHEADER), (int)pDesc->width, -(int)pDesc->height, 1, bitCnt, BI_RGB, imageSize, 0, 0, 0, 0 }
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

		AddRef();

		return true;
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
			m_pBackBuffer->GetData() = nullptr;
		}

		if (m_Desc.hOutputWindow)
		{
			CloseWindow(m_Desc.hOutputWindow);
			m_Desc.hOutputWindow = nullptr;
		}

		if (m_pBackBuffer)
		{
			m_pBackBuffer->Release();
			m_pBackBuffer = nullptr;
		}

		delete this;
	}

	void SwapChain::Present()
	{
		HDC hDC = GetDC(m_Desc.hOutputWindow);
		BitBlt(hDC, 0, 0, m_Desc.width, m_Desc.height, m_hWndDC, 0, 0, SRCCOPY);
		ReleaseDC(m_Desc.hOutputWindow, hDC);
	}

	bool SwapChain::GetBuffer(void** ppSurface)
	{
		*ppSurface = m_pBackBuffer;
		m_pBackBuffer->AddRef();

		return true;
	}

	void SwapChain::GetDesc(SwapChainDesc* pDesc)
	{
		pDesc = &m_Desc;
	}
#pragma endregion SwapChain


	bool CreateDeviceAndSwapChain(IDevice** ppDevice, IDeviceContext** ppDeviceContext, ISwapChain** ppSwapChain, const SwapChainDesc* pSwapChainDesc)
	{
		Device *pDevice = new Device();
		if (!pDevice)
		{
			return false;
		}
		pDevice->AddRef();
		*ppDevice = pDevice;

		DeviceContext* pDeviceContext = new DeviceContext();
		if (!pDeviceContext)
		{
			return false;
		}

		if (!pDeviceContext->Init(pSwapChainDesc->width, pSwapChainDesc->height))
		{
			return false;
		}
		*ppDeviceContext = pDeviceContext;

		SwapChain* pSwapChain = new SwapChain();
		if (!pSwapChain)
		{
			return false;
		}
		if (!pSwapChain->Init(pSwapChainDesc))
		{
			return false;
		}
		*ppSwapChain = pSwapChain;

		return true;
	}

#pragma region Shader
	VSOutputVertex VertexShader::VSMain(const Vertex& inVertex, const Matrix4x4& matWorld, const Matrix4x4& matView, const Matrix4x4& matProj) const
	{
		VSOutputVertex Output = {};
		Vector4 vInPos = Vector4(inVertex.vPosition, 1.0f);
		vInPos = vInPos * matWorld;
		vInPos = vInPos * matView;
		vInPos = vInPos * matProj;

		Output.SVPosition.x = vInPos.x;
		Output.SVPosition.y = vInPos.y;
		Output.SVPosition.z = vInPos.z;
		Output.SVPosition.w = vInPos.w;

		Output.Color = Vector4(inVertex.vColor, 1.0f);

		Vector4 vNormal = Vector4(inVertex.vNormal, 0.0f);
		vNormal = vNormal * matWorld;
		Output.Normal.x = vNormal.x;
		Output.Normal.y = vNormal.y;
		Output.Normal.z = vNormal.z;

		Vector4 vTangent = Vector4(inVertex.vTangent.x, inVertex.vTangent.y, inVertex.vTangent.z, 0.0f);
		vTangent = vTangent * matWorld;
		Output.Tangent.x = vTangent.x;
		Output.Tangent.y = vTangent.y;
		Output.Tangent.z = vTangent.z;
		Output.Tangent.w = inVertex.vTangent.w;

		Output.Texcoord = inVertex.vTexcoord;

		return Output;
	}

	Vector4 PixelShader::PSMain(const VSOutputVertex& PSInput, const ShaderResourceTexture* pSRTexture) const
	{
		Vector2 vUV = PSInput.Texcoord;

		Vector4 TextureColor = Sample(pSRTexture, vUV);

		//Debug Normal Map
		float fTangentNormalX = TextureColor.x;
		float fTangentNormalY = TextureColor.y;
		float fTangentNormalZ = TextureColor.z;
		fTangentNormalX = fTangentNormalX * 2.0f - 1.0f;
		fTangentNormalY = fTangentNormalY * 2.0f - 1.0f;
		fTangentNormalZ = fTangentNormalZ * 2.0f - 1.0f;

		Vector3 TangentNormal = Normalize(Vector3(fTangentNormalX, fTangentNormalY, fTangentNormalZ));

		Vector3 Tangent = Normalize(Vector3(PSInput.Tangent.x, PSInput.Tangent.y, PSInput.Tangent.z));
		Vector3 BiTangent = Normalize(CrossProduct(PSInput.Normal, Tangent) * PSInput.Tangent.w);
		Vector3 VertexNormal = Normalize(PSInput.Normal);

		Vector4 vT = Vector4(Tangent, 0.0f);
		Vector4 vB = Vector4(BiTangent, 0.0f);
		Vector4 vN = Vector4(VertexNormal, 0.0f);

		Matrix4x4 matTBN(vT, vB, vN, Vector4(0, 0, 0, 1));

		Vector4 WorldNormal = Vector4(TangentNormal, 0.0f) * matTBN;

		Vector3 DiffuseColor = CalcPhongLighing(m_pMainLight, Vector3(WorldNormal.x, WorldNormal.y, WorldNormal.z), Vector3(1.0f, 1.0f, 1.0f));

		return Vector4(DiffuseColor, 1.0f);
	}

	Vector4 PixelShader::Sample(const ShaderResourceTexture* pSRTexture, const Vector2& vUV) const
	{
		const Vector4* pColorData = pSRTexture->pColor;

		uint32_t nWidth = pSRTexture->width;
		uint32_t nHeight = pSRTexture->height;

		float fU = vUV.x - std::floor(vUV.x);
		float fV = vUV.y - std::floor(vUV.y);

		uint32_t nRow = (uint32_t)(fV * (nHeight - 1));
		uint32_t nCol = (uint32_t)(fU * (nWidth - 1));

		Vector4 color = pColorData[nRow * nWidth + nCol];

		return color;
	}

	Vector3 PixelShader::CalcPhongLighing(const MainLight* light, const Vector3& normal, const Vector3& faceColor) const
	{
		Vector3 worldLightDir = -Normalize(light->direction);
		worldLightDir = Normalize(worldLightDir);
		Vector3 worldNormal = Normalize(normal);

		float fDiffuse = DotProduct(worldLightDir, worldNormal);

		fDiffuse = Clamp(fDiffuse, 0.0f, 1.0f);

		return fDiffuse * light->color * light->luminance * faceColor;
	}
#pragma endregion Shader
}