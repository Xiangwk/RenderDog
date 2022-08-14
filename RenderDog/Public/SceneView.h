////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: SceneView.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "Matrix.h"

#include <vector>

namespace RenderDog
{
	class IPrimitive;
	class ILight;
	class FPSCamera;
	class IConstantBuffer;

	class SceneView
	{
	private:
		struct SceneViewRenderData
		{
			IConstantBuffer*		pViewParamCB;
			IConstantBuffer*		pDirLightParamCB;

			SceneViewRenderData() :
				pViewParamCB(nullptr),
				pDirLightParamCB(nullptr)
			{}
		};

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

		void						SetWorldToViewMatrix(const Matrix4x4& mat) { m_WorldToViewMatrix = mat; }
		const Matrix4x4&			GetWorldToViewMatrix() const { return m_WorldToViewMatrix; }

		void						SetViewToClipMatrix(const Matrix4x4& mat) { m_ViewToClipMatrix = mat; }
		const Matrix4x4&			GetViewToClipMatrix() const { return m_ViewToClipMatrix; }

		void						SetShadowWorldToViewMatrix(const Matrix4x4& mat) { m_ShadowWorldToViewMatrix = mat; }
		const Matrix4x4&			GetShadowWorldToViewMatrix() const { return m_ShadowWorldToViewMatrix; }

		void						SetShadowViewToClipMatrix(const Matrix4x4& mat) { m_ShadowViewToClipMatrix = mat; }
		const Matrix4x4&			GetShadowViewToClipMatrix() const { return m_ShadowViewToClipMatrix; }

		void						ClearPrimitives();
		void						ClearLights();

		void						UpdateRenderData();

		IConstantBuffer*			GetViewParamConstantBuffer() const { return m_pRenderData->pViewParamCB; }

	private:
		void						InitRenderData();
		void						ReleaseRenderData();

	private:
		std::vector<IPrimitive*>	m_OpaquePris;
		std::vector<IPrimitive*>	m_SimplePris;
		std::vector<ILight*>		m_Lights;
		FPSCamera*					m_pCamera;

		Matrix4x4					m_WorldToViewMatrix;
		Matrix4x4					m_ViewToClipMatrix;

		Matrix4x4					m_ShadowWorldToViewMatrix;
		Matrix4x4					m_ShadowViewToClipMatrix;

		SceneViewRenderData*		m_pRenderData;
	};

}// namespace RenderDog
