///////////////////////////////////////////
//ModelViewer
//FileName: ModelViewer.cpp
//Written by Xiang Weikang
///////////////////////////////////////////

#include "ModelViewer.h"
#include "Vector.h"
#include "GeometryGenerator.h"
#include "Utility.h"
#include "FbxImporter.h"

#include <windowsx.h>
#include <sstream>

#define MODEL_VIEWER_LOAD_STANDARD_MODEL	0
#define MODEL_VIEWER_LOAD_STATIC_MODEL		1
#define MODEL_VIEWER_LOAD_SKIN_MODEL		0

ModelViewer g_ModelViewer;
ModelViewer* g_pModelViewer = &g_ModelViewer;

int ModelViewer::m_Keys[512];

ModelViewer::ModelViewer() :
	m_pRenderDog(nullptr),
	m_pScene(nullptr),
	m_pGridLine(nullptr),
	m_pFloor(nullptr),
	m_pStaticModel(nullptr),
	m_pSkinModel(nullptr),
	m_pSkyBox(nullptr),
	m_pFPSCamera(nullptr),
	m_pMainLight(nullptr),
	m_pGameTimer(nullptr),
	m_LastMousePosX(0),
	m_LastMousePosY(0),
	m_bShowUnitGrid(true),
	m_bModelMoved(false),
	m_pBasicMaterial(nullptr),
	m_pSkyMaterial(nullptr)
{
	memset(m_Keys, 0, sizeof(int) * 512);
}

ModelViewer::~ModelViewer()
{}

bool ModelViewer::Init(const ModelViewerInitDesc& desc)
{
	RenderDog::CameraDesc camDesc;
	camDesc.position = RenderDog::Vector3(0.0f, 100.0f, -600.0f);
	camDesc.yaw = 0.0f;
	camDesc.pitch = 0.0f;
	camDesc.fov = 45.0f;
	camDesc.aspectRitio = (float)desc.wndDesc.width / (float)desc.wndDesc.height;
	camDesc.nearPlane = 0.1f;
	camDesc.farPlane = 100000.0f;
	camDesc.moveSpeed = 150.0f;
	camDesc.rotSpeed = 0.05f;
	m_pFPSCamera = new RenderDog::FPSCamera(camDesc);

	if (!RenderDog::CreateRenderDog(&m_pRenderDog))
	{
		return false;
	}

	RenderDog::InitDesc renderDogDesc;
	renderDogDesc.wndDesc = desc.wndDesc;
	renderDogDesc.pMainCamera = m_pFPSCamera;
	if (!m_pRenderDog->Init(renderDogDesc))
	{
		MessageBox(nullptr, "RenderDog Init Failed!", "ERROR", MB_OK);
		return false;
	}

	if (!RenderDog::g_pRDFbxImporter->Init())
	{
		MessageBox(nullptr, "RenderDog::FBXImporter Init Failed!", "ERROR", MB_OK);
		return false;
	}

	RenderDog::SceneInitDesc sceneDesc;
	sceneDesc.name = "MainScene";
	m_pScene = RenderDog::g_pISceneManager->CreateScene(sceneDesc);
	if (!m_pScene)
	{
		MessageBox(nullptr, "Create Scene Failed!", "ERROR", MB_OK);
		return false;
	}

	m_pBasicMaterial = CreateBasicMaterial("UserAsset/Materials/Basic.mtl");
	m_pSkyMaterial = CreateSkyMaterial("UserAsset/Materials/Basic.mtl");

	if (!LoadFloor(10, 10, 100.0f))
	{
		MessageBox(nullptr, "Load Floor Failed!", "ERROR", MB_OK);
		return false;
	}

	if (!LoadSkyBox())
	{
		MessageBox(nullptr, "Load Sky Failed!", "ERROR", MB_OK);
		return false;
	}

#if	MODEL_VIEWER_LOAD_STANDARD_MODEL
	if (!LoadFbxModel("", LOAD_MODEL_TYPE::STANDARD))
	{
		MessageBox(nullptr, "Load Static Model Failed!", "ERROR", MB_OK);
		return false;
	}
	m_pStaticModel->SetPosGesture(RenderDog::Vector3(0.0f, 100.0f, 0.0f), RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(1.0f));
#endif //MODEL_VIEWER_LOAD_STANDARD_MODEL

#if MODEL_VIEWER_LOAD_STATIC_MODEL
	if (!LoadFbxModel("Models/Cerberus/Cerberus_LP.FBX", LOAD_MODEL_TYPE::CUSTOM_STATIC))
	{
		MessageBox(nullptr, "Load Static Model Failed!", "ERROR", MB_OK);
		return false;
	}
	m_pStaticModel->SetPosGesture(RenderDog::Vector3(0.0f, 100.0f, 0.0f), RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(1.0f));
#endif //MODEL_VIEWER_LOAD_STATIC_MODEL

#if MODEL_VIEWER_LOAD_SKIN_MODEL
	if (!LoadFbxModel("Models/Crunch/Crunch_Crash_Site.FBX", LOAD_MODEL_TYPE::CUSTOM_SKIN))
	{
		MessageBox(nullptr, "Load Skin Model Failed!", "ERROR", MB_OK);
		return false;
	}
	m_pSkinModel->SetPosGesture(RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(1.0f));

	if (!LoadFbxAnimation("Models/Crunch/Animations/Idle_Combat.FBX", m_pSkinModel))
	{
		MessageBox(nullptr, "Load Skin Model Animation Failed!", "ERROR", MB_OK);
		return false;
	}
#endif //MODEL_VIEWER_LOAD_SKIN_MODEL

	RenderDog::LightDesc lightDesc = {};
	lightDesc.type = RenderDog::LIGHT_TYPE::DIRECTIONAL;
	lightDesc.color = RenderDog::Vector3(1.0f, 1.0f, 1.0f);
	lightDesc.eulerDir = RenderDog::Vector3(45.0f, 45.0f, 0.0f);
	lightDesc.luminance = 30.0f;
	m_pMainLight = RenderDog::g_pILightManager->CreateLight(lightDesc);

	m_pMainLight->RegisterToScene(m_pScene);

	RenderDog::g_pIFramework->RegisterScene(m_pScene);

	m_pGameTimer = new RenderDog::GameTimer();
	m_pGameTimer->Reset();

	return true;
}

