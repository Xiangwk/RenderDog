////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Camera.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "Matrix.h"

#include <cstdint>

namespace RenderDog
{
	struct CameraDesc
	{
		Vector3		position;
		Vector3		direction;
		float		fov;
		float		aspectRitio;
		float		nearPlane;
		float		farPlane;
	};

	class FPSCamera
	{
	public:
		enum MoveMode
		{
			FrontAndBack,
			UpAndDown,
			LeftAndRight
		};

	public:
		FPSCamera();
		~FPSCamera();

		FPSCamera(const CameraDesc& cameraDesc);

		Matrix4x4		GetViewMatrix() const;
		Matrix4x4		GetPerspProjectionMatrix() const;

		void			Move(float speed, MoveMode moveMode);
		void			Rotate(float deltaYaw, float deltaPitch, float speed);

		void			OnWindowResize(uint32_t width, uint32_t height);

	private:
		Vector3			m_Postion;
		Vector3			m_Direction;

		Vector3			m_Right;
		Vector3			m_Up;

		float			m_Fov;		//½Ç¶È
		float			m_Aspect;  //¿í¸ß±È
		float			m_Near;
		float			m_Far;
	};
}