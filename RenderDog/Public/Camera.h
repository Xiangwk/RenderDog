////////////////////////////////////////
//RenderDog <��,��>
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
		float		yaw;
		float		pitch;
		float		fov;
		float		aspectRitio;
		float		nearPlane;
		float		farPlane;
		float		moveSpeed;
		float		rotSpeed;

		CameraDesc() :
			position(0.0f, 0.0f, 0.0f),
			yaw(0.0f),
			pitch(0.0f),
			fov(45.0f),
			aspectRitio(1.333f),
			nearPlane(0.01f),
			farPlane(1000.0f),
			moveSpeed(1.0f),
			rotSpeed(1.0f)
		{}
	};

	class FPSCamera
	{
	public:
		enum class MOVE_MODE
		{
			FRONT,
			BACK,
			UP,
			DOWN,
			LEFT,
			RIGHT
		};

	public:
		FPSCamera();
		~FPSCamera();

		FPSCamera(const CameraDesc& cameraDesc);

		Matrix4x4		GetViewMatrix() const;
		Matrix4x4		GetPerspProjectionMatrix() const;

		void			Move(MOVE_MODE moveMode);
		void			Rotate(float deltaYaw, float deltaPitch);

		void			OnWindowResize(uint32_t width, uint32_t height);

	private:
		Vector3			m_Postion;
		Vector3			m_Direction;

		Vector3			m_Right;
		Vector3			m_Up;

		float			m_Yaw;		//ƫ���ǣ����ȱ�ʾ��
		float			m_Pitch;	//�����ǣ����ȱ�ʾ��

		float			m_Fov;		//�Ƕ�
		float			m_Aspect;	//��߱�
		float			m_Near;
		float			m_Far;

		float			m_moveSpeed;
		float			m_rotSpeed;
	};
}