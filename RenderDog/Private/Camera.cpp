////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Camera.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Camera.h"
#include "Utility.h"
#include "Transform.h"

namespace RenderDog
{
	FPSCamera::FPSCamera() :
		m_Postion(0.0f, 0.0f, -5.0f),
		m_Yaw(0.0f),
		m_Pitch(0.0f),
		m_Right(1.0f, 0.0f, 0.0f),
		m_Up(0.0f, 1.0f, 0.0f),
		m_Fov(45.0f),
		m_Aspect(1.333f),
		m_Near(0.1f),
		m_Far(1000.0f),
		m_moveSpeed(1.0f),
		m_rotSpeed(1.0f)
	{
		m_Direction.x = sinf(m_Yaw);
		m_Direction.y = sinf(m_Pitch);
		m_Direction.z = cosf(m_Yaw);
	}

	FPSCamera::~FPSCamera()
	{}

	FPSCamera::FPSCamera(const CameraDesc& cameraDesc) :
		m_Postion(cameraDesc.position),
		m_Yaw(0.0f),
		m_Pitch(0.0f),
		m_Right(1.0f, 0.0f, 0.0f),
		m_Up(0.0f, 1.0f, 0.0f),
		m_Fov(cameraDesc.fov),
		m_Aspect(cameraDesc.aspectRitio),
		m_Near(cameraDesc.nearPlane),
		m_Far(cameraDesc.farPlane),
		m_moveSpeed(cameraDesc.moveSpeed),
		m_rotSpeed(cameraDesc.rotSpeed)
	{
		Matrix4x4 matView = GetLookAtMatrixLH(m_Postion, m_Postion + m_Direction, Vector3(0, 1, 0));

		m_Right = Vector3(matView(0, 0), matView(1, 0), matView(2, 0));
		m_Up = Vector3(matView(0, 1), matView(1, 1), matView(2, 1));

		m_Direction.x = sinf(m_Yaw);
		m_Direction.y = sinf(m_Pitch);
		m_Direction.z = cosf(m_Yaw);
	}

	Matrix4x4 FPSCamera::GetViewMatrix() const
	{
		return GetLookAtMatrixLH(m_Postion, m_Postion + m_Direction, Vector3(0.0f, 1.0f, 0.0f));
	}

	Matrix4x4 FPSCamera::GetPerspProjectionMatrix() const
	{
		return GetPerspProjectionMatrixLH(m_Fov, m_Aspect, m_Near, m_Far);
	}

	void FPSCamera::Move(MOVE_MODE moveMode)
	{
		switch (moveMode)
		{
		case MOVE_MODE::FRONT:
			m_Postion += m_moveSpeed * m_Direction;
			break;
		case MOVE_MODE::BACK:
			m_Postion += m_moveSpeed * (-m_Direction);
			break;
		case MOVE_MODE::UP:
			m_Postion += m_moveSpeed * m_Up;
			break;
		case MOVE_MODE::DOWN:
			m_Postion += m_moveSpeed * (-m_Up);
			break;
		case MOVE_MODE::RIGHT:
			m_Postion += m_moveSpeed * m_Right;
			break;
		case MOVE_MODE::LEFT:
			m_Postion += m_moveSpeed * (-m_Right);
			break;
		default:
			break;
		}
	}

	void FPSCamera::Rotate(float deltaYaw, float deltaPitch)
	{
		m_Yaw += deltaYaw * m_rotSpeed;

		m_Pitch += deltaPitch * m_rotSpeed;
		if (m_Pitch >= 0.5f * RD_PI)
		{
			m_Pitch = 0.5f * RD_PI - RD_FLT_EPSILON;
		}
		if (m_Pitch <= -0.5f * RD_PI)
		{
			m_Pitch = -0.5f * RD_PI + RD_FLT_EPSILON;
		}

		m_Direction.x = sinf(m_Yaw);
		m_Direction.y = sinf(m_Pitch);
		m_Direction.z = cosf(m_Yaw);

		Matrix4x4 matView = GetLookAtMatrixLH(m_Postion, m_Postion + m_Direction, Vector3(0, 1, 0));

		m_Right = Vector3(matView(0, 0), matView(1, 0), matView(2, 0));
		m_Up = Vector3(matView(0, 1), matView(1, 1), matView(2, 1));
	}

	void FPSCamera::OnWindowResize(uint32_t width, uint32_t height)
	{
		m_Aspect = float(width) / (float)height;
	}
}