///////////////////////////////////
//RenderDog <·,·>
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
		float	pitch;
		float	yaw;
	};

	class DirectionalLight
	{
	public:
		DirectionalLight(const DirLightDesc& desc);
		~DirectionalLight() {}

		void UpdateDirection(float deltaYaw, float deltaPitch);

		Vector3 GetDirection() const { return m_Direction; }
		Vector3 GetColor() const { return m_Color; }
		float GetLuminance() const { return m_Luminance; }

		void SetColor(const Vector3& color) { m_Color = color; }
		void SetLuminance(float luma) { m_Luminance = luma; }

	private:
		Vector3	m_Direction;	//从光源发射光线的方向
		Vector3	m_Color;
		float	m_Luminance;

		float	m_Pitch;		//俯仰角
		float	m_Yaw;			//偏航角
	};
}