///////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: FbxImporter.cpp
//Written by Xiang Weikang
///////////////////////////////////////////

#include "FbxImporter.h"

namespace RenderDog
{
	RDFbxImporter	g_RDFbxImporter;
	RDFbxImporter*	g_pRDFbxImporter = &g_RDFbxImporter;

	RDFbxImporter::RDFbxImporter() :
		m_pSdkManager(nullptr),
		m_pScene(nullptr)
	{}

	RDFbxImporter::~RDFbxImporter()
	{

	}

	bool RDFbxImporter::Init()
	{
		m_pSdkManager = FbxManager::Create();
		if (!m_pSdkManager)
		{
			FBXSDK_printf("Error: Unable to create FBX Manager!\n");
			return false;
		}
		else
		{
			FBXSDK_printf("Autodesk FBX SDK version %s\n", m_pSdkManager->GetVersion());
		}

		FbxIOSettings* ioSettings = FbxIOSettings::Create(m_pSdkManager, IOSROOT);
		m_pSdkManager->SetIOSettings(ioSettings);

		
		FbxString lPath = FbxGetApplicationDirectory();
		m_pSdkManager->LoadPluginsDirectory(lPath.Buffer());

		
		m_pScene = FbxScene::Create(m_pSdkManager, "RenderDog FBX Scene");
		if (!m_pScene)
		{
			FBXSDK_printf("Error: Unable to create FBX scene!\n");
			return false;
		}

		return true;
	}

	void RDFbxImporter::Release()
	{
		if (m_pSdkManager)
		{
			m_pSdkManager->Destroy();
			m_pSdkManager = nullptr;
		}
	}

	bool RDFbxImporter::LoadFbxFile()
	{
		return true;
	}

}// namespace RenderDog