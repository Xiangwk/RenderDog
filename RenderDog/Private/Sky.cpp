////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Sky.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Sky.h"
#include "Scene.h"
#include "GeometryGenerator.h"
#include "Material.h"

namespace RenderDog
{
	SkyBox::SkyBox(IMaterial* pMtl):
		m_pSkyMesh(nullptr),
		m_pCubeTexture(nullptr),
		m_pSamplerState(nullptr)
	{
		RenderDog::GeometryGenerator::StandardMeshData BoxMeshData;
		RenderDog::g_pGeometryGenerator->GenerateBox(1, 1, 1, BoxMeshData);
		m_pSkyMesh = new RenderDog::StaticMesh();
		m_pSkyMesh->LoadFromStandardData(BoxMeshData.vertices, BoxMeshData.indices, "SkyBox");
		m_pSkyMesh->InitRenderData();
		if (!m_pSkyMesh->CreateMaterialInstance(pMtl))
		{
			MessageBox(nullptr, "SkyBox Load Texture Failed!", "ERROR", MB_OK);
		}

		m_pSkyMesh->SetPosGesture(RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(100000.0f));

		IMaterialInstance* pMtlIns = m_pSkyMesh->GetMaterialInstance();
		for (uint32_t i = 0; i < pMtlIns->GetMaterialParamNum(); ++i)
		{
			MaterialParam& param = pMtlIns->GetMaterialParamByIndex(i);
			if (param.GetType() == MATERIAL_PARAM_TYPE::TEXTURE2D)
			{
				m_pCubeTexture = param.GetTexture2D();
			}
			if (param.GetType() == MATERIAL_PARAM_TYPE::SAMPLER)
			{
				m_pSamplerState = param.GetSamplerState();
			}
		}
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

	ITexture2D* SkyBox::GetCubeTexture()
	{
		return m_pCubeTexture;
	}

	ISamplerState* SkyBox::GetCubeTextureSampler()
	{
		return m_pSamplerState;
	}

}// namespace RenderDog