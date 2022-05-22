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
		m_pCamera(nullptr)
	{}

	SceneView::SceneView(FPSCamera* pCamera) :
		m_OpaquePris(0),
		m_SimplePris(0),
		m_Lights(0),
		m_pCamera(pCamera)
	{}

	SceneView::~SceneView()
	{
		m_OpaquePris.clear();
		m_SimplePris.clear();

		m_Lights.clear();
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
		case PRIMITIVE_TYPE::OPAQUE_PRI:
			m_OpaquePris.push_back(pPri);
			break;
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

}// namespace RenderDog