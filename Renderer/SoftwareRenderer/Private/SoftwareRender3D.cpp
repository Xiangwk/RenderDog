///////////////////////////////////
//RenderDog <·,·>
//FileName: SoftwareRender3D.cpp
//Written by Xiang Weikang
///////////////////////////////////

#include "SoftwareRender3D.h"
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
	class SamplerState;

	struct VSOutputVertex
	{
		VSOutputVertex() = default;
		VSOutputVertex(const VSOutputVertex& v) = default;
		VSOutputVertex& operator=(const VSOutputVertex& v) = default;

		Vector4 svPostion;
		Vector4 color;
		Vector3 normal;
		Vector4 tangent;
		Vector2 texcoord;
	};

	class VertexShader : public ISRVertexShader
	{
	public:
		VertexShader() = default;
		~VertexShader() = default;

		virtual void AddRef() override {}
		virtual void Release() override { delete this; }

		VSOutputVertex VSMain(const LocalVertex& inVertex, const Matrix4x4& matWorld, const Matrix4x4& matView, const Matrix4x4& matProj) const;
	};

	class PixelShader : public ISRPixelShader
	{
	private:
		struct MainLight
		{
			Vector4	color;
			Vector3	direction;	//从光源发射光线的方向
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

		virtual void	AddRef() override {}
		virtual void	Release() override;

		void			SetMainLight(const Vector4& color, const Vector3& direction, float luma);
		void			SetSamplerState(uint32_t startSlot, SamplerState* pSamplerState);

		Vector4			PSMain(const VSOutputVertex& VSOutput, const ShaderResourceTexture* pSRTexture) const;

	private:
		Vector3 CalcPhongLighing(const MainLight* light, const Vector3& normal, const Vector3& faceColor) const;

	private:
		MainLight*		m_pMainLight;
		SamplerState*	m_pSampler;
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

	void PixelShader::SetMainLight(const Vector4& color, const Vector3& direction, float luma)
	{
		m_pMainLight->color = color;
		m_pMainLight->direction = direction;
		m_pMainLight->luminance = luma;
	}

	void PixelShader::SetSamplerState(uint32_t startSlot, SamplerState* pSamplerState)
	{
		m_pSampler = pSamplerState;
	}
#pragma endregion Shader

#pragma region Texture2D

	class Texture2D : public ISRTexture2D
	{
	public:
		Texture2D();
		~Texture2D();

		virtual void		AddRef() override { ++m_RefCnt; }
		virtual void		Release() override;

		virtual void		GetType(SR_RESOURCE_DIMENSION* pResDimension) override { *pResDimension = SR_RESOURCE_DIMENSION::TEXTURE2D; }
		virtual void		GetDesc(SRTexture2DDesc* pDesc) override { *pDesc = m_Desc; }

		bool				Init(const SRTexture2DDesc* pDesc, const SRSubResourceData* pInitData);
		void				SetFormat(SR_FORMAT format) { m_Desc.format = format; }

		void*&				GetData() { return m_pData; }
		const void*			GetData() const { return m_pData; }

		void				SetWidth(uint32_t width) { m_Desc.width = width; }
		void				SetHeight(uint32_t Height) { m_Desc.height = Height; }

		uint32_t			GetWidth() const { return m_Desc.width; }
		uint32_t			GetHeight() const { return m_Desc.height; }

	private:
		int					m_RefCnt;
		void*				m_pData;

		SRTexture2DDesc		m_Desc;
	};

	Texture2D::Texture2D() :
		m_RefCnt(0),
		m_pData(nullptr),
		m_Desc()
	{}

	bool Texture2D::Init(const SRTexture2DDesc* pDesc, const SRSubResourceData* pInitData)
	{
		m_Desc = *pDesc;
		uint32_t dataNum = pDesc->width * pDesc->height;

		if (pDesc->format == SR_FORMAT::R8G8B8A8_UNORM)
		{
			m_pData = new uint32_t[dataNum];
		}
		else if (pDesc->format == SR_FORMAT::R32_FLOAT)
		{
			m_pData = new float[dataNum];
		}
		else if (pDesc->format == SR_FORMAT::R32G32B32A32_FLOAT)
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

#pragma  region SamplerState
	class SamplerState : public ISRSamplerState
	{
	public:
		SamplerState();
		~SamplerState();

		virtual void		AddRef() override { ++m_RefCnt; }
		virtual void		Release() override;
		virtual void		GetDesc(SRSamplerDesc* pDesc) override { *pDesc = m_Desc; }

		bool				Init(const SRSamplerDesc* pDesc);

		Vector4				Sample(const ShaderResourceTexture* pSRTexture, const Vector2& vUV);

	private:
		int					m_RefCnt;
		SRSamplerDesc		m_Desc;
	};

	SamplerState::SamplerState() :
		m_RefCnt(0),
		m_Desc()
	{}

	SamplerState::~SamplerState()
	{}

	void SamplerState::Release()
	{
		--m_RefCnt;
		if (m_RefCnt == 0)
		{
			delete this;
		}
	}

	bool SamplerState::Init(const SRSamplerDesc* pDesc)
	{
		m_Desc = *pDesc;

		AddRef();

		return true;
	}

	Vector4 SamplerState::Sample(const ShaderResourceTexture* pSRTexture, const Vector2& vUV)
	{
		const Vector4* pColorData = pSRTexture->pColor;

		uint32_t width = pSRTexture->width;
		uint32_t height = pSRTexture->height;

		float texcoordU = vUV.x - std::floor(vUV.x);
		float texcoordV = vUV.y - std::floor(vUV.y);

		uint32_t row = (uint32_t)(texcoordV * (height - 1));
		uint32_t col = (uint32_t)(texcoordU * (width - 1));

		Vector4 color = pColorData[row * width + col];

		return color;
	}


#pragma endregion SamplerState


#pragma region View
	class RenderTargetView : public ISRRenderTargetView
	{
	public:
		RenderTargetView() :
			m_RefCnt(0),
			m_pViewResource(nullptr),
			m_Desc()
		{}
		~RenderTargetView()
		{}

		bool Init(ISFResource* pResource, const SRRenderTargetViewDesc* pDesc);

		virtual void AddRef() override { ++m_RefCnt; }
		virtual void Release() override;

		virtual void GetResource(ISFResource** ppResource) override { *ppResource = m_pViewResource; }

		virtual void GetDesc(SRRenderTargetViewDesc* pDesc) override { *pDesc = m_Desc; }

	private:
		int						m_RefCnt;

		ISFResource*				m_pViewResource;
		SRRenderTargetViewDesc	m_Desc;
	};

	bool RenderTargetView::Init(ISFResource* pResource, const SRRenderTargetViewDesc* pDesc)
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

	class DepthStencilView : public ISRDepthStencilView
	{
	public:
		DepthStencilView() :
			m_RefCnt(0),
			m_pViewResource(nullptr),
			m_Desc()
		{}
		~DepthStencilView()
		{}

		bool Init(ISFResource* pResource, const SRDepthStencilViewDesc* pDesc);

		virtual void AddRef() override { ++m_RefCnt; }
		virtual void Release() override;

		virtual void GetResource(ISFResource** ppResource) override { *ppResource = m_pViewResource; }

		virtual void GetDesc(SRDepthStencilViewDesc* pDesc) override { *pDesc = m_Desc; }

	private:
		int						m_RefCnt;

		ISFResource*				m_pViewResource;
		SRDepthStencilViewDesc	m_Desc;
	};

	bool DepthStencilView::Init(ISFResource* pResource, const SRDepthStencilViewDesc* pDesc)
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

	class ShaderResourceView : public ISRShaderResourceView
	{
	public:
		ShaderResourceView() :
			m_RefCnt(0),
			m_pViewResource(nullptr),
			m_Desc()
		{}

		~ShaderResourceView()
		{}

		bool Init(ISFResource* pResource, const SRShaderResourceViewDesc* pDesc);

		virtual void AddRef() override { ++m_RefCnt; }
		virtual void Release() override;

		virtual void GetResource(ISFResource** ppResource) override { *ppResource = m_pViewResource; }

		virtual void GetDesc(SRShaderResourceViewDesc* pDesc) override { *pDesc = m_Desc; }

	private:
		int						m_RefCnt;

		ISFResource*				m_pViewResource;
		SRShaderResourceViewDesc	m_Desc;
	};

	bool ShaderResourceView::Init(ISFResource* pResource, const SRShaderResourceViewDesc* pDesc)
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
	class VertexBuffer : public ISRBuffer
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

		bool				Init(const SRBufferDesc* pDesc, const SRSubResourceData* pInitData);

		virtual void		AddRef() override { ++m_RefCnt; }
		virtual void		Release() override;

		virtual void		GetType(SR_RESOURCE_DIMENSION* pResDimension) override { *pResDimension = SR_RESOURCE_DIMENSION::BUFFER; }

		virtual void		GetDesc(SRBufferDesc* pDesc) override { *pDesc = m_Desc; }

		const LocalVertex*	GetData() const { return m_pData; }
		const uint32_t		GetNum() const { return m_nVertsNum; }

	private:
		int					m_RefCnt;
		SRBufferDesc		m_Desc;

		LocalVertex*		m_pData;
		uint32_t			m_nVertsNum;
	};

	bool VertexBuffer::Init(const SRBufferDesc* pDesc, const SRSubResourceData* pInitData)
	{
		if (!pDesc)
		{
			return false;
		}

		m_Desc = *pDesc;

		m_nVertsNum = pDesc->byteWidth / sizeof(LocalVertex);
		m_pData = new LocalVertex[m_nVertsNum];
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

	class IndexBuffer : public ISRBuffer
	{
	public:
		IndexBuffer() :
			m_RefCnt(0),
			m_Desc(),
			m_pData(nullptr)
		{}

		~IndexBuffer()
		{}

		bool			Init(const SRBufferDesc* pDesc, const SRSubResourceData* pInitData);

		virtual void	AddRef() override { ++m_RefCnt; }
		virtual void	Release() override;

		virtual void	GetType(SR_RESOURCE_DIMENSION* pResDimension) override { *pResDimension = SR_RESOURCE_DIMENSION::BUFFER; }

		virtual void	GetDesc(SRBufferDesc* pDesc) override { *pDesc = m_Desc; }

		const uint32_t* GetData() const { return m_pData; }

	private:
		int				m_RefCnt;
		SRBufferDesc	m_Desc;

		uint32_t*		m_pData;
	};

	bool IndexBuffer::Init(const SRBufferDesc* pDesc, const SRSubResourceData* pInitData)
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

	class ConstantBuffer : public ISRBuffer
	{
	public:
		ConstantBuffer() :
			m_RefCnt(0),
			m_Desc(),
			m_pData(nullptr)
		{}
		~ConstantBuffer()
		{}

		bool				Init(const SRBufferDesc* pDesc, const SRSubResourceData* pInitData);

		virtual void		AddRef() override { ++m_RefCnt; }
		virtual void		Release() override;

		virtual void		GetType(SR_RESOURCE_DIMENSION* pResDimension) override { *pResDimension = SR_RESOURCE_DIMENSION::BUFFER; }

		virtual void		GetDesc(SRBufferDesc* pDesc) override { *pDesc = m_Desc; }

		const void*			GetData() const { return m_pData; }
		void*				GetData() { return m_pData; }

	private:
		int					m_RefCnt;

		SRBufferDesc		m_Desc;

		void*				m_pData;
	};

	bool ConstantBuffer::Init(const SRBufferDesc* pDesc, const SRSubResourceData* pInitData)
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
	class Device : public ISRDevice
	{
	public:
		Device() = default;
		~Device() = default;

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;

		virtual bool CreateTexture2D(const SRTexture2DDesc* pDesc, const SRSubResourceData* pInitData, ISRTexture2D** ppTexture) override;
		virtual bool CreateRenderTargetView(ISFResource* pResource, const SRRenderTargetViewDesc* pDesc, ISRRenderTargetView** ppRenderTargetView) override;
		virtual bool CreateDepthStencilView(ISFResource* pResource, const SRDepthStencilViewDesc* pDesc, ISRDepthStencilView** ppDepthStencilView) override;
		virtual bool CreateShaderResourceView(ISFResource* pResource, const SRShaderResourceViewDesc* pDesc, ISRShaderResourceView** ppShaderResourceView) override;
		virtual bool CreateBuffer(const SRBufferDesc* pDesc, const SRSubResourceData* pInitData, ISRBuffer** ppBuffer) override;
		virtual bool CreateVertexShader(ISRVertexShader** ppVertexShader) override;
		virtual bool CreatePixelShader(ISRPixelShader** ppPixelShader) override;
		virtual bool CreateSamplerState(const SRSamplerDesc* pDesc, ISRSamplerState** ppSamplerState) override;

		virtual void AddRef() override {}
		virtual void Release() override { delete this; }

	private:
		bool CreateVertexBuffer(const SRBufferDesc* pDesc, const SRSubResourceData* pInitData, ISRBuffer** ppBuffer);
		bool CreateIndexBuffer(const SRBufferDesc* pDesc, const SRSubResourceData* pInitData, ISRBuffer** ppBuffer);
		bool CreateConstantBuffer(const SRBufferDesc* pDesc, const SRSubResourceData* pInitData, ISRBuffer** ppBuffer);
	};


	bool Device::CreateTexture2D(const SRTexture2DDesc* pDesc, const SRSubResourceData* pInitData, ISRTexture2D** ppTexture)
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

	bool Device::CreateRenderTargetView(ISFResource* pResource, const SRRenderTargetViewDesc* pDesc, ISRRenderTargetView** ppRenderTargetView)
	{
		RenderTargetView* pRTV = new RenderTargetView();
		if (!pRTV)
		{
			return false;
		}

		SR_RESOURCE_DIMENSION resDimension;
		pResource->GetType(&resDimension);
		if (resDimension == SR_RESOURCE_DIMENSION::TEXTURE2D)
		{
			SRRenderTargetViewDesc rtvDesc;
			rtvDesc.viewDimension = SR_RTV_DIMENSION::TEXTURE2D;
			pDesc = &rtvDesc;
		}

		if (!pRTV->Init(pResource, pDesc))
		{
			return false;
		}

		*ppRenderTargetView = pRTV;

		return true;
	}

	bool Device::CreateDepthStencilView(ISFResource* pResource, const SRDepthStencilViewDesc* pDesc, ISRDepthStencilView** ppDepthStencilView)
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

	bool Device::CreateShaderResourceView(ISFResource* pResource, const SRShaderResourceViewDesc* pDesc, ISRShaderResourceView** ppShaderResourceView)
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

	bool Device::CreateBuffer(const SRBufferDesc* pDesc, const SRSubResourceData* pInitData, ISRBuffer** ppBuffer)
	{
		if (!pDesc)
		{
			return false;
		}

		switch (pDesc->bindFlag)
		{
		case SR_BIND_FLAG::BIND_VERTEX_BUFFER:
		{
			return CreateVertexBuffer(pDesc, pInitData, ppBuffer);
		}
		case SR_BIND_FLAG::BIND_INDEX_BUFFER:
		{
			return CreateIndexBuffer(pDesc, pInitData, ppBuffer);
		}
		case SR_BIND_FLAG::BIND_CONSTANT_BUFFER:
		{
			return CreateConstantBuffer(pDesc, pInitData, ppBuffer);
		}
		default:
			break;
		}
		
		return false;
	}

	bool Device::CreateVertexShader(ISRVertexShader** ppVertexShader)
	{
		VertexShader* pVS = new VertexShader();
		if (!pVS)
		{
			return false;
		}

		*ppVertexShader = pVS;

		return true;
	}

	bool Device::CreatePixelShader(ISRPixelShader** ppPixelShader)
	{
		PixelShader* pPS = new PixelShader();
		if (!pPS)
		{
			return false;
		}

		*ppPixelShader = pPS;

		return true;
	}

	bool Device::CreateSamplerState(const SRSamplerDesc* pDesc, ISRSamplerState** ppSamplerState)
	{
		SamplerState* pSamplerState = new SamplerState();
		if (!pSamplerState)
		{
			return false;
		}

		if (!pSamplerState->Init(pDesc))
		{
			return false;
		}

		*ppSamplerState = pSamplerState;

		return true;
	}

	bool Device::CreateVertexBuffer(const SRBufferDesc* pDesc, const SRSubResourceData* pInitData, ISRBuffer** ppBuffer)
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

	bool Device::CreateIndexBuffer(const SRBufferDesc* pDesc, const SRSubResourceData* pInitData, ISRBuffer** ppBuffer)
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

	bool Device::CreateConstantBuffer(const SRBufferDesc* pDesc, const SRSubResourceData* pInitData, ISRBuffer** ppBuffer)
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
	class DeviceContext : public ISRDeviceContext
	{
	public:
		DeviceContext();
		~DeviceContext();

		DeviceContext(const DeviceContext&) = delete;
		DeviceContext& operator=(const DeviceContext&) = delete;

		bool Init(uint32_t width, uint32_t height);

		virtual void AddRef() override {}
		virtual void Release() override { delete this; }

		virtual void IASetVertexBuffer(ISRBuffer* pVB) override;
		virtual void IASetIndexBuffer(ISRBuffer* pIB) override;
		virtual void IASetPrimitiveTopology(SR_PRIMITIVE_TOPOLOGY topology) override { m_PriTopology = topology; }

		virtual	void UpdateSubresource(ISFResource* pDstResource, const void* pSrcData, uint32_t srcRowPitch, uint32_t srcDepthPitch) override;

		virtual void VSSetShader(ISRVertexShader* pVS) override { m_pVS = dynamic_cast<VertexShader*>(pVS); }
		virtual void VSSetConstantBuffer(uint32_t startSlot, ISRBuffer* const* ppConstantBuffer) override;
		virtual void PSSetConstantBuffer(uint32_t startSlot, ISRBuffer* const* ppConstantBuffer) override;
		virtual void PSSetShader(ISRPixelShader* pPS) override { m_pPS = dynamic_cast<PixelShader*>(pPS); }
		virtual void PSSetShaderResource(ISRShaderResourceView* const* ppShaderResourceView) override;
		virtual void PSSetSampler(uint32_t startSlot, ISRSamplerState* const* ppSamplerState) override;

		virtual void RSSetViewport(const SRViewport* pVP) override;

		virtual void OMSetRenderTarget(ISRRenderTargetView* pRenderTargetView, ISRDepthStencilView* pDepthStencilVew) override;
		virtual void ClearRenderTargetView(ISRRenderTargetView* pRenderTargetView, const float* clearColor) override;
		virtual void ClearDepthStencilView(ISRDepthStencilView* pDepthStencilView, float fDepth) override;
		virtual void Draw() override;
		virtual void DrawIndex(uint32_t indexNum) override;
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

		void ViewportTransform();

		void ClipTriangleWithPlaneX(int nSign); //sign为+1或者-1
		void ClipTriangleWithPlaneY(int nSign); //sign为+1或者-1
		void ClipTriangleWithPlaneZeroZ();
		void ClipTriangleWithPlanePositiveZ();

		void ClipTwoVertsInTriangle(const VSOutputVertex& vertIn, VSOutputVertex& vertOut1, VSOutputVertex& vertOut2, float lerpFactor1, float lerpFactor2);
		void ClipOneVertInTriangle(VSOutputVertex& vertOut, const VSOutputVertex& vertIn1, const VSOutputVertex& vertIn2, float lerpFactor1, float lerpFactor2, std::vector<VSOutputVertex>& tempVerts);
		float GetClipLerpFactorX(const VSOutputVertex& vert0, const VSOutputVertex& vert1, int sign);
		float GetClipLerpFactorY(const VSOutputVertex& vert0, const VSOutputVertex& vert1, int sign);
		float GetClipLerpFactorZeroZ(const VSOutputVertex& vert0, const VSOutputVertex& vert1);
		float GetClipLerpFactorPositiveZ(const VSOutputVertex& vert0, const VSOutputVertex& vert1);

		void ShapeAssemble(uint32_t indexNum);

		void BackFaceCulling();

		void Rasterization();

	private:
		uint32_t*					m_pFrameBuffer;
		float*						m_pDepthBuffer;
		uint32_t					m_BackBufferWidth;
		uint32_t					m_BackBufferHeight;

		VertexBuffer*				m_pVB;
		IndexBuffer*				m_pIB;
		ConstantBuffer*				m_pVertexShaderCB[2];
		ConstantBuffer*				m_pPixelShaderCB;

		VertexShader*				m_pVS;
		PixelShader*				m_pPS;

		ShaderResourceTexture		m_SRTexture;

		std::vector<VSOutputVertex>	m_VSOutputs;
		std::vector<VSOutputVertex> m_AssembledVerts;
		std::vector<VSOutputVertex> m_BackFaceCulledVerts;
		std::vector<VSOutputVertex>	m_ClipOutputVerts;
		std::vector<VSOutputVertex> m_ClippingVerts;

		Matrix4x4					m_ViewportMatrix;

		SR_PRIMITIVE_TOPOLOGY		m_PriTopology;
	};

	DeviceContext::DeviceContext() :
		m_pFrameBuffer(nullptr),
		m_pDepthBuffer(nullptr),
		m_BackBufferWidth(0),
		m_BackBufferHeight(0),
		m_pVB(nullptr),
		m_pIB(nullptr),
		m_pVS(nullptr),
		m_pPS(nullptr),
		m_SRTexture(),
		m_PriTopology(SR_PRIMITIVE_TOPOLOGY::TRIANGLE_LIST)
	{
		m_pVertexShaderCB[0] = nullptr;
		m_pVertexShaderCB[1] = nullptr;

		m_pPixelShaderCB = nullptr;

		m_ViewportMatrix.Identity();
	}

	DeviceContext::~DeviceContext()
	{
		m_VSOutputs.clear();
		m_ClipOutputVerts.clear();
		m_AssembledVerts.clear();
		m_ClippingVerts.clear();
	}

	bool DeviceContext::Init(uint32_t width, uint32_t height)
	{
		m_BackBufferWidth = width;
		m_BackBufferHeight = height;

		AddRef();

		return true;
	}

	void DeviceContext::IASetVertexBuffer(ISRBuffer* pVB)
	{
		if (m_pVB != pVB)
		{
			m_VSOutputs.clear();

			m_pVB = dynamic_cast<VertexBuffer*>(pVB);

			uint32_t nVertexNum = m_pVB->GetNum();
			m_VSOutputs.resize(nVertexNum);
		}
	}
	void DeviceContext::IASetIndexBuffer(ISRBuffer* pIB)
	{
		if (m_pIB != pIB)
		{
			m_pIB = dynamic_cast<IndexBuffer*>(pIB);
		}
	}

	void DeviceContext::UpdateSubresource(ISFResource* pDstResource, const void* pSrcData, uint32_t srcRowPitch, uint32_t srcDepthPitch)
	{
		SR_RESOURCE_DIMENSION resDimension;
		pDstResource->GetType(&resDimension);

		switch (resDimension)
		{
		case RenderDog::SR_RESOURCE_DIMENSION::UNKNOWN:
		{
			return;
			break;
		}
		case RenderDog::SR_RESOURCE_DIMENSION::BUFFER:
		{
			ISRBuffer* pBuffer = dynamic_cast<ISRBuffer*>(pDstResource);
			SRBufferDesc bufferDesc;
			pBuffer->GetDesc(&bufferDesc);
			if (bufferDesc.bindFlag == SR_BIND_FLAG::BIND_CONSTANT_BUFFER)
			{
				ConstantBuffer* pCB = dynamic_cast<ConstantBuffer*>(pBuffer);
				memcpy(pCB->GetData(), pSrcData, bufferDesc.byteWidth);
			}
			
			break;
		}
		case RenderDog::SR_RESOURCE_DIMENSION::TEXTURE2D:
		{
			break;
		}
		default:
			break;
		}
	}

	void DeviceContext::VSSetConstantBuffer(uint32_t startSlot, ISRBuffer* const* ppConstantBuffer)
	{
		m_pVertexShaderCB[startSlot] = dynamic_cast<ConstantBuffer*>(*ppConstantBuffer);
	}

	void DeviceContext::PSSetConstantBuffer(uint32_t startSlot, ISRBuffer* const* ppConstantBuffer)
	{
		m_pPixelShaderCB = dynamic_cast<ConstantBuffer*>(*ppConstantBuffer);

		float* pData = static_cast<float*>(m_pPixelShaderCB->GetData());
		Vector4 mainLightColor(pData[0], pData[1], pData[2], pData[3]);
		Vector3 mainLightDir(pData[4], pData[5], pData[6]);
		float mainLightLuma = pData[7];

		m_pPS->SetMainLight(mainLightColor, mainLightDir, mainLightLuma);
	}

	void DeviceContext::PSSetShaderResource(ISRShaderResourceView* const* ppShaderResourceView)
	{ 
		ISFResource* pRes = nullptr;
		(*ppShaderResourceView)->GetResource(&pRes);

		SRShaderResourceViewDesc srvDesc;
		(*ppShaderResourceView)->GetDesc(&srvDesc);

		switch (srvDesc.viewDimension)
		{
		case SR_SRV_DIMENSION::TEXTURE2D:
		{
			Texture2D* pTex2D = dynamic_cast<Texture2D*>(pRes);
			SRTexture2DDesc texDesc;
			pTex2D->GetDesc(&texDesc);

			m_SRTexture.pColor = static_cast<Vector4*>(pTex2D->GetData());
			m_SRTexture.width = texDesc.width;
			m_SRTexture.height = texDesc.height;
		}
		default:
			break;
		}
	}

	void DeviceContext::PSSetSampler(uint32_t startSlot, ISRSamplerState* const* ppSamplerState)
	{
		if (m_pPS)
		{
			m_pPS->SetSamplerState(startSlot, dynamic_cast<SamplerState*>(*ppSamplerState));
		}
	}

	void DeviceContext::RSSetViewport(const SRViewport* pViewport)
	{
		m_ViewportMatrix(0, 0) = (pViewport->width - 1) / 2.0f;
		m_ViewportMatrix(3, 0) = (pViewport->width - 1) / 2.0f + pViewport->topLeftX;
		m_ViewportMatrix(1, 1) = -(pViewport->height - 1) / 2.0f;
		m_ViewportMatrix(3, 1) = (pViewport->height - 1) / 2.0f + pViewport->topLeftY;
		m_ViewportMatrix(2, 2) = (pViewport->maxDepth - pViewport->minDepth);
		m_ViewportMatrix(3, 2) = pViewport->minDepth;
	}

	void DeviceContext::OMSetRenderTarget(ISRRenderTargetView* pRenderTargetView, ISRDepthStencilView* pDepthStencilView)
	{
		ISFResource* pTex = nullptr;
		pRenderTargetView->GetResource(&pTex);
		
		SRRenderTargetViewDesc rtvDesc;
		pRenderTargetView->GetDesc(&rtvDesc);

		switch (rtvDesc.viewDimension)
		{
		case SR_RTV_DIMENSION::UNKNOWN:
		{
			m_pFrameBuffer = nullptr;
			break;
		}
		case SR_RTV_DIMENSION::BUFFER:
		{
			break;
		}
		case SR_RTV_DIMENSION::TEXTURE2D:
		{
			Texture2D* pTex2D = dynamic_cast<Texture2D*>(pTex);

			//TODO: use format to determine m_pFrameBuffer's type;
			m_pFrameBuffer = (uint32_t*)pTex2D->GetData();
			SRTexture2DDesc desc;
			pTex2D->GetDesc(&desc);

			m_BackBufferWidth = desc.width;
			m_BackBufferHeight = desc.height;

			break;
		}
		default:
			break;
		}
		
		
		pDepthStencilView->GetResource(&pTex);
		
		SRDepthStencilViewDesc dsvDesc;
		pDepthStencilView->GetDesc(&dsvDesc);

		switch (dsvDesc.viewDimension)
		{
		case SR_DSV_DIMENSION::UNKNOWN:
		{
			m_pDepthBuffer = nullptr;
			break;
		}
		case SR_DSV_DIMENSION::TEXTURE2D:
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

	void DeviceContext::ClearRenderTargetView(ISRRenderTargetView* pRenderTargetView, const float* clearColor)
	{
		Vector4 colorVector(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
		Vector4 ARGB = ConvertRGBAColorToARGBColor(colorVector);
		uint32_t nClearColor = ConvertColorToUInt32(ARGB);

		ISFResource* pTex = nullptr;
		pRenderTargetView->GetResource(&pTex);

		SRRenderTargetViewDesc rtvDesc;
		pRenderTargetView->GetDesc(&rtvDesc);
		
		if(rtvDesc.viewDimension == SR_RTV_DIMENSION::TEXTURE2D)
		{
			Texture2D* pTex2D = dynamic_cast<Texture2D*>(pTex);

			SRTexture2DDesc texDesc;
			pTex2D->GetDesc(&texDesc);

			uint32_t rtWidth = texDesc.width;
			uint32_t rtHeight = texDesc.height;
			if (texDesc.format == SR_FORMAT::R8G8B8A8_UNORM)
			{
				uint32_t* pRT = static_cast<uint32_t*>(pTex2D->GetData());
				for (uint32_t row = 0; row < rtHeight; ++row)
				{
					for (uint32_t col = 0; col < rtWidth; ++col)
					{
						uint32_t index = row * rtWidth + col;
						pRT[index] = nClearColor;
					}
				}
			}
		}
	}

	void DeviceContext::ClearDepthStencilView(ISRDepthStencilView* pDepthStencil, float depth)
	{
		ISFResource* pTex = nullptr;
		pDepthStencil->GetResource(&pTex);

		SRDepthStencilViewDesc dsvDesc;
		pDepthStencil->GetDesc(&dsvDesc);

		if (dsvDesc.viewDimension == SR_DSV_DIMENSION::TEXTURE2D)
		{
			Texture2D* pTex2D = dynamic_cast<Texture2D*>(pTex);
			SRTexture2DDesc texDesc;
			pTex2D->GetDesc(&texDesc);

			if (texDesc.format == SR_FORMAT::R32_FLOAT)
			{
				float* pDepth = static_cast<float*>(pTex2D->GetData());
				uint32_t width = texDesc.width;
				uint32_t height = texDesc.height;
				for (uint32_t row = 0; row < height; ++row)
				{
					for (uint32_t col = 0; col < width; ++col)
					{
						uint32_t index = row * width + col;
						pDepth[index] = depth;
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

	void DeviceContext::DrawIndex(uint32_t indexNum)
	{
		const LocalVertex* pVerts = m_pVB->GetData();
		//Local Space to Clip Space
		for (uint32_t i = 0; i < m_pVB->GetNum(); ++i)
		{
			const LocalVertex& vert = pVerts[i];

			Matrix4x4* pWorldMatrix = (Matrix4x4*)m_pVertexShaderCB[1]->GetData();
			Matrix4x4* pViewMatrix = (Matrix4x4*)m_pVertexShaderCB[0]->GetData() + 0;
			Matrix4x4* pProjMatrix = (Matrix4x4*)m_pVertexShaderCB[0]->GetData() + 1;
			m_VSOutputs[i] = m_pVS->VSMain(vert, *pWorldMatrix, *pViewMatrix, *pProjMatrix);
		}

		ShapeAssemble(indexNum);

		BackFaceCulling();

		ClipTrianglesInClipSpace();

		ViewportTransform();

		Rasterization();
	}


	//------------------------------------------------------------------------------------------------------------------
	//Private Funtion
	//------------------------------------------------------------------------------------------------------------------
	void DeviceContext::DrawLineWithDDA(float pos1X, float pos1Y, float pos2X, float pos2Y, const float* lineColor)
	{
		Vector4 lineColorVector(lineColor[0], lineColor[1], lineColor[2], lineColor[3]);
		uint32_t clearColor = ConvertColorToUInt32(lineColorVector);

		float DeltaX = pos2X - pos1X;
		float DeltaY = pos2Y - pos1Y;

		if (std::abs(DeltaX - 0.0f) < 0.000001f)
		{
			float yStart = std::fmin(pos1Y, pos2Y);
			float yEnd = std::fmax(pos1Y, pos2Y);
			for (int yStep = (int)yStart; yStep <= (int)yEnd; ++yStep)
			{
				m_pFrameBuffer[(int)pos1X + yStep * m_BackBufferWidth] = clearColor;
			}
			return;
		}

		float k = DeltaY / DeltaX;
		if (std::abs(k) <= 1.0f)
		{
			float xStart = pos1X < pos2X ? pos1X : pos2X;
			float xEnd = pos1X < pos2X ? pos2X : pos1X;
			float y = pos1X < pos2X ? pos1Y : pos2Y;
			for (int xStep = (int)xStart; xStep < (int)xEnd; ++xStep)
			{
				y += k;
				int yStep = (int)y;
				m_pFrameBuffer[xStep + yStep * m_BackBufferWidth] = clearColor;
			}
		}
		else
		{
			float yStart = pos1Y < pos2Y ? pos1Y : pos2Y;
			float yEnd = pos1Y < pos2Y ? pos2Y : pos1Y;
			float x = pos1Y < pos2Y ? pos1X : pos2X;
			for (int yStep = (int)yStart; yStep < (int)yEnd; ++yStep)
			{
				x += 1.0f / k;
				int xStep = (int)x;
				m_pFrameBuffer[xStep + yStep * m_BackBufferWidth] = clearColor;
			}
		}
	}

	void DeviceContext::DrawTriangleWithLine(const VSOutputVertex& v0, const VSOutputVertex& v1, const VSOutputVertex& v2)
	{
		float lineColor[4] = { v0.color.x, v0.color.y, v0.color.z, 1.0f };
		DrawLineWithDDA(v0.svPostion.x, v0.svPostion.y, v1.svPostion.x, v1.svPostion.y, lineColor);
		DrawLineWithDDA(v1.svPostion.x, v1.svPostion.y, v2.svPostion.x, v2.svPostion.y, lineColor);
		DrawLineWithDDA(v2.svPostion.x, v2.svPostion.y, v0.svPostion.x, v0.svPostion.y, lineColor);
	}

	void DeviceContext::DrawTriangleWithFlat(const VSOutputVertex& v0, const VSOutputVertex& v1, const VSOutputVertex& v2)
	{
		if (FloatEqual(v0.svPostion.y, v1.svPostion.y, RD_FLT_EPSILON) && FloatEqual(v0.svPostion.y, v2.svPostion.y, RD_FLT_EPSILON) ||
			FloatEqual(v0.svPostion.x, v1.svPostion.x, RD_FLT_EPSILON) && FloatEqual(v0.svPostion.x, v2.svPostion.x, RD_FLT_EPSILON))
		{
			return;
		}

		VSOutputVertex vert0(v0);
		VSOutputVertex vert1(v1);
		VSOutputVertex vert2(v2);
		SortTriangleVertsByYGrow(vert0, vert1, vert2);

		if (FloatEqual(vert0.svPostion.y, vert1.svPostion.y, RD_FLT_EPSILON))
		{
			DrawTopTriangle(vert0, vert1, vert2);
		}
		else if (FloatEqual(vert1.svPostion.y, vert2.svPostion.y, RD_FLT_EPSILON))
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
		if (v1.svPostion.y <= v0.svPostion.y)
		{
			VSOutputVertex vTemp = v0;
			v0 = v1;
			v1 = vTemp;
		}

		if (v2.svPostion.y <= v0.svPostion.y)
		{
			VSOutputVertex vTemp = v0;
			v0 = v2;
			v2 = vTemp;
		}

		if (v2.svPostion.y <= v1.svPostion.y)
		{
			VSOutputVertex vTemp = v1;
			v1 = v2;
			v2 = vTemp;
		}
	}

	void DeviceContext::SortScanlineVertsByXGrow(VSOutputVertex& v0, VSOutputVertex& v1)
	{
		if (v1.svPostion.x <= v0.svPostion.x)
		{
			VSOutputVertex vTemp = v0;
			v0 = v1;
			v1 = vTemp;
		}
	}

	void DeviceContext::DrawTopTriangle(VSOutputVertex& v0, VSOutputVertex& v1, VSOutputVertex& v2)
	{
		SortScanlineVertsByXGrow(v0, v1);

		float yStart = std::ceilf(v0.svPostion.y - 0.5f);
		float yEnd = std::ceilf(v2.svPostion.y - 0.5f);
		float deltaY = v2.svPostion.y - v0.svPostion.y;

		for (uint32_t i = (uint32_t)yStart; i < (uint32_t)yEnd; ++i)
		{
			float lerpFactorY = (i + 0.5f - v0.svPostion.y) / deltaY;

			VSOutputVertex vStart;
			LerpVertexParamsInScreen(v0, v2, vStart, lerpFactorY);
			VSOutputVertex vEnd;
			LerpVertexParamsInScreen(v1, v2, vEnd, lerpFactorY);

			float xStart = std::ceilf(vStart.svPostion.x - 0.5f);
			float xEnd = std::ceilf(vEnd.svPostion.x - 0.5f);

			float deltaX = vEnd.svPostion.x - vStart.svPostion.x;
			for (uint32_t j = (uint32_t)xStart; j < (uint32_t)xEnd; ++j)
			{
				float lerpFactorX = (j + 0.5f - vStart.svPostion.x) / deltaX;

				VSOutputVertex vCurr;
				LerpVertexParamsInScreen(vStart, vEnd, vCurr, lerpFactorX);

				float pixelDepth = m_pDepthBuffer[j + i * m_BackBufferWidth];
				if (vCurr.svPostion.z <= pixelDepth)
				{
#ifndef RD_DEBUG_RASTERIZATION
					Vector4 color = m_pPS->PSMain(vCurr, &m_SRTexture);
					Vector4 argbColor = ConvertRGBAColorToARGBColor(color);
					m_pFrameBuffer[j + i * m_BackBufferWidth] = ConvertColorToUInt32(argbColor);
#else // defined RD_DEBUG_RASTERIZATION
					Vector4 color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
					Vector4 overDrawColor = Vector4(0.0f, 1.0f, 0.0f, 1.0f);

					uint32_t colorUint = ConvertColorToUInt32(ConvertRGBAColorToARGBColor(color));
					
					uint32_t drawColor = (m_pFrameBuffer[j + i * m_BackBufferWidth] == colorUint) ?
										ConvertColorToUInt32(ConvertRGBAColorToARGBColor(overDrawColor)) :
										colorUint;
					
					m_pFrameBuffer[j + i * m_BackBufferWidth] = drawColor;
#endif // RD_DEBUG_RASTERIZATION

					m_pDepthBuffer[j + i * m_BackBufferWidth] = vCurr.svPostion.z;
				}
			}
		}
	}

	void DeviceContext::DrawBottomTriangle(VSOutputVertex& v0, VSOutputVertex& v1, VSOutputVertex& v2)
	{
		SortScanlineVertsByXGrow(v1, v2);

		float yStart = std::ceilf(v0.svPostion.y - 0.5f);
		float yEnd = std::ceilf(v1.svPostion.y - 0.5f);
		float deltaY = v1.svPostion.y - v0.svPostion.y;

		for (uint32_t i = (uint32_t)yStart; i < (uint32_t)yEnd; ++i)
		{
			float lerpFactorY = (i + 0.5f - v0.svPostion.y) / deltaY;

			VSOutputVertex vStart;
			LerpVertexParamsInScreen(v0, v1, vStart, lerpFactorY);
			VSOutputVertex vEnd;
			LerpVertexParamsInScreen(v0, v2, vEnd, lerpFactorY);

			float xStart = std::ceilf(vStart.svPostion.x - 0.5f);
			float xEnd = std::ceilf(vEnd.svPostion.x - 0.5f);

			float deltaX = vEnd.svPostion.x - vStart.svPostion.x;
			for (uint32_t j = (uint32_t)xStart; j < (uint32_t)xEnd; ++j)
			{
				float lerpFactorX = (j + 0.5f - vStart.svPostion.x) / deltaX;

				VSOutputVertex vCurr;
				LerpVertexParamsInScreen(vStart, vEnd, vCurr, lerpFactorX);

				float pixelDepth = m_pDepthBuffer[j + i * m_BackBufferWidth];
				if (vCurr.svPostion.z <= pixelDepth)
				{
#ifndef RD_DEBUG_RASTERIZATION
					Vector4 color = m_pPS->PSMain(vCurr, &m_SRTexture);
					Vector4 argbColor = ConvertRGBAColorToARGBColor(color);
					m_pFrameBuffer[j + i * m_BackBufferWidth] = ConvertColorToUInt32(argbColor);
#else // defined RD_DEBUG_RASTERIZATION
					Vector4 color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
					Vector4 overDrawColor = Vector4(0.0f, 1.0f, 0.0f, 1.0f);

					uint32_t colorUint = ConvertColorToUInt32(ConvertRGBAColorToARGBColor(color));

					uint32_t drawColor = (m_pFrameBuffer[j + i * m_BackBufferWidth] == colorUint) ? 
										ConvertColorToUInt32(ConvertRGBAColorToARGBColor(overDrawColor)) : 
										colorUint;

					m_pFrameBuffer[j + i * m_BackBufferWidth] = drawColor;
#endif // RD_DEBUG_RASTERIZATION

					m_pDepthBuffer[j + i * m_BackBufferWidth] = vCurr.svPostion.z;
				}
			}
		}
	}

	void DeviceContext::SliceTriangleToUpAndBottom(const VSOutputVertex& v0, const VSOutputVertex& v1, const VSOutputVertex& v2, VSOutputVertex& vNew)
	{
		float lerpFactor = (v1.svPostion.y - v0.svPostion.y) / (v2.svPostion.y - v0.svPostion.y);

		LerpVertexParamsInScreen(v0, v2, vNew, lerpFactor);
	}

	void DeviceContext::LerpVertexParamsInScreen(const VSOutputVertex& vStart, const VSOutputVertex& vEnd, VSOutputVertex& vNew, float lerpFactor)
	{
		float newX = vStart.svPostion.x + (vEnd.svPostion.x - vStart.svPostion.x) * lerpFactor;
		float newY = vStart.svPostion.y + (vEnd.svPostion.y - vStart.svPostion.y) * lerpFactor;
		float newZ = vStart.svPostion.z + (vEnd.svPostion.z - vStart.svPostion.z) * lerpFactor;
		float newW = 1.0f / (1.0f / vStart.svPostion.w + (1.0f / vEnd.svPostion.w - 1.0f / vStart.svPostion.w) * lerpFactor);

		vNew.svPostion	= Vector4(newX, newY, newZ, newW);
		vNew.color		= newW * ((vStart.color / vStart.svPostion.w) * (1.0f - lerpFactor) + (vEnd.color / vEnd.svPostion.w) * lerpFactor);
		vNew.normal		= newW * ((vStart.normal / vStart.svPostion.w) * (1.0f - lerpFactor) + (vEnd.normal / vEnd.svPostion.w) * lerpFactor);
		vNew.tangent	= newW * ((vStart.tangent / vStart.svPostion.w) * (1.0f - lerpFactor) + (vEnd.tangent / vEnd.svPostion.w) * lerpFactor);
		vNew.texcoord	= newW * ((vStart.texcoord / vStart.svPostion.w) * (1.0f - lerpFactor) + (vEnd.texcoord / vEnd.svPostion.w) * lerpFactor);
	}

	void DeviceContext::LerpVertexParamsInClip(const VSOutputVertex& vStart, const VSOutputVertex& vEnd, VSOutputVertex& vNew, float lerpFactor)
	{
		float newX = vStart.svPostion.x + (vEnd.svPostion.x - vStart.svPostion.x) * lerpFactor;
		float newY = vStart.svPostion.y + (vEnd.svPostion.y - vStart.svPostion.y) * lerpFactor;
		float newZ = vStart.svPostion.z + (vEnd.svPostion.z - vStart.svPostion.z) * lerpFactor;
		float newW = vStart.svPostion.w + (vEnd.svPostion.w - vStart.svPostion.w) * lerpFactor;

		vNew.svPostion	= Vector4(newX, newY, newZ, newW);
		vNew.color		= vStart.color * (1.0f - lerpFactor) + vEnd.color * lerpFactor;
		vNew.normal		= vStart.normal * (1.0f - lerpFactor) + vEnd.normal * lerpFactor;
		vNew.tangent	= vStart.tangent * (1.0f - lerpFactor) + vEnd.tangent * lerpFactor;
		vNew.texcoord	= vStart.texcoord * (1.0f - lerpFactor) + vEnd.texcoord * lerpFactor;
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
			m_ClipOutputVerts[i].svPostion.x /= m_ClipOutputVerts[i].svPostion.w;
			m_ClipOutputVerts[i].svPostion.y /= m_ClipOutputVerts[i].svPostion.w;
			m_ClipOutputVerts[i].svPostion.z /= m_ClipOutputVerts[i].svPostion.w;
		}
	}

	void DeviceContext::ViewportTransform()
	{
		for (uint32_t i = 0; i < m_ClipOutputVerts.size(); ++i)
		{
			VSOutputVertex& vsOutput = m_ClipOutputVerts[i];
			Vector4 vScreenPos(vsOutput.svPostion.x, vsOutput.svPostion.y, vsOutput.svPostion.z, 1.0f);
			vScreenPos = vScreenPos * m_ViewportMatrix;
			vsOutput.svPostion.x = vScreenPos.x;
			vsOutput.svPostion.y = vScreenPos.y;
			vsOutput.svPostion.z = vScreenPos.z;
		}
	}

	void DeviceContext::ClipTriangleWithPlaneX(int sign)
	{
		std::vector<VSOutputVertex> currClipResultVerts = {};

		for (uint32_t i = 0; i < m_ClippingVerts.size(); i += 3)
		{
			int outOfClipPlaneNum = 0;

			VSOutputVertex& vert0 = m_ClippingVerts[i];
			VSOutputVertex& vert1 = m_ClippingVerts[i + 1];
			VSOutputVertex& vert2 = m_ClippingVerts[i + 2];

			sign* vert0.svPostion.x > vert0.svPostion.w ? ++outOfClipPlaneNum : outOfClipPlaneNum;
			sign* vert1.svPostion.x > vert1.svPostion.w ? ++outOfClipPlaneNum : outOfClipPlaneNum;
			sign* vert2.svPostion.x > vert2.svPostion.w ? ++outOfClipPlaneNum : outOfClipPlaneNum;

			if (outOfClipPlaneNum == 0)
			{
				currClipResultVerts.push_back(vert0);
				currClipResultVerts.push_back(vert1);
				currClipResultVerts.push_back(vert2);

				continue;
			}
			else if (outOfClipPlaneNum == 3)
			{
				continue;
			}
			else if (outOfClipPlaneNum == 2)
			{
				if (sign * vert0.svPostion.x < vert0.svPostion.w)
				{
					float lerpFactor1 = GetClipLerpFactorX(vert0, vert1, sign);
					float lerpFactor2 = GetClipLerpFactorX(vert0, vert2, sign);
					ClipTwoVertsInTriangle(vert0, vert1, vert2, lerpFactor1, lerpFactor2);
				}
				else if (sign * vert1.svPostion.x < vert1.svPostion.w)
				{
					float lerpFactor1 = GetClipLerpFactorX(vert1, vert2, sign);
					float lerpFactor2 = GetClipLerpFactorX(vert1, vert0, sign);
					ClipTwoVertsInTriangle(vert1, vert2, vert0, lerpFactor1, lerpFactor2);
				}
				else
				{
					float lerpFactor1 = GetClipLerpFactorX(vert2, vert0, sign);
					float lerpFactor2 = GetClipLerpFactorX(vert2, vert1, sign);
					ClipTwoVertsInTriangle(vert2, vert0, vert1, lerpFactor1, lerpFactor2);
				}

				currClipResultVerts.push_back(vert0);
				currClipResultVerts.push_back(vert1);
				currClipResultVerts.push_back(vert2);

				continue;
			}
			else
			{
				if (sign * vert0.svPostion.x > vert0.svPostion.w)
				{
					float lerpFactor1 = GetClipLerpFactorX(vert1, vert0, sign);
					float lerpFactor2 = GetClipLerpFactorX(vert2, vert0, sign);
					ClipOneVertInTriangle(vert0, vert1, vert2, lerpFactor1, lerpFactor2, currClipResultVerts);
				}
				else if (sign * vert1.svPostion.x > vert1.svPostion.w)
				{
					float lerpFactor1 = GetClipLerpFactorX(vert2, vert1, sign);
					float lerpFactor2 = GetClipLerpFactorX(vert0, vert1, sign);
					ClipOneVertInTriangle(vert1, vert2, vert0, lerpFactor1, lerpFactor2, currClipResultVerts);
				}
				else
				{
					float lerpFactor1 = GetClipLerpFactorX(vert0, vert2, sign);
					float lerpFactor2 = GetClipLerpFactorX(vert1, vert2, sign);
					ClipOneVertInTriangle(vert2, vert0, vert1, lerpFactor1, lerpFactor2, currClipResultVerts);
				}

				currClipResultVerts.push_back(vert0);
				currClipResultVerts.push_back(vert1);
				currClipResultVerts.push_back(vert2);

				continue;
			}
		}

		m_ClippingVerts.clear();
		for (uint32_t i = 0; i < currClipResultVerts.size(); ++i)
		{
			m_ClippingVerts.push_back(currClipResultVerts[i]);
		}
	}

	void DeviceContext::ClipTriangleWithPlaneY(int sign)
	{
		std::vector<VSOutputVertex> currClipResultVerts = {};

		for (uint32_t i = 0; i < m_ClippingVerts.size(); i += 3)
		{
			int outOfClipPlaneNum = 0;

			VSOutputVertex& vert0 = m_ClippingVerts[i];
			VSOutputVertex& vert1 = m_ClippingVerts[i + 1];
			VSOutputVertex& vert2 = m_ClippingVerts[i + 2];

			sign * vert0.svPostion.y > vert0.svPostion.w ? ++outOfClipPlaneNum : outOfClipPlaneNum;
			sign * vert1.svPostion.y > vert1.svPostion.w ? ++outOfClipPlaneNum : outOfClipPlaneNum;
			sign * vert2.svPostion.y > vert2.svPostion.w ? ++outOfClipPlaneNum : outOfClipPlaneNum;

			if (outOfClipPlaneNum == 0)
			{
				currClipResultVerts.push_back(vert0);
				currClipResultVerts.push_back(vert1);
				currClipResultVerts.push_back(vert2);

				continue;
			}
			else if (outOfClipPlaneNum == 3)
			{
				continue;
			}
			else if (outOfClipPlaneNum == 2)
			{
				if (sign * vert0.svPostion.y < vert0.svPostion.w)
				{
					float lerpFactor1 = GetClipLerpFactorY(vert0, vert1, sign);
					float lerpFactor2 = GetClipLerpFactorY(vert0, vert2, sign);
					ClipTwoVertsInTriangle(vert0, vert1, vert2, lerpFactor1, lerpFactor2);
				}
				else if (sign * vert1.svPostion.y < vert1.svPostion.w)
				{
					float lerpFactor1 = GetClipLerpFactorY(vert1, vert2, sign);
					float lerpFactor2 = GetClipLerpFactorY(vert1, vert0, sign);
					ClipTwoVertsInTriangle(vert1, vert2, vert0, lerpFactor1, lerpFactor2);
				}
				else
				{
					float lerpFactor1 = GetClipLerpFactorY(vert2, vert0, sign);
					float lerpFactor2 = GetClipLerpFactorY(vert2, vert1, sign);
					ClipTwoVertsInTriangle(vert2, vert0, vert1, lerpFactor1, lerpFactor2);
				}

				currClipResultVerts.push_back(vert0);
				currClipResultVerts.push_back(vert1);
				currClipResultVerts.push_back(vert2);

				continue;
			}
			else
			{
				if (sign * vert0.svPostion.y > vert0.svPostion.w)
				{
					float lerpFactor1 = GetClipLerpFactorY(vert1, vert0, sign);
					float lerpFactor2 = GetClipLerpFactorY(vert2, vert0, sign);
					ClipOneVertInTriangle(vert0, vert1, vert2, lerpFactor1, lerpFactor2, currClipResultVerts);
				}
				else if (sign * vert1.svPostion.y > vert1.svPostion.w)
				{
					float lerpFactor1 = GetClipLerpFactorY(vert2, vert1, sign);
					float lerpFactor2 = GetClipLerpFactorY(vert0, vert1, sign);
					ClipOneVertInTriangle(vert1, vert2, vert0, lerpFactor1, lerpFactor2, currClipResultVerts);
				}
				else
				{
					float lerpFactor1 = GetClipLerpFactorY(vert0, vert2, sign);
					float lerpFactor2 = GetClipLerpFactorY(vert1, vert2, sign);
					ClipOneVertInTriangle(vert2, vert0, vert1, lerpFactor1, lerpFactor2, currClipResultVerts);
				}

				currClipResultVerts.push_back(vert0);
				currClipResultVerts.push_back(vert1);
				currClipResultVerts.push_back(vert2);

				continue;
			}
		}

		m_ClippingVerts.clear();
		for (uint32_t i = 0; i < currClipResultVerts.size(); ++i)
		{
			m_ClippingVerts.push_back(currClipResultVerts[i]);
		}
	}

	void DeviceContext::ClipTriangleWithPlaneZeroZ()
	{
		std::vector<VSOutputVertex> currClipResultVerts = {};

		for (uint32_t i = 0; i < m_ClippingVerts.size(); i += 3)
		{
			int outOfClipPlaneNum = 0;

			VSOutputVertex& vert0 = m_ClippingVerts[i];
			VSOutputVertex& vert1 = m_ClippingVerts[i + 1];
			VSOutputVertex& vert2 = m_ClippingVerts[i + 2];

			vert0.svPostion.z < 0.0f ? ++outOfClipPlaneNum : outOfClipPlaneNum;
			vert1.svPostion.z < 0.0f ? ++outOfClipPlaneNum : outOfClipPlaneNum;
			vert2.svPostion.z < 0.0f ? ++outOfClipPlaneNum : outOfClipPlaneNum;

			if (outOfClipPlaneNum == 0)
			{
				currClipResultVerts.push_back(vert0);
				currClipResultVerts.push_back(vert1);
				currClipResultVerts.push_back(vert2);

				continue;
			}
			else if (outOfClipPlaneNum == 3)
			{
				continue;
			}
			else if (outOfClipPlaneNum == 2)
			{
				if (vert0.svPostion.z > 0.0f)
				{
					float lerpFactor1 = GetClipLerpFactorZeroZ(vert0, vert1);
					float lerpFactor2 = GetClipLerpFactorZeroZ(vert0, vert2);
					ClipTwoVertsInTriangle(vert0, vert1, vert2, lerpFactor1, lerpFactor2);
				}
				else if (vert1.svPostion.z > 0.0f)
				{
					float lerpFactor1 = GetClipLerpFactorZeroZ(vert1, vert2);
					float lerpFactor2 = GetClipLerpFactorZeroZ(vert1, vert0);
					ClipTwoVertsInTriangle(vert1, vert2, vert0, lerpFactor1, lerpFactor2);
				}
				else
				{
					float lerpFactor1 = GetClipLerpFactorZeroZ(vert2, vert0);
					float lerpFactor2 = GetClipLerpFactorZeroZ(vert2, vert1);
					ClipTwoVertsInTriangle(vert2, vert0, vert1, lerpFactor1, lerpFactor2);
				}

				currClipResultVerts.push_back(vert0);
				currClipResultVerts.push_back(vert1);
				currClipResultVerts.push_back(vert2);

				continue;
			}
			else
			{
				if (vert0.svPostion.z < 0.0f)
				{
					float lerpFactor1 = GetClipLerpFactorZeroZ(vert1, vert0);
					float lerpFactor2 = GetClipLerpFactorZeroZ(vert2, vert0);
					ClipOneVertInTriangle(vert0, vert1, vert2, lerpFactor1, lerpFactor2, currClipResultVerts);
				}
				else if (vert1.svPostion.z < 0.0f)
				{
					float lerpFactor1 = GetClipLerpFactorZeroZ(vert2, vert1);
					float lerpFactor2 = GetClipLerpFactorZeroZ(vert0, vert1);
					ClipOneVertInTriangle(vert1, vert2, vert0, lerpFactor1, lerpFactor2, currClipResultVerts);
				}
				else
				{
					float lerpFactor1 = GetClipLerpFactorZeroZ(vert0, vert2);
					float lerpFactor2 = GetClipLerpFactorZeroZ(vert1, vert2);
					ClipOneVertInTriangle(vert2, vert0, vert1, lerpFactor1, lerpFactor2, currClipResultVerts);
				}

				currClipResultVerts.push_back(vert0);
				currClipResultVerts.push_back(vert1);
				currClipResultVerts.push_back(vert2);

				continue;
			}
		}

		m_ClippingVerts.clear();
		for (uint32_t i = 0; i < currClipResultVerts.size(); ++i)
		{
			m_ClippingVerts.push_back(currClipResultVerts[i]);
		}
	}

	void DeviceContext::ClipTriangleWithPlanePositiveZ()
	{
		std::vector<VSOutputVertex> currClipResultVerts = {};

		for (uint32_t i = 0; i < m_ClippingVerts.size(); i += 3)
		{
			int outOfClipPlaneNum = 0;

			VSOutputVertex& vert0 = m_ClippingVerts[i];
			VSOutputVertex& vert1 = m_ClippingVerts[i + 1];
			VSOutputVertex& vert2 = m_ClippingVerts[i + 2];

			vert0.svPostion.z > vert0.svPostion.w ? ++outOfClipPlaneNum : outOfClipPlaneNum;
			vert1.svPostion.z > vert1.svPostion.w ? ++outOfClipPlaneNum : outOfClipPlaneNum;
			vert2.svPostion.z > vert2.svPostion.w ? ++outOfClipPlaneNum : outOfClipPlaneNum;

			if (outOfClipPlaneNum == 0)
			{
				currClipResultVerts.push_back(vert0);
				currClipResultVerts.push_back(vert1);
				currClipResultVerts.push_back(vert2);

				continue;
			}
			else if (outOfClipPlaneNum == 3)
			{
				continue;
			}
			else if (outOfClipPlaneNum == 2)
			{
				if (vert0.svPostion.z < vert0.svPostion.w)
				{
					float lerpFactor1 = GetClipLerpFactorPositiveZ(vert0, vert1);
					float lerpFactor2 = GetClipLerpFactorPositiveZ(vert0, vert2);
					ClipTwoVertsInTriangle(vert0, vert1, vert2, lerpFactor1, lerpFactor2);
				}
				else if (vert1.svPostion.z < vert1.svPostion.w)
				{
					float lerpFactor1 = GetClipLerpFactorPositiveZ(vert1, vert2);
					float lerpFactor2 = GetClipLerpFactorPositiveZ(vert1, vert0);
					ClipTwoVertsInTriangle(vert1, vert2, vert0, lerpFactor1, lerpFactor2);
				}
				else
				{
					float lerpFactor1 = GetClipLerpFactorPositiveZ(vert2, vert0);
					float lerpFactor2 = GetClipLerpFactorPositiveZ(vert2, vert1);
					ClipTwoVertsInTriangle(vert2, vert0, vert1, lerpFactor1, lerpFactor2);
				}

				currClipResultVerts.push_back(vert0);
				currClipResultVerts.push_back(vert1);
				currClipResultVerts.push_back(vert2);

				continue;
			}
			else
			{
				if (vert0.svPostion.z > vert0.svPostion.w)
				{
					float lerpFactor1 = GetClipLerpFactorPositiveZ(vert1, vert0);
					float lerpFactor2 = GetClipLerpFactorPositiveZ(vert2, vert0);
					ClipOneVertInTriangle(vert0, vert1, vert2, lerpFactor1, lerpFactor2, currClipResultVerts);
				}
				else if (vert1.svPostion.z > vert1.svPostion.w)
				{
					float lerpFactor1 = GetClipLerpFactorPositiveZ(vert2, vert1);
					float lerpFactor2 = GetClipLerpFactorPositiveZ(vert0, vert1);
					ClipOneVertInTriangle(vert1, vert2, vert0, lerpFactor1, lerpFactor2, currClipResultVerts);
				}
				else
				{
					float lerpFactor1 = GetClipLerpFactorPositiveZ(vert0, vert2);
					float lerpFactor2 = GetClipLerpFactorPositiveZ(vert1, vert2);
					ClipOneVertInTriangle(vert2, vert0, vert1, lerpFactor1, lerpFactor2, currClipResultVerts);
				}

				currClipResultVerts.push_back(vert0);
				currClipResultVerts.push_back(vert1);
				currClipResultVerts.push_back(vert2);

				continue;
			}
		}

		m_ClippingVerts.clear();
		for (uint32_t i = 0; i < currClipResultVerts.size(); ++i)
		{
			m_ClippingVerts.push_back(currClipResultVerts[i]);
		}
	}

	void DeviceContext::ClipTwoVertsInTriangle(const VSOutputVertex& vertIn, VSOutputVertex& vertOut1, VSOutputVertex& vertOut2, float lerpFactor1, float lerpFactor2)
	{
		VSOutputVertex vertNew1;
		LerpVertexParamsInClip(vertIn, vertOut1, vertNew1, lerpFactor1);

		VSOutputVertex vertNew2;
		LerpVertexParamsInClip(vertIn, vertOut2, vertNew2, lerpFactor2);

		vertOut1 = vertNew1;
		vertOut2 = vertNew2;
	}

	void DeviceContext::ClipOneVertInTriangle(VSOutputVertex& vertOut, const VSOutputVertex& vertIn1, const VSOutputVertex& vertIn2, float lerpFactor1, float lerpFactor2, std::vector<VSOutputVertex>& tempVerts)
	{
		VSOutputVertex vertNew1;
		LerpVertexParamsInClip(vertIn1, vertOut, vertNew1, lerpFactor1);

		VSOutputVertex vertNew2;
		LerpVertexParamsInClip(vertIn2, vertOut, vertNew2, lerpFactor2);

		vertOut = vertNew2;
		tempVerts.push_back(vertNew2);
		tempVerts.push_back(vertNew1);
		tempVerts.push_back(vertIn1);
	}

	float DeviceContext::GetClipLerpFactorX(const VSOutputVertex& vert0, const VSOutputVertex& vert1, int sign)
	{
		return (vert0.svPostion.x - sign * vert0.svPostion.w) / (sign * vert1.svPostion.w - sign * vert0.svPostion.w - vert1.svPostion.x + vert0.svPostion.x);
	}

	float DeviceContext::GetClipLerpFactorY(const VSOutputVertex& vert0, const VSOutputVertex& vert1, int sign)
	{
		return (vert0.svPostion.y - sign * vert0.svPostion.w) / (sign * vert1.svPostion.w - sign * vert0.svPostion.w - vert1.svPostion.y + vert0.svPostion.y);
	}

	float DeviceContext::GetClipLerpFactorZeroZ(const VSOutputVertex& vert0, const VSOutputVertex& vert1)
	{
		return (-vert0.svPostion.z / (vert1.svPostion.z - vert0.svPostion.z));
	}

	float DeviceContext::GetClipLerpFactorPositiveZ(const VSOutputVertex& vert0, const VSOutputVertex& vert1)
	{
		return (vert0.svPostion.z - vert0.svPostion.w) / (vert1.svPostion.w - vert0.svPostion.w - vert1.svPostion.z + vert0.svPostion.z);
	}

	void DeviceContext::ShapeAssemble(uint32_t indexNum)
	{
		m_AssembledVerts.clear();
		if (m_AssembledVerts.capacity() < indexNum)
		{
			m_AssembledVerts.reserve(indexNum);
		}

		if (m_PriTopology == SR_PRIMITIVE_TOPOLOGY::LINE_LIST || m_PriTopology == SR_PRIMITIVE_TOPOLOGY::TRIANGLE_LIST)
		{
			const uint32_t* pIndice = m_pIB->GetData();
			for (uint32_t i = 0; i < indexNum; ++i)
			{
				uint32_t index = pIndice[i];
				const VSOutputVertex& vert0 = m_VSOutputs[index];

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
			const Vector4& pos1 = m_AssembledVerts[i].svPostion;
			const Vector4& pos2 = m_AssembledVerts[i + 1].svPostion;
			const Vector4& pos3 = m_AssembledVerts[i + 2].svPostion;

			//三角形三个顶点若逆时针环绕，则行列式的值为正数，顺时针为负数，等于0则退化为线段
			if (GetArea2(Vector3(pos1.x, pos1.y, pos1.z), Vector3(pos2.x, pos2.y, pos2.z), Vector3(pos3.x, pos3.y, pos3.z)) < 0.0f)
			{
				m_BackFaceCulledVerts.push_back(m_AssembledVerts[i]);
				m_BackFaceCulledVerts.push_back(m_AssembledVerts[i + 1]);
				m_BackFaceCulledVerts.push_back(m_AssembledVerts[i + 2]);
			}
		}
	}

	//这里规定像素的采样点在像素中心点，即对于左上角的第一个像素来说，采样点为(0.5, 0.5)
	void DeviceContext::Rasterization()
	{
		for (uint32_t i = 0; i < m_ClipOutputVerts.size(); i += 3)
		{
			const VSOutputVertex& vert0 = m_ClipOutputVerts[i];
			const VSOutputVertex& vert1 = m_ClipOutputVerts[i + 1];
			const VSOutputVertex& vert2 = m_ClipOutputVerts[i + 2];

			if (m_PriTopology == SR_PRIMITIVE_TOPOLOGY::LINE_LIST)
			{
				DrawTriangleWithLine(vert0, vert1, vert2);
			}
			else if (m_PriTopology == SR_PRIMITIVE_TOPOLOGY::TRIANGLE_LIST)
			{
				DrawTriangleWithFlat(vert0, vert1, vert2);
			}
		}
	}

#pragma endregion DeviceContext

#pragma region SwapChain
	class SwapChain : public ISRSwapChain
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
		virtual bool ResizeBuffers(uint32_t bufferCnts, uint32_t width, uint32_t height, SR_FORMAT format) override;

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

		SRTexture2DDesc texDesc;
		texDesc.width = m_Desc.width;
		texDesc.height = m_Desc.height;
		texDesc.format = SR_FORMAT::UNKNOWN;   //这里不传入pDesc->format是为了Init Texture2D时避免分配内存，SwapChain的backbuffer的内存由CreateDIBSection来分配；
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
		case SR_FORMAT::R8G8B8A8_UNORM:
		{
			bitCnt = 32;
			imageSize = pDesc->width * pDesc->height * 4;

			m_pBackBuffer->SetFormat(SR_FORMAT::R8G8B8A8_UNORM);

			break;
		}
		case SR_FORMAT::UNKNOWN:
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

		BITMAPINFO bitMapInfo =
		{
			{ sizeof(BITMAPINFOHEADER), (int)pDesc->width, -(int)pDesc->height, 1, bitCnt, BI_RGB, imageSize, 0, 0, 0, 0 }
		};
		m_hBitMap = CreateDIBSection(m_hWndDC, &bitMapInfo, DIB_RGB_COLORS, &pTempBitMapBuffer, 0, 0);
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

	bool SwapChain::ResizeBuffers(uint32_t bufferCnts, uint32_t width, uint32_t height, SR_FORMAT format)
	{
		if (m_hOldBitMap)
		{
			SelectObject(m_hWndDC, m_hOldBitMap);
			m_hOldBitMap = nullptr;
		}

		if (m_hBitMap)
		{
			DeleteObject(m_hBitMap);
			m_hBitMap = nullptr;
			m_pBackBuffer->GetData() = nullptr;
		}

		if (m_pBackBuffer)
		{
			m_pBackBuffer->Release();
			m_pBackBuffer = nullptr;
		}

		m_Desc.width = width;
		m_Desc.height = height;
		m_Desc.format = format;

		m_pBackBuffer = new Texture2D();
		if (!m_pBackBuffer)
		{
			return false;
		}

		SRTexture2DDesc texDesc;
		texDesc.width = m_Desc.width;
		texDesc.height = m_Desc.height;
		texDesc.format = SR_FORMAT::UNKNOWN;   //这里不传入pDesc->format是为了Init Texture2D时避免分配内存，SwapChain的backbuffer的内存由CreateDIBSection来分配；
		if (!m_pBackBuffer->Init(&texDesc, nullptr))
		{
			return false;
		}

		void*& pTempBitMapBuffer = m_pBackBuffer->GetData();

		uint16_t bitCnt = 0;
		uint32_t imageSize = 0;
		switch (format)
		{
		case SR_FORMAT::R8G8B8A8_UNORM:
		{
			bitCnt = 32;
			imageSize = width * height * 4;

			m_pBackBuffer->SetFormat(SR_FORMAT::R8G8B8A8_UNORM);

			break;
		}
		case SR_FORMAT::UNKNOWN:
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

		BITMAPINFO bitMapInfo =
		{
			{ sizeof(BITMAPINFOHEADER), (int)width, -(int)height, 1, bitCnt, BI_RGB, imageSize, 0, 0, 0, 0 }
		};
		m_hBitMap = CreateDIBSection(m_hWndDC, &bitMapInfo, DIB_RGB_COLORS, &pTempBitMapBuffer, 0, 0);
		if (m_hBitMap)
		{
			m_hOldBitMap = (HBITMAP)SelectObject(m_hWndDC, m_hBitMap);
		}
		else
		{
			m_hOldBitMap = nullptr;

			return false;
		}

		return true;
	}

	void SwapChain::GetDesc(SwapChainDesc* pDesc)
	{
		pDesc = &m_Desc;
	}
#pragma endregion SwapChain


	bool CreateDeviceAndSwapChain(ISRDevice** ppDevice, ISRDeviceContext** ppDeviceContext, ISRSwapChain** ppSwapChain, const SwapChainDesc* pSwapChainDesc)
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
	VSOutputVertex VertexShader::VSMain(const LocalVertex& inVertex, const Matrix4x4& matWorld, const Matrix4x4& matView, const Matrix4x4& matProj) const
	{
		VSOutputVertex vOutput = {};
		Vector4 inPos = Vector4(inVertex.position, 1.0f);
		inPos = inPos * matWorld;
		inPos = inPos * matView;
		inPos = inPos * matProj;

		vOutput.svPostion.x = inPos.x;
		vOutput.svPostion.y = inPos.y;
		vOutput.svPostion.z = inPos.z;
		vOutput.svPostion.w = inPos.w;

		vOutput.color = Vector4(inVertex.color);

		Vector4 vNormal = Vector4(inVertex.normal, 0.0f);
		vNormal = vNormal * matWorld;
		vOutput.normal.x = vNormal.x;
		vOutput.normal.y = vNormal.y;
		vOutput.normal.z = vNormal.z;

		Vector4 vTangent = Vector4(inVertex.tangent.x, inVertex.tangent.y, inVertex.tangent.z, 0.0f);
		vTangent = vTangent * matWorld;
		vOutput.tangent.x = vTangent.x;
		vOutput.tangent.y = vTangent.y;
		vOutput.tangent.z = vTangent.z;
		vOutput.tangent.w = inVertex.tangent.w;

		vOutput.texcoord = inVertex.texcoord;

		return vOutput;
	}

	Vector4 PixelShader::PSMain(const VSOutputVertex& psInput, const ShaderResourceTexture* pSRTexture) const
	{
		Vector2 uv = psInput.texcoord;

		Vector4 textureColor = m_pSampler->Sample(pSRTexture, uv);

		float tangentNormalX = textureColor.x;
		float tangentNormalY = textureColor.y;
		float tangentNormalZ = textureColor.z;
		tangentNormalX = tangentNormalX * 2.0f - 1.0f;
		tangentNormalY = tangentNormalY * 2.0f - 1.0f;
		tangentNormalZ = tangentNormalZ * 2.0f - 1.0f;

		Vector3 tangentNormal = Normalize(Vector3(tangentNormalX, tangentNormalY, tangentNormalZ));

		Vector3 tangent = Normalize(Vector3(psInput.tangent.x, psInput.tangent.y, psInput.tangent.z));
		Vector3 biTangent = Normalize(CrossProduct(psInput.normal, tangent) * psInput.tangent.w);
		Vector3 vertexNormal = Normalize(psInput.normal);

		Vector4 T = Vector4(tangent, 0.0f);
		Vector4 B = Vector4(biTangent, 0.0f);
		Vector4 N = Vector4(vertexNormal, 0.0f);

		Matrix4x4 matTBN(T, B, N, Vector4(0, 0, 0, 1));

		Vector4 worldNormal = Vector4(tangentNormal, 0.0f) * matTBN;

		Vector3 diffuseColor = CalcPhongLighing(m_pMainLight, Vector3(worldNormal.x, worldNormal.y, worldNormal.z), Vector3(1.0f, 1.0f, 1.0f));

		Vector3 AmbientColor = Vector3(0.05f, 0.05f, 0.05f);
		Vector3 finalColor = diffuseColor + AmbientColor;

		finalColor.x = Clamp(finalColor.x, 0.0f, 1.0f);
		finalColor.y = Clamp(finalColor.y, 0.0f, 1.0f);
		finalColor.z = Clamp(finalColor.z, 0.0f, 1.0f);

		return Vector4(finalColor, 1.0f);
	}

	Vector3 PixelShader::CalcPhongLighing(const MainLight* light, const Vector3& normal, const Vector3& faceColor) const
	{
		Vector3 worldLightDir = -Normalize(light->direction);
		worldLightDir = Normalize(worldLightDir);
		Vector3 worldNormal = Normalize(normal);

		float diffuseFactor = DotProduct(worldLightDir, worldNormal);

		diffuseFactor = Clamp(diffuseFactor, 0.0f, 1.0f);

		Vector3 lightColor = Vector3(light->color.x, light->color.y, light->color.z);

		return diffuseFactor * lightColor * light->luminance * faceColor;
	}
#pragma endregion Shader
}