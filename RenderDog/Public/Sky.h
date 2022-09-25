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
	class IMaterial;
	class StaticMesh;

	class SkyBox
	{
	public:
		SkyBox(IMaterial* pMtl);
		~SkyBox();

		SkyBox(const SkyBox&) = delete;
		SkyBox& operator=(const SkyBox&) = delete;

		void				RegisterToScene(IScene* pScene);

		IPrimitive*			GetPrimitive() { return (IPrimitive*)m_pSkyMesh; }

		ITexture2D*			GetCubeTexture();
		ISamplerState*		GetCubeTextureSampler();

	private:
		StaticMesh*			m_pSkyMesh;

		ITexture2D*			m_pCubeTexture;
		ISamplerState*		m_pSamplerState;
	};

}// namespace RenderDog