void ModelViewer::Release()
{
	RenderDog::g_pRDFbxImporter->Release();

	if (m_pGridLine)
	{
		delete m_pGridLine;
		m_pGridLine = nullptr;
	}

	if (m_pFloor)
	{
		delete m_pFloor;
		m_pFloor = nullptr;
	}

	if (m_pBasicMaterial)
	{
		m_pBasicMaterial->Release();
		m_pBasicMaterial = nullptr;
	}

	if (m_pSkyMaterial)
	{
		m_pSkyMaterial->Release();
		m_pSkyMaterial = nullptr;
	}

	if (m_pStaticModel)
	{
		delete m_pStaticModel;
		m_pStaticModel = nullptr;
	}

	if (m_pSkinModel)
	{
		delete m_pSkinModel;
		m_pSkinModel = nullptr;
	}

	if (m_pSkyBox)
	{
		delete m_pSkyBox;
		m_pSkyBox = nullptr;
	}

	if (m_pMainLight)
	{
		m_pMainLight->Release();
	}

	if (m_pFPSCamera)
	{
		delete m_pFPSCamera;
		m_pFPSCamera = nullptr;
	}

	if (m_pScene)
	{
		m_pScene->Release();
		m_pScene = nullptr;
	}
	

	if (m_pGameTimer)
	{
		delete m_pGameTimer;
		m_pGameTimer = nullptr;
	}

	m_pRenderDog->Release();

	RenderDog::DestoryRenderDog(&m_pRenderDog);
}

int ModelViewer::Run()
{
	MSG Msg = { 0 };
	while (Msg.message != WM_QUIT)
	{
		if (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		else
		{
			m_pGameTimer->Tick();
			CalculateFrameStats();

			float deltaTime = static_cast<float>(m_pGameTimer->GetDeltaTime());
			Update(deltaTime);
			RenderDog::g_pIFramework->Frame();
		}
	}

	return (int)Msg.wParam;
}

LRESULT ModelViewer::MessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		//当窗口切换激活状态时（激活或者未激活）发送该消息
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
		{
		}
		else
		{
		}
		break;
	}
	//当窗口改变大小时发送该消息
	case WM_SIZE:
	{
		uint32_t newWidth = LOWORD(lParam);
		uint32_t newHeight = HIWORD(lParam);

		RenderDog::g_pIWindow->SetWidth(newWidth);
		RenderDog::g_pIWindow->SetHeight(newHeight);

		if (wParam != SIZE_MINIMIZED)
		{
			RenderDog::g_pIFramework->OnResize(newWidth, newHeight);
		}

		break;
	}
	case WM_ENTERSIZEMOVE:
	{
		break;
	}
	case WM_EXITSIZEMOVE:
	{
		break;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	{
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	}
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	{
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	}
	case WM_MOUSEMOVE:
	{
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	}
	case WM_MOUSEWHEEL:
	{
		break;
	}
	case WM_KEYDOWN:
	{
		m_Keys[wParam & 511] = 1;
		break;
	}
		
	case WM_KEYUP:
	{
		m_Keys[wParam & 511] = 0;
		break;
	}
	default:
	{
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	}
	return 0;
}

