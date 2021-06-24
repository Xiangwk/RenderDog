#include "Light.h"

namespace RenderDog
{
	DirectionalLight::DirectionalLight(const DirLightDesc& desc):
		m_Color(desc.color),
		m_fLuminance(desc.luminance),
		m_fPhi(desc.fPhi),
		m_fTheta(desc.fTheta)
	{
		float fX = cosf(m_fPhi) * cosf(m_fTheta);
		float fY = cosf(m_fPhi);
		float fZ = cosf(m_fPhi) * sinf(m_fTheta);

		m_Direction = Vector3(fX, fY, fZ);
	}

	void DirectionalLight::UpdateDirection(float fDeltaTheta, float fDeltaPhi)
	{
		m_fPhi += fDeltaPhi;
		m_fTheta += fDeltaTheta;

		float fX = cosf(m_fPhi) * cosf(m_fTheta);
		float fY = cosf(m_fPhi);
		float fZ = cosf(m_fPhi) * sinf(m_fTheta);

		m_Direction = Vector3(fX, fY, fZ);
	}
}