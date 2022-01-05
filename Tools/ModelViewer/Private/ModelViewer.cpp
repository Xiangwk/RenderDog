///////////////////////////////////////////
//ModelViewer
//FileName: ModelViewer.vpp
//Written by Xiang Weikang
///////////////////////////////////////////

#include "ModelViewer.h"

ModelViewer g_ModelViewer;
ModelViewer* g_pModelViewer = &g_ModelViewer;

ModelViewer::ModelViewer() :
	m_pRenderDog(nullptr)
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

	return true;
}

void ModelViewer::Release()
{
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