bool ModelViewer::LoadFloor(uint32_t width, uint32_t depth, float unit)
{
	RenderDog::GeometryGenerator::SimpleMeshData GridLineMeshData;
	RenderDog::g_pGeometryGenerator->GenerateGridLine(width, depth, unit, RenderDog::Vector4(0.8f, 0.8f, 0.8f, 1.0f), GridLineMeshData);
	m_pGridLine = new RenderDog::SimpleModel();
	m_pGridLine->LoadFromSimpleData(GridLineMeshData.vertices, GridLineMeshData.indices, "MainSceneGridLine");
	m_pGridLine->SetPosGesture(RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(1.0f));
	

	RenderDog::GeometryGenerator::StandardMeshData GridMeshData;
	RenderDog::g_pGeometryGenerator->GenerateGrid(width, depth, unit, GridMeshData);
	m_pFloor = new RenderDog::StaticModel();
	m_pFloor->LoadFromStandardData(GridMeshData.vertices, GridMeshData.indices, "MainSceneFloor");
	if (!m_pFloor->CreateMaterialInstance(m_pBasicMaterial, nullptr))
	{
		MessageBox(nullptr, "Load Texture Failed!", "ERROR", MB_OK);
		return false;
	}
	m_pFloor->SetPosGesture(RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(1.0f));
	

	return true;
}

