////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Scene.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "RefCntObject.h"

#include <string>

namespace RenderDog
{
	class IPrimitive;

	struct SceneInitDesc
	{
		std::string name;
	};

	class IScene : public RefCntObject
	{
	public:
		virtual ~IScene() = default;

		virtual bool Init(const SceneInitDesc& desc) = 0;
		virtual void Release() = 0;

		virtual void		RegisterPrimitive(IPrimitive* pPrimitive) = 0;
		virtual IPrimitive* GetPrimitive(uint32_t index) = 0;
		virtual uint32_t	GetPrimitivesNum() const = 0;
	};

	class ISceneManager
	{
	public:
		virtual ~ISceneManager() = default;

		virtual IScene* CreateScene(const SceneInitDesc& sceneDesc) = 0;
		virtual void	ReleaseScene(IScene* pScene) = 0;
	};

	extern ISceneManager* g_pISceneManager;

}// namespace RenderDog
