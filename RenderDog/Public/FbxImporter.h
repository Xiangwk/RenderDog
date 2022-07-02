///////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: FbxImporter.h
//Written by Xiang Weikang
///////////////////////////////////////////

#pragma once

#include "fbxsdk.h"

namespace RenderDog
{
	class RDFbxImporter
	{
	public:
		RDFbxImporter();
		~RDFbxImporter();

		bool			Init();
		void			Release();

		bool			LoadFbxFile();

	private:
		FbxManager*		m_pSdkManager;
		FbxScene*		m_pScene;
	};

	extern RDFbxImporter* g_pRDFbxImporter;

}// namespace RenderDog