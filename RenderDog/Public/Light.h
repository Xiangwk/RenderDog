///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Light.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include "Vector.h"
#include "RefCntObject.h"

namespace RenderDog
{
	class IScene;

	enum class LIGHT_TYPE
	{
		DIRECTIONAL,
		POINT,
		SPOT
	};

	struct LightDesc
	{
		LIGHT_TYPE	type;
		Vector3		color;
		Vector3		eulerDir;
		float		luminance;
	};

	class ILight
	{
	protected:
		virtual ~ILight() = default;

	public:
		virtual void		Release() = 0;

		virtual LIGHT_TYPE	GetType() const = 0;

		virtual Vector3		GetDirection() const = 0;
		virtual Vector3		GetEulerAngle() const = 0;
		virtual Vector3		GetColor() const = 0;
		virtual float		GetLuminance() const = 0;
		
		virtual void		SetDirection(float eulerX, float eulerY, float eulerZ) = 0;
		virtual void		SetColor(const Vector3& color) = 0;
		virtual void		SetLuminance(float luma) = 0;

		virtual void		RegisterToScene(IScene* pScene) = 0;
	};

	class ILightManager
	{
	public:
		virtual ~ILightManager() = default;

		virtual	ILight*		CreateLight(const LightDesc& desc) = 0;
	};

	extern ILightManager* g_pILightManager;

}// namespace RenderDog