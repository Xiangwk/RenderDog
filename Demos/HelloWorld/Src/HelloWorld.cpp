///////////////////////////////////
//RenderDog <��,��>
//FileName: HelloWorld.h
//Hello World Demo
//Written by Xiang Weikang
///////////////////////////////////

#include "HelloWorld.h"
#include "Window.h"

#include <sstream>

DemoApp g_HelloWorldDemo;
DemoApp* g_pHelloWorldDemo = &g_HelloWorldDemo;

int DemoApp::m_Keys[512];

DemoApp::DemoApp():
	m_pRenderDog(nullptr),
	m_pScene(nullptr),
	m_pModel(nullptr),
	m_pFPSCamera(nullptr),
	m_pMainLight(nullptr),
	m_pGameTimer(nullptr)
{
	memset(m_Keys, 0, sizeof(int) * 512);
}

DemoApp::~DemoApp()
{}

bool DemoApp::Init(const DemoInitDesc& desc)
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

	m_pModel = new RenderDog::StaticModel();
	m_pModel->LoadFromFile("Models/generator/generator_small.obj");
	//m_pModel->LoadTextureFromFile(L"EngineAsset/Textures/awesomeface.dds");
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

	m_pGameTimer = new RenderDog::GameTimer();
	m_pGameTimer->Reset();

	return true;
}

void DemoApp::Release()
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

	if (m_pGameTimer)
	{
		delete m_pGameTimer;
		m_pGameTimer = nullptr;
	}
}

int	DemoApp::Run()
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

			Update();

			RenderDog::g_pIFramework->Frame();
		}
	}

	return (int)Msg.wParam;
}

LRESULT DemoApp::MessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		//�������л�����״̬ʱ���������δ������͸���Ϣ
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
	//�����ڸı��Сʱ���͸���Ϣ
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

void DemoApp::Update()
{
	float cameraSpeed = 0.1f;
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

void DemoApp::CalculateFrameStats()
{
	static int frameCnt = 0;
	static double timeElapsed = 0.0;

	++frameCnt;
	//��ͳ��ʱ�����1sʱ��ͳ��framecnt�Լ���fps
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