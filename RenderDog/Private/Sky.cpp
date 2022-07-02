////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Sky.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Sky.h"
#include "Scene.h"
#include "GeometryGenerator.h"

namespace RenderDog
{
	SkyBox::SkyBox(const std::wstring& texFilePath):
		m_pSkyMesh(nullptr)
	{
		RenderDog::GeometryGenerator::StandardMeshData BoxMeshData;
		RenderDog::g_pGeometryGenerator->GenerateBox(1, 1, 1, BoxMeshData);
		m_pSkyMesh = new RenderDog::StaticMesh();
		m_pSkyMesh->LoadFromStandardData(BoxMeshData.vertices, BoxMeshData.indices, "SkyBox");
		m_pSkyMesh->InitRenderData("Shaders/SkyVertexShader.hlsl", "Shaders/SkyPixelShader.hlsl");
		if (!m_pSkyMesh->LoadTextureFromFile(texFilePath, L""))
		{
			MessageBox(nullptr, "SkyBox Load Texture Failed!", "ERROR", MB_OK);
		}

		m_pSkyMesh->SetPosGesture(RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(1000.0f));
	}

	SkyBox::~SkyBox()
	{
		if (m_pSkyMesh)
		{
			delete m_pSkyMesh;
			m_pSkyMesh = nullptr;
		}
	}

	void SkyBox::RegisterToScene(IScene* pScene)
	{
		pScene->RegisterSkyBox(this);
	}

}// namespace RenderDog