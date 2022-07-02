////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Sky.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "StaticMesh.h"

#include <string>

namespace RenderDog
{
	class IScene;
	class IPrimitive;
	class ITexture2D;
	class ISamplerState;
	class StaticMesh;

	class SkyBox
	{
	public:
		SkyBox(const std::wstring& texFilePath);
		~SkyBox();

		SkyBox(const SkyBox&) = delete;
		SkyBox& operator=(const SkyBox&) = delete;

		void				RegisterToScene(IScene* pScene);

		IPrimitive*			GetPrimitive() { return (IPrimitive*)m_pSkyMesh; }

		ITexture2D*			GetCubeTexture() { return m_pSkyMesh->GetDiffuseTexture(); }
		ISamplerState*		GetCubeTextureSampler() { return m_pSkyMesh->GetDiffuseSampler(); }

	private:
		StaticMesh*			m_pSkyMesh;
	};

}// namespace RenderDog
