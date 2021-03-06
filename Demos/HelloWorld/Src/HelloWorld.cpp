///////////////////////////////////
//RenderDog <??,??>
//FileName: HelloWorld.h
//Hello World Demo
//Written by Xiang Weikang
///////////////////////////////////

#include "HelloWorld.h"
#include "Window.h"
#include "Utility.h"
#include "GeometryGenerator.h"
#include "FbxImporter.h"

#include <windowsx.h>
#include <sstream>

DemoApp g_HelloWorldDemo;
DemoApp* g_pHelloWorldDemo = &g_HelloWorldDemo;

int DemoApp::m_Keys[512];

DemoApp::DemoApp():
	m_pRenderDog(nullptr),
	m_pScene(nullptr),
	m_pGridLine(nullptr),
	m_pModel(nullptr),
	m_pFPSCamera(nullptr),
	m_pMainLight(nullptr),
	m_pGameTimer(nullptr),
	m_LastMousePosX(0),
	m_LastMousePosY(0)
{
	memset(m_Keys, 0, sizeof(int) * 512);
}

DemoApp::~DemoApp()
{}

bool DemoApp::Init(const DemoInitDesc& desc)
{
	RenderDog::CameraDesc camDesc;
	camDesc.position = RenderDog::Vector3(0.0f, 50.0f, -500.0f);
	camDesc.yaw = 0.0f;
	camDesc.pitch = 0.0f;
	camDesc.fov = 45.0f;
	camDesc.aspectRitio = (float)desc.wndDesc.width / (float)desc.wndDesc.height;
	camDesc.nearPlane = 0.1f;
	camDesc.farPlane = 10000.0f;
	camDesc.moveSpeed = 0.5f;
	camDesc.rotSpeed = 0.1f;
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

	if (!LoadModel("Models/generator/Generator.FBX", LOAD_MODEL_TYPE::CUSTOM_STATIC))
	{
		MessageBox(nullptr, "Load Model Failed!", "ERROR", MB_OK);
		return false;
	}
	m_pModel->RegisterToScene(m_pScene);

	RenderDog::LightDesc lightDesc = {};
	lightDesc.type = RenderDog::LIGHT_TYPE::DIRECTIONAL;
	lightDesc.color = RenderDog::Vector3(1.0f, 1.0f, 1.0f);
	lightDesc.eulerDir = RenderDog::Vector3(45.0f, 45.0f, 45.0f);
	lightDesc.luminance = 0.8f;
	m_pMainLight = RenderDog::g_pILightManager->CreateLight(lightDesc);

	m_pMainLight->RegisterToScene(m_pScene);

	RenderDog::g_pIFramework->RegisterScene(m_pScene);

	m_pGameTimer = new RenderDog::GameTimer();
	m_pGameTimer->Reset();

	return true;
}

void DemoApp::Release()
{
	RenderDog::g_pRDFbxImporter->Release();

	if (m_pGridLine)
	{
		delete m_pGridLine;
		m_pGridLine = nullptr;
	}

	if (m_pModel)
	{
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
		//???????л?????״̬ʱ??????????δ??????͸???Ϣ
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
	//?????ڸı???Сʱ???͸???Ϣ
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

bool DemoApp::LoadModel(const std::string& fileName, LOAD_MODEL_TYPE modelType)
{
	m_pModel = new RenderDog::StaticModel();

	if (modelType == LOAD_MODEL_TYPE::STANDARD)
	{
		RenderDog::GeometryGenerator::StandardMeshData SphereMeshData;
		RenderDog::g_pGeometryGenerator->GenerateSphere(50, 50, 50, SphereMeshData);
		m_pModel->LoadFromStandardData(SphereMeshData.vertices, SphereMeshData.indices, "Shaders/StaticModelVertexShader.hlsl", "Shaders/PhongLightingPixelShader.hlsl", "Sphere");
		if (!m_pModel->LoadTextureFromFile(L"EngineAsset/Textures/ErrorTexture_diff.dds", L"EngineAsset/Textures/FlatNormal_norm.dds"))
		{
			MessageBox(nullptr, "Load Texture Failed!", "ERROR", MB_OK);
			return false;
		}
	}
	else if (modelType == LOAD_MODEL_TYPE::CUSTOM_STATIC)
	{
		RenderDog::RDFbxImporter::FbxLoadParam fbxLoadParam;
		fbxLoadParam.bIsFlipTexcoordV = true;
		if (!RenderDog::g_pRDFbxImporter->LoadFbxFile(fileName, RenderDog::RDFbxImporter::FBX_LOAD_TYPE::STATIC_MODEL, fbxLoadParam))
		{
			MessageBox(nullptr, "Import FBX File Failed!", "ERROR", MB_OK);
			return false;
		}

		if (!m_pModel->LoadFromRawMeshData(RenderDog::g_pRDFbxImporter->GetRawMeshData(), "Shaders/StaticModelVertexShader.hlsl", "Shaders/PhongLightingPixelShader.hlsl", fileName))
		{
			MessageBox(nullptr, "Load Model Failed!", "ERROR", MB_OK);
			return false;
		}

		if (!m_pModel->LoadTextureFromFile(L"EngineAsset/Textures/White_diff.dds", L"Models/generator/Textures/PolybumpTangent_DDN.tga"))
		{
			MessageBox(nullptr, "Load Texture Failed!", "ERROR", MB_OK);
			return false;
		}
	}
	else
	{
		MessageBox(nullptr, "Load Unknown Type Model!", "ERROR", MB_OK);
		return false;
	}

	m_pModel->SetPosGesture(RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(1.0f));

	return true;
}

void DemoApp::Update()
{
	//W
	if (m_Keys[0x57])
	{
		m_pFPSCamera->Move(RenderDog::FPSCamera::MOVE_MODE::FRONT);
	}
	//S
	if (m_Keys[0x53])
	{
		m_pFPSCamera->Move(RenderDog::FPSCamera::MOVE_MODE::BACK);
	}
	//A
	if (m_Keys[0x41])
	{
		m_pFPSCamera->Move(RenderDog::FPSCamera::MOVE_MODE::LEFT);
	}
	//D
	if (m_Keys[0x44])
	{
		m_pFPSCamera->Move(RenderDog::FPSCamera::MOVE_MODE::RIGHT);
	}
	//Q
	if (m_Keys[0x51])
	{
		m_pFPSCamera->Move(RenderDog::FPSCamera::MOVE_MODE::UP);
	}
	//E
	if (m_Keys[0x45])
	{
		m_pFPSCamera->Move(RenderDog::FPSCamera::MOVE_MODE::DOWN);
	}
}

void DemoApp::CalculateFrameStats()
{
	static int frameCnt = 0;
	static double timeElapsed = 0.0;

	++frameCnt;
	//??ͳ??ʱ??????1sʱ??ͳ??framecnt?Լ???fps
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

void DemoApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePosX = x;
	m_LastMousePosY = y;

	SetCapture(RenderDog::g_pIWindow->GetHandle());
}

void DemoApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void DemoApp::OnMouseMove(WPARAM btnState, int x, int y)
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