///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Light.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include "Vector.h"

namespace RenderDog
{
	struct DirLightDesc
	{
		Vector3	color;
		float	luminance;
		float	fPhi;
		float	fTheta;
	};

	class DirectionalLight
	{
	public:
		DirectionalLight(const DirLightDesc& desc);
		~DirectionalLight() {}

		void UpdateDirection(float fDeltaTheta, float fDeltaPhi);

		Vector3 GetDirection() const { return m_Direction; }
		Vector3 GetColor() const { return m_Color; }
		float GetLuminance() const { return m_fLuminance; }

		void SetColor(const Vector3& color) { m_Color = color; }
		void SetLuminance(float fLuma) { m_fLuminance = fLuma; }

	private:
		Vector3	m_Direction;
		Vector3	m_Color;
		float	m_fLuminance;

		float	m_fPhi;			//¸©Ñö½Ç
		float	m_fTheta;		//Æ«º½½Ç
	};
}