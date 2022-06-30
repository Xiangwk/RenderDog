////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Sky.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include <string>

namespace RenderDog
{
	class IScene;
	class IPrimitive;
	class StaticMesh;

	class SkyBox
	{
	public:
		SkyBox(const std::wstring& texFilePath);
		~SkyBox();

		SkyBox(const SkyBox&) = delete;
		SkyBox& operator=(const SkyBox&) = delete;

		void			RegisterToScene(IScene* pScene);

		IPrimitive*		GetPrimitive() { return (IPrimitive*)m_pSkyMesh; }

	private:
		StaticMesh*		m_pSkyMesh;
	};

}// namespace RenderDog