bool ModelViewer::LoadStaticModelMaterialParams(std::vector<RenderDog::MaterialParam>& params)
{
	std::wstring diffuseTexturePath = L"Models/Cerberus/Textures/Cerberus_diff.dds";
	std::wstring normalTexturePath = L"Models/Cerberus/Textures/Cerberus_norm.dds";
	std::wstring metallicRoughnessTexturePath = L"Models/Cerberus/Textures/Cerberus_mr.dds";

	if (!diffuseTexturePath.empty())
	{
		RenderDog::ITexture2D* pDiffuseTexture = RenderDog::g_pITextureManager->GetTexture2D(diffuseTexturePath);
		if (!pDiffuseTexture)
		{
			return false;
		}
		RenderDog::MaterialParam DiffuseTextureParam("LocVar_Material_DiffuseTexture", RenderDog::MATERIAL_PARAM_TYPE::TEXTURE2D);
		DiffuseTextureParam.SetTexture2D(pDiffuseTexture);
		params.push_back(DiffuseTextureParam);


		RenderDog::SamplerDesc samplerDesc = {};
		samplerDesc.name = "LocVar_Material_DiffuseTextureSampler";
		samplerDesc.filterMode = RenderDog::SAMPLER_FILTER::LINEAR;
		samplerDesc.addressMode = RenderDog::SAMPLER_ADDRESS::WRAP;
		RenderDog::ISamplerState* pDiffuseTextureSampler = RenderDog::g_pISamplerStateManager->GetSamplerState(samplerDesc);
		if (!pDiffuseTextureSampler)
		{
			return false;
		}
		RenderDog::MaterialParam DiffuseTextureSamplerParam("LocVar_Material_DiffuseTextureSampler", RenderDog::MATERIAL_PARAM_TYPE::SAMPLER);
		DiffuseTextureSamplerParam.SetSamplerState(pDiffuseTextureSampler);
		params.push_back(DiffuseTextureSamplerParam);
	}

	if (!normalTexturePath.empty())
	{
		RenderDog::ITexture2D* pNormalTexture = RenderDog::g_pITextureManager->GetTexture2D(normalTexturePath);
		if (!pNormalTexture)
		{
			return false;
		}
		RenderDog::MaterialParam NormalTextureParam("LocVar_Material_NormalTexture", RenderDog::MATERIAL_PARAM_TYPE::TEXTURE2D);
		NormalTextureParam.SetTexture2D(pNormalTexture);
		params.push_back(NormalTextureParam);

		RenderDog::SamplerDesc samplerDesc = {};
		samplerDesc.name = "LocVar_Material_NormalTextureSampler";
		samplerDesc.filterMode = RenderDog::SAMPLER_FILTER::LINEAR;
		samplerDesc.addressMode = RenderDog::SAMPLER_ADDRESS::WRAP;
		RenderDog::ISamplerState* pNormalTextureSampler = RenderDog::g_pISamplerStateManager->GetSamplerState(samplerDesc);
		if (!pNormalTextureSampler)
		{
			return false;
		}
		RenderDog::MaterialParam NormalTextureSamplerParam("LocVar_Material_NormalTextureSampler", RenderDog::MATERIAL_PARAM_TYPE::SAMPLER);
		NormalTextureSamplerParam.SetSamplerState(pNormalTextureSampler);
		params.push_back(NormalTextureSamplerParam);
	}

	if (!metallicRoughnessTexturePath.empty())
	{
		RenderDog::ITexture2D* pMRTexture = RenderDog::g_pITextureManager->GetTexture2D(metallicRoughnessTexturePath);
		if (!pMRTexture)
		{
			return false;
		}
		RenderDog::MaterialParam MRTextureParam("LocVar_Material_MetallicRoughnessTexture", RenderDog::MATERIAL_PARAM_TYPE::TEXTURE2D);
		MRTextureParam.SetTexture2D(pMRTexture);
		params.push_back(MRTextureParam);

		RenderDog::SamplerDesc samplerDesc = {};
		samplerDesc.name = "LocVar_Material_MetallicRoughnessTextureSampler";
		samplerDesc.filterMode = RenderDog::SAMPLER_FILTER::LINEAR;
		samplerDesc.addressMode = RenderDog::SAMPLER_ADDRESS::WRAP;
		RenderDog::ISamplerState* pMRTextureSampler = RenderDog::g_pISamplerStateManager->GetSamplerState(samplerDesc);
		if (!pMRTextureSampler)
		{
			return false;
		}
		RenderDog::MaterialParam MRTextureSamplerParam("LocVar_Material_MetallicRoughnessTextureSampler", RenderDog::MATERIAL_PARAM_TYPE::SAMPLER);
		MRTextureSamplerParam.SetSamplerState(pMRTextureSampler);
		params.push_back(MRTextureSamplerParam);
	}

	return true;
}

