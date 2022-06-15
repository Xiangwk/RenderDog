////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Scene.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include <string>

namespace RenderDog
{
	class	FPSCamera;
	class	IPrimitive;
	class	ILight;
	struct	BoundingSphere;

	struct SceneInitDesc
	{
		std::string name;
	};

	class IScene
	{
	protected:
		virtual ~IScene() = default;

	public:
		virtual void					Release() = 0;

		virtual void					RegisterPrimitive(IPrimitive* pPrimitive) = 0;
		virtual IPrimitive*				GetPrimitive(uint32_t index) = 0;
		virtual uint32_t				GetPrimitivesNum() const = 0;

		virtual	void					RegisterLight(ILight* pLight) = 0;
		virtual ILight*					GetLight(uint32_t index) = 0;
		virtual uint32_t				GetLightsNum() const = 0;

		virtual const BoundingSphere&	GetBoundingSphere() const = 0;
		virtual BoundingSphere&			GetBoundingSphere() = 0;
	};

	class ISceneManager
	{
	public:
		virtual ~ISceneManager() = default;

		virtual IScene*					CreateScene(const SceneInitDesc& sceneDesc) = 0;
	};

	extern ISceneManager* g_pISceneManager;

}// namespace RenderDog
