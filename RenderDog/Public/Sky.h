////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Sky.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "StaticModel.h"

#include <string>

namespace RenderDog
{
	class IScene;

	class SkyBox
	{
	public:
		SkyBox(const std::wstring& texFilePath);
		~SkyBox();

		SkyBox(const SkyBox&) = delete;
		SkyBox& operator=(const SkyBox&) = delete;

		void			RegisterToScene(IScene* pScene);

	private:
		StaticModel*	m_pSkyModel;
	};

}// namespace RenderDog
