#include "Camera.h"

#include "Transform.h"

namespace RenderDog
{
	FPSCamera::FPSCamera() :
		m_vPostion(0.0f, 0.0f, -5.0f),
		m_vDirection(0.0f, 0.0f, 1.0f),
		m_vRight(1.0f, 0.0f, 0.0f),
		m_vUp(0.0f, 1.0f, 0.0f),
		m_fFov(45.0f),
		m_fAspect(1.333f),
		m_fNear(0.1f),
		m_fFar(1000.0f)
	{}

	FPSCamera::~FPSCamera()
	{}

	FPSCamera::FPSCamera(const CameraDesc& cameraDesc) :
		m_vPostion(cameraDesc.vPosition),
		m_vDirection(cameraDesc.vDirection),
		m_vRight(1.0f, 0.0f, 0.0f),
		m_vUp(0.0f, 1.0f, 0.0f),
		m_fFov(cameraDesc.fFov),
		m_fAspect(cameraDesc.fAspect),
		m_fNear(cameraDesc.fNear),
		m_fFar(cameraDesc.fFar)
	{
		Matrix4x4 matView = GetLookAtMatrixLH(m_vPostion, m_vPostion + m_vDirection, Vector3(0, 1, 0));

		m_vRight = Vector3(matView(0, 0), matView(1, 0), matView(2, 0));
		m_vUp = Vector3(matView(0, 1), matView(1, 1), matView(2, 1));
	}

	Matrix4x4 FPSCamera::GetViewMatrix() const
	{
		return GetLookAtMatrixLH(m_vPostion, m_vPostion + m_vDirection, Vector3(0.0f, 1.0f, 0.0f));
	}

	Matrix4x4 FPSCamera::GetPerspProjectionMatrix() const
	{
		return GetPerspProjectionMatrixLH(m_fFov, m_fAspect, m_fNear, m_fFar);
	}

	void FPSCamera::Move(float fSpeed, MoveMode moveMode)
	{
		switch (moveMode)
		{
		case RenderDog::FPSCamera::FrontAndBack:
			m_vPostion += fSpeed * m_vDirection;
			break;
		case RenderDog::FPSCamera::UpAndDown:
			m_vPostion += fSpeed * m_vUp;
			break;
		case RenderDog::FPSCamera::LeftAndRight:
			m_vPostion += fSpeed * m_vRight;
			break;
		default:
			break;
		}
	}

	void FPSCamera::Rotate(float fDeltaYaw, float fDeltaPitch, float fSpeed)
	{
		Matrix4x4 matRotateY = GetRotationMatrix(fDeltaYaw * fSpeed, Vector3(0, 1, 0));
		Matrix4x4 matRotateX = GetRotationMatrix(fDeltaPitch * fSpeed, Vector3(1, 0, 0));

		Vector4 vDir = Vector4(m_vDirection, 0.0f);
		vDir = vDir * matRotateX * matRotateY;

		m_vDirection.x = vDir.x;
		m_vDirection.y = vDir.y;
		m_vDirection.z = vDir.z;

		Matrix4x4 matView = GetLookAtMatrixLH(m_vPostion, m_vPostion + m_vDirection, Vector3(0, 1, 0));

		m_vRight = Vector3(matView(0, 0), matView(1, 0), matView(2, 0));
		m_vUp = Vector3(matView(0, 1), matView(1, 1), matView(2, 1));
	}
}