bool ModelViewer::LoadSkinModelMaterialParams(std::vector<RenderDog::MaterialParam>& params)
{
	std::wstring diffuseTexturePath = L"EngineAsset/Textures/Red_diff.dds";
	std::wstring normalTexturePath = L"EngineAsset/Textures/FlatNormal_norm.dds";
	std::wstring metallicRoughnessTexturePath = L"EngineAsset/Textures/Orange_mr.dds";

	if (!diffuseTexturePath.empty())
	{
		RenderDog::ITexture2D* pDiffuseTexture = RenderDog::g_pITextureManager->GetTexture2D(diffuseTexturePath);
		if (!pDiffuseTexture)
		{
			return false;
		}
		RenderDog::MaterialParam DiffuseTextureParam("DiffuseTexture", RenderDog::MATERIAL_PARAM_TYPE::TEXTURE2D);
		DiffuseTextureParam.SetTexture2D(pDiffuseTexture);
		params.push_back(DiffuseTextureParam);


		RenderDog::SamplerDesc samplerDesc = {};
		samplerDesc.name = "DiffuseTextureSampler";
		samplerDesc.filterMode = RenderDog::SAMPLER_FILTER::LINEAR;
		samplerDesc.addressMode = RenderDog::SAMPLER_ADDRESS::WRAP;
		RenderDog::ISamplerState* pDiffuseTextureSampler = RenderDog::g_pISamplerStateManager->GetSamplerState(samplerDesc);
		if (!pDiffuseTextureSampler)
		{
			return false;
		}
		RenderDog::MaterialParam DiffuseTextureSamplerParam("DiffuseTextureSampler", RenderDog::MATERIAL_PARAM_TYPE::SAMPLER);
		DiffuseTextureSamplerParam.SetSamplerState(pDiffuseTextureSampler);
		params.push_back(DiffuseTextureSamplerParam);
	}

	if (!normalTexturePath.empty())
	{
		RenderDog::ITexture2D* pNormalTexture = RenderDog::g_pITextureManager->GetTexture2D(normalTexturePath);
		if (!pNormalTexture)
		{
			return false;
		}
		RenderDog::MaterialParam NormalTextureParam("NormalTexture", RenderDog::MATERIAL_PARAM_TYPE::TEXTURE2D);
		NormalTextureParam.SetTexture2D(pNormalTexture);
		params.push_back(NormalTextureParam);

		RenderDog::SamplerDesc samplerDesc = {};
		samplerDesc.name = "NormalTextureSampler";
		samplerDesc.filterMode = RenderDog::SAMPLER_FILTER::LINEAR;
		samplerDesc.addressMode = RenderDog::SAMPLER_ADDRESS::WRAP;
		RenderDog::ISamplerState* pNormalTextureSampler = RenderDog::g_pISamplerStateManager->GetSamplerState(samplerDesc);
		if (!pNormalTextureSampler)
		{
			return false;
		}
		RenderDog::MaterialParam NormalTextureSamplerParam("NormalTextureSampler", RenderDog::MATERIAL_PARAM_TYPE::SAMPLER);
		NormalTextureSamplerParam.SetSamplerState(pNormalTextureSampler);
		params.push_back(NormalTextureSamplerParam);
	}

	if (!metallicRoughnessTexturePath.empty())
	{
		RenderDog::ITexture2D* pMRTexture = RenderDog::g_pITextureManager->GetTexture2D(metallicRoughnessTexturePath);
		if (!pMRTexture)
		{
			return false;
		}
		RenderDog::MaterialParam MRTextureParam("MetallicRoughnessTexture", RenderDog::MATERIAL_PARAM_TYPE::TEXTURE2D);
		MRTextureParam.SetTexture2D(pMRTexture);
		params.push_back(MRTextureParam);

		RenderDog::SamplerDesc samplerDesc = {};
		samplerDesc.name = "MetallicRoughnessTextureSampler";
		samplerDesc.filterMode = RenderDog::SAMPLER_FILTER::LINEAR;
		samplerDesc.addressMode = RenderDog::SAMPLER_ADDRESS::WRAP;
		RenderDog::ISamplerState* pMRTextureSampler = RenderDog::g_pISamplerStateManager->GetSamplerState(samplerDesc);
		if (!pMRTextureSampler)
		{
			return false;
		}
		RenderDog::MaterialParam MRTextureSamplerParam("MetallicRoughnessTextureSampler", RenderDog::MATERIAL_PARAM_TYPE::SAMPLER);
		MRTextureSamplerParam.SetSamplerState(pMRTextureSampler);
		params.push_back(MRTextureSamplerParam);
	}

	return true;
}

bool ModelViewer::LoadSkyBox()
{
	m_pSkyBox = new RenderDog::SkyBox(m_pSkyMaterial);

	return true;
}

