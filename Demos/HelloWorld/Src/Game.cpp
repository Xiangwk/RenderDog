///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Game.cpp
//Hello World Demo, Clear RenderTarget with Red Color
//Written by Xiang Weikang
///////////////////////////////////

#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "Device.h"
#include "RenderTargetView.h"
#include "Texture.h"
#include "Buffer.h"
#include "Shader.h"
#include "Vertex.h"
#include "Transform.h"
#include "Viewport.h"

#include <windows.h>
#include <string>

HINSTANCE						g_hInst = nullptr;
HWND							g_hWnd = nullptr;
uint32_t						g_nWindowWidth = 1024;
uint32_t						g_nWindowHeight = 768;
								
RenderDog::Device*				g_pDevice = nullptr;
RenderDog::DeviceContext*		g_pDeviceContext = nullptr;
RenderDog::SwapChain*			g_pSwapChain = nullptr;
RenderDog::RenderTargetView*	g_pRenderTargetView = nullptr;
RenderDog::VertexBuffer*		g_pVertexBuffer = nullptr;
RenderDog::IndexBuffer*			g_pIndexBuffer = nullptr;
RenderDog::VertexShader*		g_pVertexShader = nullptr;
RenderDog::PixelShader*			g_pPixelShader = nullptr;

RenderDog::Matrix4x4			g_WorldMatrix;
RenderDog::Matrix4x4			g_ViewMatrix;
RenderDog::Matrix4x4			g_PerspProjMatrix;

using RenderDog::Vector3;
using RenderDog::Vertex;

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
bool InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void Update(float fTime);
void Render();

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = nullptr;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"GameWindowClass";
	wcex.hIconSm = nullptr;
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, (LONG)g_nWindowWidth, (LONG)g_nWindowHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(L"GameWindowClass", L"Hello World",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL);
	if (!g_hWnd)
	{
		return E_FAIL;
	}

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}

