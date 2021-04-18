///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Game.cpp
//Hello World Demo, Clear RenderTarget with Red Color
//Written by Xiang Weikang
///////////////////////////////////

#include "Vector.h"
#include "Device.h"

#include <windows.h>
#include <string>

HINSTANCE					g_hInst = nullptr;
HWND						g_hWnd = nullptr;
uint32_t**					g_pRenderTarget = nullptr;
uint32_t					g_nWindowWidth = 1024;
uint32_t					g_nWindowHeight = 768;
							
RenderDog::Device*			g_pDevice = nullptr;
RenderDog::DeviceContext*	g_pDeviceContext = nullptr;
RenderDog::SwapChain*		g_pSwapChain = nullptr;

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
bool InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
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

	if (!RenderDog::CreateDeviceAndSwapChain(&g_pDevice, &g_pDeviceContext, &g_pSwapChain, swapChainDesc))
	{
		return false;
	}

	RenderDog::RenderTargetDesc rtDesc;
	rtDesc.width = g_nWindowWidth;
	rtDesc.height = g_nWindowHeight;
	rtDesc.pFrameBuf = (void*)g_pSwapChain->GetBackBuffer();
	g_pDevice->CreateRenderTarget(rtDesc, g_pRenderTarget);

	return true;
}

void CleanupDevice()
{
	g_pSwapChain->Release();
	RenderDog::ReleaseDevice(g_pDevice, g_pDeviceContext);
}

void Render()
{
	g_pDeviceContext->OMSetRenderTarget(*g_pRenderTarget);
	float ClearColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	g_pDeviceContext->ClearRenderTarget(g_pRenderTarget, ClearColor);

	g_pDeviceContext->Draw();

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
			Render();
		}
	}

	CleanupDevice();

	return (int)msg.wParam;
}