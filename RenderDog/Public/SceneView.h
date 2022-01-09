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

	class SceneView
	{
	public:
		SceneView();
		~SceneView();

		void		AddPrimitive(IPrimitive* pPri);
		IPrimitive* GetPrimitive(uint32_t index);
		uint32_t	GetPrimitiveNum() const { return (uint32_t)m_Primitives.size(); }

	private:
		std::vector<IPrimitive*>	m_Primitives;
	};

}// namespace RenderDog
