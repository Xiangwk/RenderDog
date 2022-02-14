////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SceneView.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "SceneView.h"
#include "Camera.h"

namespace RenderDog
{
	SceneView::SceneView():
		m_Primitives(0),
		m_Lights(0),
		m_pCamera(nullptr)
	{}

	SceneView::SceneView(FPSCamera* pCamera) :
		m_Primitives(0),
		m_Lights(0),
		m_pCamera(pCamera)
	{}

	SceneView::~SceneView()
	{
		m_Primitives.clear();

		m_Lights.clear();
	}

	void SceneView::AddPrimitive(IPrimitive* pPri)
	{
		m_Primitives.push_back(pPri);
	}

	IPrimitive* SceneView::GetPrimitive(uint32_t index)
	{
		return m_Primitives[index];
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
		m_Primitives.clear(); 
	}

	void SceneView::ClearLights() 
	{ 
		m_Lights.clear();
	}

}// namespace RenderDog