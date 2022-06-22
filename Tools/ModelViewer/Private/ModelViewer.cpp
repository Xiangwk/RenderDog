///////////////////////////////////////////
//ModelViewer
//FileName: ModelViewer.vpp
//Written by Xiang Weikang
///////////////////////////////////////////

#include "ModelViewer.h"
#include "Vector.h"
#include "GeometryGenerator.h"
#include "Utility.h"

#include <windowsx.h>
#include <sstream>

ModelViewer g_ModelViewer;
ModelViewer* g_pModelViewer = &g_ModelViewer;

int ModelViewer::m_Keys[512];

ModelViewer::ModelViewer() :
	m_pRenderDog(nullptr),
	m_pScene(nullptr),
	m_pGridLine(nullptr),
	m_pFloor(nullptr),
	m_pModel(nullptr),
	m_pFPSCamera(nullptr),
	m_pMainLight(nullptr),
	m_pGameTimer(nullptr),
	m_LastMousePosX(0),
	m_LastMousePosY(0)
{
	memset(m_Keys, 0, sizeof(int) * 512);
}

ModelViewer::~ModelViewer()
{}

bool ModelViewer::Init(const ModelViewerInitDesc& desc)
{
	RenderDog::CameraDesc camDesc;
	camDesc.position = RenderDog::Vector3(0.0f, 2.0f, -20.0f);
	camDesc.yaw = 0.0f;
	camDesc.pitch = 0.0f;
	camDesc.fov = 45.0f;
	camDesc.aspectRitio = (float)desc.wndDesc.width / (float)desc.wndDesc.height;
	camDesc.nearPlane = 0.1f;
	camDesc.farPlane = 1000.0f;
	camDesc.moveSpeed = 0.002f;
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

	RenderDog::SceneInitDesc sceneDesc;
	sceneDesc.name = "MainScene";
	m_pScene = RenderDog::g_pISceneManager->CreateScene(sceneDesc);
	if (!m_pScene)
	{
		MessageBox(nullptr, "Create Scene Failed!", "ERROR", MB_OK);
		return false;
	}

	RenderDog::GeometryGenerator::SimpleMeshData GridLineMeshData;
	RenderDog::g_pGeometryGenerator->GenerateGridLine(100, 100, 1, RenderDog::Vector4(0.8f, 0.8f, 0.8f, 1.0f), GridLineMeshData);
	m_pGridLine = new RenderDog::SimpleModel();
	m_pGridLine->LoadFromSimpleData(GridLineMeshData.vertices, GridLineMeshData.indices, "Shaders/SimpleModelVertexShader.hlsl", "Shaders/SingleColorPixelShader.hlsl", "MainSceneGridLine");
	m_pGridLine->SetPosGesture(RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(1.0f));
	m_pGridLine->RegisterToScene(m_pScene);

	RenderDog::GeometryGenerator::StandardMeshData GridMeshData;
	RenderDog::g_pGeometryGenerator->GenerateGrid(100, 100, 1, GridMeshData);
	m_pFloor = new RenderDog::StaticModel();
	m_pFloor->LoadFromStandardData(GridMeshData.vertices, GridMeshData.indices, "Shaders/StaticModelVertexShader.hlsl", "Shaders/PhongLightingPixelShader.hlsl", "MainSceneFloor");
	m_pFloor->LoadTextureFromFile(L"Textures/FlatNormal_norm.dds");
	m_pFloor->SetPosGesture(RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(1.0f));
	m_pFloor->RegisterToScene(m_pScene);

	m_pModel = new RenderDog::StaticModel();
	m_pModel->LoadFromFile("Models/generator/generator_small.obj", "Shaders/StaticModelVertexShader.hlsl", "Shaders/PhongLightingPixelShader.hlsl");
	m_pModel->LoadTextureFromFile(L"Textures/PolybumpTangent_DDN.tga");
	m_pModel->SetPosGesture(RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(90.0f, 0.0f, 0.0f), RenderDog::Vector3(0.1f));
	m_pModel->RegisterToScene(m_pScene);
	/*RenderDog::GeometryGenerator::StandardMeshData BoxMeshData;
	RenderDog::g_pGeometryGenerator->GenerateBox(10, 10, 10, BoxMeshData);
	m_pModel->LoadFromStandardData(BoxMeshData.vertices, BoxMeshData.indices, "Shaders/StaticModelVertexShader.hlsl", "Shaders/PhongLightingPixelShader.hlsl", "Box");
	m_pModel->LoadTextureFromFile(L"Textures/PolybumpTangent_DDN.tga");
	m_pModel->SetPosGesture(RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(0.0f, 0.0f, 0.0f), RenderDog::Vector3(1.0f));
	m_pModel->RegisterToScene(m_pScene);*/

	RenderDog::LightDesc lightDesc = {};
	lightDesc.type = RenderDog::LIGHT_TYPE::DIRECTIONAL;
	lightDesc.color = RenderDog::Vector3(1.0f, 1.0f, 1.0f);
	lightDesc.eulerDir = RenderDog::Vector3(45.0f, 45.0f, 0.0f);
	lightDesc.luminance = 0.8f;
	m_pMainLight = RenderDog::g_pILightManager->CreateLight(lightDesc);

	m_pMainLight->RegisterToScene(m_pScene);

	RenderDog::g_pIFramework->RegisterScene(m_pScene);

	m_pGameTimer = new RenderDog::GameTimer();
	m_pGameTimer->Reset();

	return true;
}

void ModelViewer::Release()
{
	if (m_pGridLine)
	{
		m_pGridLine->ReleaseRenderData();

		delete m_pGridLine;
		m_pGridLine = nullptr;
	}

	if (m_pFloor)
	{
		m_pFloor->ReleaseRenderData();

		delete m_pFloor;
		m_pFloor = nullptr;
	}

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

void ModelViewer::Update()
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

		float speed = 1.0f;
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