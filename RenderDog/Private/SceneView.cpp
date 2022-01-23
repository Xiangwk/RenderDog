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

	SceneView::SceneView(uint32_t viewWidth, uint32_t viewHeight):
		m_Primitives(0),
		m_pCamera(nullptr)
	{
		CameraDesc cameraDesc = {};
		cameraDesc.position = Vector3(0.0f, 0.0f, -5.0f);
		cameraDesc.direction = Vector3(0.0f, 0.0f, 1.0f);
		cameraDesc.fov = 45.0f;
		cameraDesc.aspectRitio = (float)viewWidth / (float)viewHeight;
		cameraDesc.nearPlane = 0.1f;
		cameraDesc.farPlane = 1000.0f;
		m_pCamera = new FPSCamera(cameraDesc);
	}


	SceneView::~SceneView()
	{
		m_Primitives.clear();

		m_Lights.clear();

		if (m_pCamera)
		{
			delete m_pCamera;
			m_pCamera = nullptr;
		}
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