bool ModelViewer::LoadFbxModel(const std::string& fileName, LOAD_MODEL_TYPE modelType)
{
	if (modelType == LOAD_MODEL_TYPE::STANDARD)
	{
		m_pStaticModel = new RenderDog::StaticModel();

		RenderDog::GeometryGenerator::StandardMeshData SphereMeshData;
		RenderDog::g_pGeometryGenerator->GenerateSphere(50, 50, 50, SphereMeshData);
		m_pStaticModel->LoadFromStandardData(SphereMeshData.vertices, SphereMeshData.indices, "Sphere");
		if (!m_pStaticModel->CreateMaterialInstance(m_pBasicMaterial, nullptr))
		{
			MessageBox(nullptr, "Load Texture Failed!", "ERROR", MB_OK);
			return false;
		}

		m_pStaticModel->SetPosGesture(RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(1.0f));
	}
	else if (modelType == LOAD_MODEL_TYPE::CUSTOM_STATIC)
	{
		m_pStaticModel = new RenderDog::StaticModel();

		RenderDog::RDFbxImporter::FbxLoadParam fbxLoadParam;
		fbxLoadParam.bIsFlipTexcoordV = true;
		if (!RenderDog::g_pRDFbxImporter->LoadFbxFile(fileName, RenderDog::RDFbxImporter::FBX_LOAD_TYPE::STATIC_MODEL, fbxLoadParam))
		{
			MessageBox(nullptr, "Import FBX File Failed!", "ERROR", MB_OK);
			return false;
		}

		if (!m_pStaticModel->LoadFromRawMeshData(RenderDog::g_pRDFbxImporter->GetRawMeshData(), fileName))
		{
			MessageBox(nullptr, "Load Model Failed!", "ERROR", MB_OK);
			return false;
		}

		m_pStaticModel->SetPosGesture(RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(1.0f));
	}
	else if (modelType == LOAD_MODEL_TYPE::CUSTOM_SKIN)
	{
		m_pSkinModel = new RenderDog::SkinModel();

		RenderDog::RDFbxImporter::FbxLoadParam fbxLoadParam;
		fbxLoadParam.bIsFlipTexcoordV = false;
		if (!RenderDog::g_pRDFbxImporter->LoadFbxFile(fileName, RenderDog::RDFbxImporter::FBX_LOAD_TYPE::SKIN_MODEL, fbxLoadParam))
		{
			MessageBox(nullptr, "Import FBX File Failed!", "ERROR", MB_OK);
			return false;
		}

		if (!m_pSkinModel->LoadFromRawMeshData(RenderDog::g_pRDFbxImporter->GetRawMeshData(),
											   RenderDog::g_pRDFbxImporter->GetRawSkeletonData(),
											   fileName))
		{
			MessageBox(nullptr, "Load Model Failed!", "ERROR", MB_OK);
			return false;
		}

		m_pSkinModel->SetPosGesture(RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(1.0f));
	}
	else
	{
		MessageBox(nullptr, "Load Unknown Type Model!", "ERROR", MB_OK);
		return false;
	}

	return true;
}

bool ModelViewer::LoadFbxAnimation(const std::string& fileName, RenderDog::SkinModel* pSkinModel)
{
	if (!pSkinModel)
	{
		MessageBox(nullptr, "Load FBX Animation: Unknown SkinModel Model!", "ERROR", MB_OK);
		return false;
	}

	if (!RenderDog::g_pRDFbxImporter->LoadFbxFile(fileName, RenderDog::RDFbxImporter::FBX_LOAD_TYPE::ANIMATION))
	{
		MessageBox(nullptr, "Import FBX Animation Failed!", "ERROR", MB_OK);

		return false;
	}

	if (!pSkinModel->LoadBoneAnimation(RenderDog::g_pRDFbxImporter->GetRawBoneAnimation()))
	{
		MessageBox(nullptr, "SkinModel Load Animation Failed!", "ERROR", MB_OK);

		return false;
	}

	return true;
}

RenderDog::IMaterial* ModelViewer::CreateBasicMaterial(const std::string& mtlName)
{
	RenderDog::IMaterial* pMtl = RenderDog::g_pMaterialManager->GetMaterial(mtlName);

	return pMtl;
}

RenderDog::IMaterial* ModelViewer::CreateSkyMaterial(const std::string& mtlName)
{
	RenderDog::IMaterial* pMtl = RenderDog::g_pMaterialManager->GetMaterial(mtlName);

	std::wstring diffuseTexturePath = L"EngineAsset/Textures/AlpsFieldCubeMap_d.dds";

	if (!diffuseTexturePath.empty())
	{
		RenderDog::ITexture2D* pDiffuseTexture = RenderDog::g_pITextureManager->GetTexture2D(diffuseTexturePath);
		if (!pDiffuseTexture)
		{
			return nullptr;
		}
		RenderDog::MaterialParam DiffuseTextureParam("ComVar_Texture_SkyCubeTexture", RenderDog::MATERIAL_PARAM_TYPE::TEXTURE2D);
		DiffuseTextureParam.SetTexture2D(pDiffuseTexture);
		pMtl->AddParam(DiffuseTextureParam);


		RenderDog::SamplerDesc samplerDesc = {};
		samplerDesc.name = "ComVar_Texture_SkyCubeTextureSampler";
		samplerDesc.filterMode = RenderDog::SAMPLER_FILTER::LINEAR;
		samplerDesc.addressMode = RenderDog::SAMPLER_ADDRESS::WRAP;
		RenderDog::ISamplerState* pDiffuseTextureSampler = RenderDog::g_pISamplerStateManager->GetSamplerState(samplerDesc);
		if (!pDiffuseTextureSampler)
		{
			return nullptr;
		}
		RenderDog::MaterialParam DiffuseTextureSamplerParam("ComVar_Texture_SkyCubeTextureSampler", RenderDog::MATERIAL_PARAM_TYPE::SAMPLER);
		DiffuseTextureSamplerParam.SetSamplerState(pDiffuseTextureSampler);
		pMtl->AddParam(DiffuseTextureSamplerParam);
	}

	return pMtl;
}

