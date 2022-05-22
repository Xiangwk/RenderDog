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
	class ILight;
	class FPSCamera;

	class SceneView
	{
	public:
		SceneView();
		SceneView(FPSCamera* pCamera);
		~SceneView();

		void						AddPrimitive(IPrimitive* pPri);

		IPrimitive*					GetOpaquePri(uint32_t index);
		uint32_t					GetOpaquePrisNum() const { return (uint32_t)m_OpaquePris.size(); }

		IPrimitive*					GetSimplePri(uint32_t index);
		uint32_t					GetSimplePrisNum() const { return (uint32_t)m_SimplePris.size(); }

		void						AddLight(ILight* pLight);
		ILight*						GetLight(uint32_t index);
		uint32_t					GetLightNum() const { return (uint32_t)m_Lights.size(); }

		FPSCamera*					GetCamera() { return m_pCamera; }
		void						SetCamera(FPSCamera* pCamera) { m_pCamera = pCamera; }

		void						ClearPrimitives();
		void						ClearLights();

	private:
		std::vector<IPrimitive*>	m_OpaquePris;
		std::vector<IPrimitive*>	m_SimplePris;
		std::vector<ILight*>		m_Lights;
		FPSCamera*					m_pCamera;
	};

}// namespace RenderDog
