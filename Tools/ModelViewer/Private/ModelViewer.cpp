///////////////////////////////////////////
//ModelViewer
//FileName: ModelViewer.vpp
//Written by Xiang Weikang
///////////////////////////////////////////

#include "ModelViewer.h"
#include "Vector.h"
#include "GeometryGenerator.h"

ModelViewer g_ModelViewer;
ModelViewer* g_pModelViewer = &g_ModelViewer;

int ModelViewer::m_Keys[512];

ModelViewer::ModelViewer() :
	m_pRenderDog(nullptr),
	m_pScene(nullptr),
	m_pModel(nullptr),
	m_pFPSCamera(nullptr),
	m_pMainLight(nullptr)
{
	memset(m_Keys, 0, sizeof(int) * 512);
}

ModelViewer::~ModelViewer()
{}

bool ModelViewer::Init(const ModelViewerInitDesc& desc)
{
	RenderDog::CameraDesc camDesc;
	camDesc.position = RenderDog::Vector3(0.0f, 0.0f, -100.0f);
	camDesc.direction = RenderDog::Vector3(0.0f, 0.0f, 1.0f);
	camDesc.fov = 45.0f;
	camDesc.aspectRitio = (float)desc.wndDesc.width / (float)desc.wndDesc.height;
	camDesc.nearPlane = 0.1f;
	camDesc.farPlane = 1000.0f;
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

	RenderDog::SceneInitDesc sceneDesc;
	sceneDesc.name = "MainScene";
	m_pScene = RenderDog::g_pISceneManager->CreateScene(sceneDesc);
	if (!m_pScene)
	{
		MessageBox(nullptr, "Create Scene Failed!", "ERROR", MB_OK);
		return false;
	}

	GeometryGenerator::LocalMeshData boxMeshData;
	g_pGeometryGenerator->GenerateBox(1, 1, 1, boxMeshData);

	m_pModel = new RenderDog::StaticModel();
	//m_pModel->LoadFromData(boxMeshData.vertices, boxMeshData.indices);
	m_pModel->LoadFromFile("Models/generator/generator_small.obj");
	//m_pModel->LoadTextureFromFile(L"EngineAsset/Textures/Brick_norm.dds");
	m_pModel->LoadTextureFromFile(L"Textures/PolybumpTangent_DDN.tga");
	m_pModel->SetPosGesture(RenderDog::Vector3(0.0f, -25.0f, 0.0f), RenderDog::Vector3(90.0f, 0.0f, 0.0f), RenderDog::Vector3(1.0f));

	m_pModel->RegisterToScene(m_pScene);

	RenderDog::LightDesc lightDesc = {};
	lightDesc.type = RenderDog::LightType::RD_LIGHT_TYPE_DIRECTIONAL;
	lightDesc.color = RenderDog::Vector3(1.0f, 1.0f, 1.0f);
	lightDesc.eulerDir = RenderDog::Vector3(45.0f, 45.0f, 45.0f);
	lightDesc.luminance = 1.0f;
	m_pMainLight = RenderDog::g_pILightManager->CreateLight(lightDesc);

	m_pMainLight->RegisterToScene(m_pScene);

	RenderDog::g_pIFramework->RegisterScene(m_pScene);

	return true;
}

void ModelViewer::Release()
{
	if (m_pModel)
	{
		m_pModel->ReleaseRenderData();

		delete m_pModel;
		m_pModel = nullptr;
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

	m_pScene->Release();

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
			Update();
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
		break;
	}
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	{
		break;
	}
	case WM_MOUSEMOVE:
	{
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

void ModelViewer::Update()
{
	float cameraSpeed = 0.01f;
	//W
	if (m_Keys[0x57])
	{
		m_pFPSCamera->Move(cameraSpeed, RenderDog::FPSCamera::MoveMode::FrontAndBack);
	}
	//S
	if (m_Keys[0x53])
	{
		m_pFPSCamera->Move(-cameraSpeed, RenderDog::FPSCamera::MoveMode::FrontAndBack);
	}
	//A
	if (m_Keys[0x41])
	{
		m_pFPSCamera->Move(-cameraSpeed, RenderDog::FPSCamera::MoveMode::LeftAndRight);
	}
	//D
	if (m_Keys[0x44])
	{
		m_pFPSCamera->Move(cameraSpeed, RenderDog::FPSCamera::MoveMode::LeftAndRight);
	}
	//Q
	if (m_Keys[0x51])
	{
		m_pFPSCamera->Move(cameraSpeed, RenderDog::FPSCamera::MoveMode::UpAndDown);
	}
	//E
	if (m_Keys[0x45])
	{
		m_pFPSCamera->Move(-cameraSpeed, RenderDog::FPSCamera::MoveMode::UpAndDown);
	}
}