void ModelViewer::Update(float deltaTime)
{
	//W
	if (m_Keys[0x57])
	{
		m_pFPSCamera->Move(RenderDog::FPSCamera::MOVE_MODE::FRONT, deltaTime);
	}
	//S
	if (m_Keys[0x53])
	{
		m_pFPSCamera->Move(RenderDog::FPSCamera::MOVE_MODE::BACK, deltaTime);
	}
	//A
	if (m_Keys[0x41])
	{
		m_pFPSCamera->Move(RenderDog::FPSCamera::MOVE_MODE::LEFT, deltaTime);
	}
	//D
	if (m_Keys[0x44])
	{
		m_pFPSCamera->Move(RenderDog::FPSCamera::MOVE_MODE::RIGHT, deltaTime);
	}
	//Q
	if (m_Keys[0x51])
	{
		m_pFPSCamera->Move(RenderDog::FPSCamera::MOVE_MODE::UP, deltaTime);
	}
	//E
	if (m_Keys[0x45])
	{
		m_pFPSCamera->Move(RenderDog::FPSCamera::MOVE_MODE::DOWN, deltaTime);
	}

	if (m_bModelMoved && m_pStaticModel)
	{
		m_pStaticModel->SetPosGesture(RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(1.0f));
	}

	if (m_pSkinModel)
	{
		m_pSkinModel->Tick((float)(m_pGameTimer->GetDeltaTime()));
	}

	RegisterObjectToScene();
}

void ModelViewer::RegisterObjectToScene()
{
	m_pScene->Clear();

	if (m_pMainLight)
	{
		m_pMainLight->RegisterToScene(m_pScene);
	}

	if (m_bShowUnitGrid)
	{
		m_pGridLine->RegisterToScene(m_pScene);
	}

	if (m_pFloor)
	{
		m_pFloor->RegisterToScene(m_pScene);
	}
	
	if (m_pSkyBox)
	{
		m_pSkyBox->RegisterToScene(m_pScene);
	}

	if (m_pStaticModel)
	{
		m_pStaticModel->RegisterToScene(m_pScene);
	}
	
	if (m_pSkinModel)
	{
		m_pSkinModel->RegisterToScene(m_pScene);
	}
}

void ModelViewer::CalculateFrameStats()
{
	static int frameCnt = 0;
	static double timeElapsed = 0.0;

	++frameCnt;
	//当统计时间大于1s时，统计framecnt以计算fps
	if ((m_pGameTimer->GetTotalTime() - timeElapsed) >= 1.0f)
	{
		float framePerSecond = static_cast<float>(frameCnt);
		float millisecondsPerFrame = 1000.0f / framePerSecond;

		std::ostringstream outs;
		outs.precision(6);
		outs << "FPS: " << framePerSecond << "    "
			<< "Frame Time: " << millisecondsPerFrame << " (ms)";
		SetWindowTextA(RenderDog::g_pIWindow->GetHandle(), outs.str().c_str());

		frameCnt = 0;
		timeElapsed += 1.0;
	}
}

void ModelViewer::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePosX = x;
	m_LastMousePosY = y;

	SetCapture(RenderDog::g_pIWindow->GetHandle());
}

void ModelViewer::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void ModelViewer::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		float dx = RenderDog::AngleToRadians((float)(x - m_LastMousePosX));
		float dy = RenderDog::AngleToRadians((float)(y - m_LastMousePosY));

		m_pFPSCamera->Rotate(dx, -dy);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		float dx = (float)(x - m_LastMousePosX);
		float dy = (float)(y - m_LastMousePosY);

		RenderDog::Vector3 euler = m_pMainLight->GetEulerAngle();

		euler.y -= dx;
		euler.x -= dy;

		m_pMainLight->SetDirection(euler.x, euler.y, euler.z);
	}

	m_LastMousePosX = x;
	m_LastMousePosY = y;
}