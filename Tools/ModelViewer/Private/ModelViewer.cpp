///////////////////////////////////////////
//ModelViewer
//FileName: ModelViewer.vpp
//Written by Xiang Weikang
///////////////////////////////////////////

#include "ModelViewer.h"
#include "GeometryGenerator.h"

ModelViewer g_ModelViewer;
ModelViewer* g_pModelViewer = &g_ModelViewer;

ModelViewer::ModelViewer() :
	m_pRenderDog(nullptr),
	m_pScene(nullptr),
	m_pModel(nullptr)
{}

ModelViewer::~ModelViewer()
{}

bool ModelViewer::Init(const RenderDog::InitDesc& desc)
{
	if (!RenderDog::CreateRenderDog(&m_pRenderDog))
	{
		return false;
	}

	if (!m_pRenderDog->Init(desc))
	{
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
	m_pModel->LoadFromData(boxMeshData.vertices, boxMeshData.indices);

	m_pModel->RegisterToScene(m_pScene);

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

		RenderDog::g_pIFramework->OnResize(newWidth, newHeight);

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
		break;
	}
	case WM_KEYUP:
	{
		break;
	}
	default:
	{
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	}
	return 0;
}