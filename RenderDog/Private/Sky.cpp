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
		m_pSkyModel(nullptr)
	{
		RenderDog::GeometryGenerator::StandardMeshData BoxMeshData;
		RenderDog::g_pGeometryGenerator->GenerateBox(1, 1, 1, BoxMeshData);
		m_pSkyModel = new RenderDog::StaticModel();
		m_pSkyModel->LoadFromStandardData(BoxMeshData.vertices, BoxMeshData.indices, "Shaders/SkyVertexShader.hlsl", "Shaders/SkyPixelShader.hlsl", "SkyBox");
		if (!m_pSkyModel->LoadTextureFromFile(texFilePath, L""))
		{
			MessageBox(nullptr, "SkyBox Load Texture Failed!", "ERROR", MB_OK);
		}

		m_pSkyModel->SetPosGesture(RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(1000.0f));
	}

	SkyBox::~SkyBox()
	{

	}

}// namespace RenderDog