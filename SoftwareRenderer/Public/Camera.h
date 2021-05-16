#pragma once

#include "Matrix.h"

namespace RenderDog
{
	struct CameraDesc
	{
		Vector3 vPosition;
		Vector3 vDirection;
		float	fFov;
		float	fAspect;
		float	fNear;
		float	fFar;
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

		Matrix4x4 GetViewMatrix() const;
		Matrix4x4 GetPerspProjectionMatrix() const;

		void Move(float fSpeed, MoveMode moveMode);
		void Rotate(float fDeltaYaw, float fDeltaPitch, float fSpeed);

	private:
		Vector3	m_vPostion;
		Vector3	m_vDirection;

		Vector3 m_vRight;
		Vector3 m_vUp;

		float	m_fFov;		//½Ç¶È
		float	m_fAspect;  //¿í¸ß±È
		float   m_fNear;
		float	m_fFar;
	};
}