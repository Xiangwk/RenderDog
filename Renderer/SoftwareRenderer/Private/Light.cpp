#include "Light.h"

namespace RenderDog
{
	DirectionalLight::DirectionalLight(const DirLightDesc& desc):
		m_Color(desc.color),
		m_Luminance(desc.luminance),
		m_Pitch(desc.pitch),
		m_Yaw(desc.yaw)
	{
		float fX = cosf(m_Pitch) * cosf(m_Yaw);
		float fY = sinf(m_Pitch);
		float fZ = cosf(m_Pitch) * sinf(m_Yaw);

		m_Direction = Vector3(fX, fY, fZ);
	}

	void DirectionalLight::UpdateDirection(float deltaYaw, float deltaPitch)
	{
		m_Pitch += deltaPitch;
		m_Yaw += deltaYaw;

		float fX = cosf(m_Pitch) * cosf(m_Yaw);
		float fY = cosf(m_Pitch);
		float fZ = cosf(m_Pitch) * sinf(m_Yaw);

		m_Direction = Vector3(fX, fY, fZ);
	}
}