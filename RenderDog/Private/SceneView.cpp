////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SceneView.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "SceneView.h"
#include "Camera.h"
#include "Primitive.h"

namespace RenderDog
{
	SceneView::SceneView():
		m_OpaquePris(0),
		m_SimplePris(0),
		m_Lights(0),
		m_pCamera(nullptr),
		m_WorldToViewMatrix(),
		m_ViewToClipMatrix(),
		m_ShadowWorldToViewMatrix(),
		m_ShadowViewToClipMatrix(),
		m_pRenderData(nullptr)
	{
		m_WorldToViewMatrix.Identity();
		m_ViewToClipMatrix.Identity();
		m_ShadowWorldToViewMatrix.Identity();
		m_ShadowViewToClipMatrix.Identity();

		m_pRenderData = new SceneViewRenderData();

		InitRenderData();
	}

	SceneView::SceneView(FPSCamera* pCamera) :
		m_OpaquePris(0),
		m_SimplePris(0),
		m_Lights(0),
		m_pCamera(pCamera),
		m_pRenderData(nullptr)
	{
		m_WorldToViewMatrix.Identity();
		m_ViewToClipMatrix.Identity();
		m_ShadowWorldToViewMatrix.Identity();
		m_ShadowViewToClipMatrix.Identity();

		m_pRenderData = new SceneViewRenderData();

		InitRenderData();
	}

	SceneView::~SceneView()
	{
		m_OpaquePris.clear();
		m_SimplePris.clear();

		m_Lights.clear();

		ReleaseRenderData();

		if (m_pRenderData)
		{
			delete m_pRenderData;
			m_pRenderData = nullptr;
		}
	}

	void SceneView::AddPrimitive(IPrimitive* pPri)
	{
		switch (pPri->GetPriType())
		{
		case PRIMITIVE_TYPE::SIMPLE_PRI:
		{
			m_SimplePris.push_back(pPri);
			break;
		}
		case PRIMITIVE_TYPE::STATIC_PRI:
		case PRIMITIVE_TYPE::SKIN_PRI:
		{
			m_OpaquePris.push_back(pPri);
			break;
		}
		default:
			break;
		}
	}

	IPrimitive* SceneView::GetOpaquePri(uint32_t index)
	{
		return m_OpaquePris[index];
	}

	IPrimitive* SceneView::GetSimplePri(uint32_t index)
	{
		return m_SimplePris[index];
	}

	void SceneView::AddLight(ILight* pLight)
	{
		m_Lights.push_back(pLight);
	}

	ILight* SceneView::GetLight(uint32_t index)
	{
		return m_Lights[index];
	}

	void SceneView::ClearPrimitives() 
	{ 
		m_OpaquePris.clear();
		m_SimplePris.clear();
	}

	void SceneView::ClearLights() 
	{ 
		m_Lights.clear();
	}

	void SceneView::UpdateRenderData()
	{
		ViewParamConstantData viewParamData = {};
		viewParamData.worldToViewMatrix = m_pCamera->GetViewMatrix();
		viewParamData.viewToClipMatrix = m_pCamera->GetPerspProjectionMatrix();
		viewParamData.mainCameraWorldPos = Vector4(m_pCamera->GetPosition(), 1.0f);
		m_pRenderData->pViewParamCB->Update(&viewParamData, sizeof(ViewParamConstantData));
	}

	void SceneView::InitRenderData()
	{
		if (!m_pRenderData)
		{
			return;
		}

		BufferDesc cbDesc = {};
		cbDesc.name = "ComVar_ConstantBuffer_ViewParam";
		cbDesc.byteWidth = sizeof(ViewParamConstantData);
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = true;
		m_pRenderData->pViewParamCB = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);

		cbDesc.name = "ComVar_ConstantBuffer_LightingParam";
		cbDesc.byteWidth = sizeof(DirectionalLightData);
		cbDesc.pInitData = nullptr;
		cbDesc.isDynamic = true;
		m_pRenderData->pDirLightParamCB = (IConstantBuffer*)g_pIBufferManager->GetConstantBuffer(cbDesc);
	}

	void SceneView::ReleaseRenderData()
	{
		if (!m_pRenderData)
		{
			return;
		}

		if (m_pRenderData->pViewParamCB)
		{
			m_pRenderData->pViewParamCB->Release();
			m_pRenderData->pViewParamCB = nullptr;
		}

		if (m_pRenderData->pDirLightParamCB)
		{
			m_pRenderData->pDirLightParamCB->Release();
			m_pRenderData->pDirLightParamCB = nullptr;
		}
	}

}// namespace RenderDog