bool InitDevice()
{
	RenderDog::SwapChainDesc swapChainDesc;
	swapChainDesc.nWidth = g_nWindowWidth;
	swapChainDesc.nHeight = g_nWindowHeight;
	swapChainDesc.hOutputWindow = g_hWnd;

	if (!RenderDog::CreateDeviceAndSwapChain(&g_pDevice, &g_pDeviceContext, &g_pSwapChain, &swapChainDesc))
	{
		return false;
	}

	RenderDog::Texture2D* pBackBuffer = nullptr;
	if (!g_pSwapChain->GetBuffer(&pBackBuffer))
	{
		return false;
	}

	if (!g_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView))
	{
		return false;
	}
	delete pBackBuffer;
	pBackBuffer = nullptr;

	RenderDog::Viewport vp;
	vp.fWidth = (float)g_nWindowWidth;
	vp.fHeight = (float)g_nWindowHeight;
	vp.fMinDepth = 0.0f;
	vp.fMaxDepth = 1.0f;
	vp.fTopLeftX = 0;
	vp.fTopLeftY = 0;
	g_pDeviceContext->RSSetViewport(&vp);

	if (!g_pDevice->CreateVertexShader(&g_pVertexShader))
	{
		return false;
	}

	if (!g_pDevice->CreatePixelShader(&g_pPixelShader))
	{
		return false;
	}

	Vertex aBoxVertices[] =
	{
		{Vector3(-1.0f, 1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(1.0f, 1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(1.0f, 1.0f, 1.0f),		Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(-1.0f, 1.0f, 1.0f),	Vector3(1.0f, 1.0f, 1.0f)},

		{Vector3(-1.0f, -1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(1.0f, -1.0f, -1.0f) ,	Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(1.0f, -1.0f, 1.0f),	Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(-1.0f, -1.0f, 1.0f) ,	Vector3(1.0f, 1.0f, 1.0f)},

		{Vector3(-1.0f, -1.0f, 1.0f),	Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(-1.0f, -1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(-1.0f, 1.0f, -1.0f), 	Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(-1.0f, 1.0f, 1.0f),	Vector3(1.0f, 1.0f, 1.0f)},

		{Vector3(1.0f, -1.0f, 1.0f),	Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(1.0f, -1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(1.0f, 1.0f, -1.0f), 	Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(1.0f, 1.0f, 1.0f),		Vector3(1.0f, 1.0f, 1.0f)},

		{Vector3(-1.0f, -1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(1.0f, -1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(1.0f, 1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(-1.0f, 1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f)},

		{Vector3(-1.0f, -1.0f, 1.0f),	Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(1.0f, -1.0f, 1.0f),	Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(1.0f, 1.0f, 1.0f),		Vector3(1.0f, 1.0f, 1.0f)},
		{Vector3(-1.0f, 1.0f, 1.0f),	Vector3(1.0f, 1.0f, 1.0f)},
	};

	RenderDog::VertexBufferDesc vbDesc;
	vbDesc.nVertexNum = sizeof(aBoxVertices) / sizeof(Vertex);
	vbDesc.pInitData = aBoxVertices;
	if (!g_pDevice->CreateVertexBuffer(vbDesc, &g_pVertexBuffer))
	{
		return false;
	}

	uint32_t aBoxIndices[] =
	{
		3,1,0,
		2,1,3,

		6,4,5,
		7,4,6,

		11,9,8,
		10,9,11,

		14,12,13,
		15,12,14,

		19,17,16,
		18,17,19,

		22,20,21,
		23,20,22
	};

	RenderDog::IndexBufferDesc ibDesc;
	ibDesc.nIndexNum = sizeof(aBoxIndices) / sizeof(uint32_t);
	ibDesc.pInitData = aBoxIndices;
	if (!g_pDevice->CreateIndexBuffer(ibDesc, &g_pIndexBuffer))
	{
		return false;
	}

	g_WorldMatrix = RenderDog::GetIdentityMatrix();
	g_ViewMatrix = RenderDog::GetLookAtMatrixLH(Vector3(0.0f, 0.0f, -10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
	g_PerspProjMatrix = RenderDog::GetPerspProjectionMatrixLH(45.0f, (float)g_nWindowWidth / g_nWindowHeight, 0.01f, 1000.0f);

	return true;
}

void CleanupDevice()
{
	if (g_pSwapChain)
	{
		g_pSwapChain->Release();
		delete g_pSwapChain;
		g_pSwapChain = nullptr;
	}
	
	
	if (g_pRenderTargetView)
	{
		delete g_pRenderTargetView;
		g_pRenderTargetView = nullptr;
	}

	if (g_pVertexShader)
	{
		delete g_pVertexShader;
		g_pVertexShader = nullptr;
	}

	if (g_pPixelShader)
	{
		delete g_pPixelShader;
		g_pPixelShader = nullptr;
	}

	if (g_pVertexBuffer)
	{
		g_pVertexBuffer->Release();
		delete g_pVertexBuffer;
		g_pVertexBuffer = nullptr;
	}

	if (g_pIndexBuffer)
	{
		g_pIndexBuffer->Release();
		delete g_pIndexBuffer;
		g_pIndexBuffer = nullptr;
	}

	if (g_pDevice)
	{
		delete g_pDevice;
		g_pDevice = nullptr;
	}

	if (g_pDeviceContext)
	{
		delete g_pDeviceContext;
		g_pDeviceContext = nullptr;
	}
}

void Update(float fTime)
{
	g_WorldMatrix = GetRotationMatrix(fTime, Vector3(1.0f, 1.0f, 1.0f));
}

void Render()
{
	g_pDeviceContext->OMSetRenderTarget(g_pRenderTargetView);
	float ClearColor[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
	g_pDeviceContext->ClearRenderTarget(g_pRenderTargetView, ClearColor);

	g_pDeviceContext->IASetVertexBuffer(g_pVertexBuffer);
	g_pDeviceContext->IASetIndexBuffer(g_pIndexBuffer);

	g_pDeviceContext->VSSetShader(g_pVertexShader);
	g_pDeviceContext->VSSetTransMats(&g_WorldMatrix, &g_ViewMatrix, &g_PerspProjMatrix);
	g_pDeviceContext->PSSetShader(g_pPixelShader);

	g_pDeviceContext->DrawIndex(36);

	g_pSwapChain->Present();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
		return 0;
	}

	if (!InitDevice())
	{
		return 0;
	}

	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			static float fTime = 0.0f;
			Update(fTime);
			Render();
			fTime += 0.1f;
		}
	}

	CleanupDevice();

	_CrtDumpMemoryLeaks();

	return (int)msg.wParam;
}