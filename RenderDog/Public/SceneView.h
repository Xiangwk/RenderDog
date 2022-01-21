////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SceneView.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include <vector>

namespace RenderDog
{
	class IPrimitive;
	class FPSCamera;

	class SceneView
	{
	public:
		SceneView();
		SceneView(uint32_t viewWidth, uint32_t viewHeight);
		~SceneView();

		void						AddPrimitive(IPrimitive* pPri);
		IPrimitive*					GetPrimitive(uint32_t index);
		uint32_t					GetPrimitiveNum() const { return (uint32_t)m_Primitives.size(); }

		FPSCamera*					GetCamera() { return m_pCamera; }

		void						ClearPrimitives() { m_Primitives.clear(); }

	private:
		std::vector<IPrimitive*>	m_Primitives;
		FPSCamera*					m_pCamera;
	};

}// namespace RenderDog
