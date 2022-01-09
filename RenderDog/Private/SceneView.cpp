////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SceneView.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "SceneView.h"

namespace RenderDog
{
	SceneView::SceneView() :
		m_Primitives(0)
	{}

	SceneView::~SceneView()
	{}

	void SceneView::AddPrimitive(IPrimitive* pPri)
	{
		m_Primitives.push_back(pPri);
	}

	IPrimitive* SceneView::GetPrimitive(uint32_t index)
	{
		return m_Primitives[index];
	}

}// namespace RenderDog