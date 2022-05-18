////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Camera.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Camera.h"

#include "Transform.h"

namespace RenderDog
{
	FPSCamera::FPSCamera() :
		m_Postion(0.0f, 0.0f, -5.0f),
		m_Direction(0.0f, 0.0f, 1.0f),
		m_Right(1.0f, 0.0f, 0.0f),
		m_Up(0.0f, 1.0f, 0.0f),
		m_Fov(45.0f),
		m_Aspect(1.333f),
		m_Near(0.1f),
		m_Far(1000.0f)
	{}

	FPSCamera::~FPSCamera()
	{}

	FPSCamera::FPSCamera(const CameraDesc& cameraDesc) :
		m_Postion(cameraDesc.position),
		m_Direction(cameraDesc.direction),
		m_Right(1.0f, 0.0f, 0.0f),
		m_Up(0.0f, 1.0f, 0.0f),
		m_Fov(cameraDesc.fov),
		m_Aspect(cameraDesc.aspectRitio),
		m_Near(cameraDesc.nearPlane),
		m_Far(cameraDesc.farPlane)
	{
		Matrix4x4 matView = GetLookAtMatrixLH(m_Postion, m_Postion + m_Direction, Vector3(0, 1, 0));

		m_Right = Vector3(matView(0, 0), matView(1, 0), matView(2, 0));
		m_Up = Vector3(matView(0, 1), matView(1, 1), matView(2, 1));
	}

	Matrix4x4 FPSCamera::GetViewMatrix() const
	{
		return GetLookAtMatrixLH(m_Postion, m_Postion + m_Direction, Vector3(0.0f, 1.0f, 0.0f));
	}

	Matrix4x4 FPSCamera::GetPerspProjectionMatrix() const
	{
		return GetPerspProjectionMatrixLH(m_Fov, m_Aspect, m_Near, m_Far);
	}

	void FPSCamera::Move(float speed, MOVE_MODE moveMode)
	{
		switch (moveMode)
		{
		case MOVE_MODE::FRONT_BACK:
			m_Postion += speed * m_Direction;
			break;
		case MOVE_MODE::UP_DOWN:
			m_Postion += speed * m_Up;
			break;
		case MOVE_MODE::LEFT_RIGHT:
			m_Postion += speed * m_Right;
			break;
		default:
			break;
		}
	}

	void FPSCamera::Rotate(float deltaYaw, float deltaPitch, float speed)
	{
		Matrix4x4 matRotateY = GetRotationMatrix(deltaYaw * speed, Vector3(0, 1, 0));
		Matrix4x4 matRotateX = GetRotationMatrix(deltaPitch * speed, Vector3(1, 0, 0));

		Vector4 direction = Vector4(m_Direction, 0.0f);
		direction = direction * matRotateX * matRotateY;

		m_Direction.x = direction.x;
		m_Direction.y = direction.y;
		m_Direction.z = direction.z;

		Matrix4x4 matView = GetLookAtMatrixLH(m_Postion, m_Postion + m_Direction, Vector3(0, 1, 0));

		m_Right = Vector3(matView(0, 0), matView(1, 0), matView(2, 0));
		m_Up = Vector3(matView(0, 1), matView(1, 1), matView(2, 1));
	}

	void FPSCamera::OnWindowResize(uint32_t width, uint32_t height)
	{
		m_Aspect = float(width) / (float)height;
	}
}