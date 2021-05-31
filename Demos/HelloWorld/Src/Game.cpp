///////////////////////////////////
//RenderDog <·,·>
//FileName: Game.cpp
//Hello World Demo, Clear RenderTarget with Red Color
//Written by Xiang Weikang
///////////////////////////////////

#include "RenderDog.h"
#include "RenderTargetView.h" 
#include "DepthStencilView.h"
#include "ShaderResourceView.h"
#include "Texture.h"
#include "Buffer.h"
#include "Shader.h"
#include "Vertex.h"
#include "Transform.h"
#include "Viewport.h"
#include "Camera.h"
#include "Model.h"

#include <windowsx.h>
#include <string>

#include <vld.h>

using RenderDog::Vector2;
using RenderDog::Vector3;
using RenderDog::Vertex;

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
RenderDog::Texture2D*			g_pDepthTexture = nullptr;
RenderDog::DepthStencilView*	g_pDepthStencilView = nullptr;
RenderDog::ShaderResourceView*	g_pTextureSRV = nullptr;

RenderDog::Matrix4x4			g_WorldMatrix;
RenderDog::Matrix4x4			g_ViewMatrix;
RenderDog::Matrix4x4			g_PerspProjMatrix;

Vector2							g_LastMousePos;

RenderDog::StaticModel*			g_pStaticModel;


int aKeys[512];	// 当前键盘按下状态

RenderDog::FPSCamera*			g_pMainCamera;

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
bool InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void Update(float fTime);
void Render();

void    OnMouseDown(WPARAM btnState, int x, int y);
void    OnMouseUp(WPARAM btnState, int x, int y);
void    OnMouseMove(WPARAM btnState, int x, int y);
void    OnMouseWheelMove(WPARAM btnState);

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

	RenderDog::Texture2DDesc depthDesc;
	depthDesc.width = g_nWindowWidth;
	depthDesc.height = g_nWindowWidth;
	depthDesc.format = RenderDog::TextureFormat::TF_FLOAT32;
	if (!g_pDevice->CreateTexture2D(&depthDesc, &g_pDepthTexture))
	{
		return false;
	}

	if (!g_pDevice->CreateDepthStencilView(g_pDepthTexture, &g_pDepthStencilView))
	{
		return false;
	}

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

	g_pStaticModel = new RenderDog::StaticModel;
	g_pStaticModel->LoadFromFile("Models/generator/generator_small.obj");
	if (!g_pStaticModel->Init(g_pDevice))
	{
		return false;
	}

	/*Vertex aBoxVertices[] =
	{
		{Vector3(-1.0f, 1.0f, -1.0f),	Vector3(1.0f, 0.0f, 0.0f),	Vector2(0.0f, 1.0f)},
		{Vector3(1.0f, 1.0f, -1.0f),	Vector3(1.0f, 0.0f, 0.0f),	Vector2(1.0f, 1.0f)},
		{Vector3(1.0f, 1.0f, 1.0f),		Vector3(1.0f, 0.0f, 0.0f),	Vector2(1.0f, 0.0f)},
		{Vector3(-1.0f, 1.0f, 1.0f),	Vector3(1.0f, 0.0f, 0.0f),	Vector2(0.0f, 0.0f)},

		{Vector3(-1.0f, -1.0f, -1.0f),	Vector3(0.0f, 1.0f, 0.0f),	Vector2(0.0f, 1.0f)},
		{Vector3(1.0f, -1.0f, -1.0f) ,	Vector3(0.0f, 1.0f, 0.0f),	Vector2(1.0f, 1.0f)},
		{Vector3(1.0f, -1.0f, 1.0f),	Vector3(0.0f, 1.0f, 0.0f),	Vector2(1.0f, 0.0f)},
		{Vector3(-1.0f, -1.0f, 1.0f) ,	Vector3(0.0f, 1.0f, 0.0f),	Vector2(0.0f, 0.0f)},

		{Vector3(-1.0f, -1.0f, 1.0f),	Vector3(0.0f, 0.0f, 1.0f),	Vector2(0.0f, 1.0f)},
		{Vector3(-1.0f, -1.0f, -1.0f),	Vector3(0.0f, 0.0f, 1.0f),	Vector2(1.0f, 1.0f)},
		{Vector3(-1.0f, 1.0f, -1.0f), 	Vector3(0.0f, 0.0f, 1.0f),	Vector2(1.0f, 0.0f)},
		{Vector3(-1.0f, 1.0f, 1.0f),	Vector3(0.0f, 0.0f, 1.0f),	Vector2(0.0f, 0.0f)},

		{Vector3(1.0f, -1.0f, 1.0f),	Vector3(1.0f, 1.0f, 0.0f),	Vector2(0.0f, 1.0f)},
		{Vector3(1.0f, -1.0f, -1.0f),	Vector3(1.0f, 1.0f, 0.0f),	Vector2(1.0f, 1.0f)},
		{Vector3(1.0f, 1.0f, -1.0f), 	Vector3(1.0f, 1.0f, 0.0f),	Vector2(1.0f, 0.0f)},
		{Vector3(1.0f, 1.0f, 1.0f),		Vector3(1.0f, 1.0f, 0.0f),	Vector2(0.0f, 0.0f)},

		{Vector3(-1.0f, -1.0f, -1.0f),	Vector3(1.0f, 0.0f, 1.0f),	Vector2(0.0f, 1.0f)},
		{Vector3(1.0f, -1.0f, -1.0f),	Vector3(1.0f, 0.0f, 1.0f),	Vector2(1.0f, 1.0f)},
		{Vector3(1.0f, 1.0f, -1.0f),	Vector3(1.0f, 0.0f, 1.0f),	Vector2(1.0f, 0.0f)},
		{Vector3(-1.0f, 1.0f, -1.0f),	Vector3(1.0f, 0.0f, 1.0f),	Vector2(0.0f, 0.0f)},

		{Vector3(-1.0f, -1.0f, 1.0f),	Vector3(0.0f, 1.0f, 1.0f),	Vector2(0.0f, 1.0f)},
		{Vector3(1.0f, -1.0f, 1.0f),	Vector3(0.0f, 1.0f, 1.0f),	Vector2(1.0f, 1.0f)},
		{Vector3(1.0f, 1.0f, 1.0f),		Vector3(0.0f, 1.0f, 1.0f),	Vector2(1.0f, 0.0f)},
		{Vector3(-1.0f, 1.0f, 1.0f),	Vector3(0.0f, 1.0f, 1.0f),	Vector2(0.0f, 0.0f)},
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
	}*/

	g_pTextureSRV = new RenderDog::ShaderResourceView();

	if (!g_pTextureSRV->LoadFromFile("Textures/ErrorTexture_diff.tga"))
	{
		return false;
	}

	RenderDog::CameraDesc camDesc;
	camDesc.vPosition = Vector3(0, 0, -100);
	camDesc.vDirection = Vector3(0, 0, 1);
	camDesc.fFov = 45.0f;
	camDesc.fAspect = (float)g_nWindowWidth / g_nWindowHeight;
	camDesc.fNear = 0.1f;
	camDesc.fFar = 1000.0f;

	g_pMainCamera = new RenderDog::FPSCamera(camDesc);

	g_WorldMatrix = RenderDog::GetRotationMatrix(90.0f, Vector3(1.0f, 0.0f, 0.0f));
	g_ViewMatrix = g_pMainCamera->GetViewMatrix();
	g_PerspProjMatrix = g_pMainCamera->GetPerspProjectionMatrix();

	memset(aKeys, 0, sizeof(int) * 512);

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

	if (g_pStaticModel)
	{
		g_pStaticModel->Release();

		delete g_pStaticModel;
		g_pStaticModel = nullptr;
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

	if (g_pDepthTexture)
	{
		g_pDepthTexture->Release();

		delete g_pDepthTexture;
		g_pDepthTexture = nullptr;
	}

	if (g_pDepthStencilView)
	{
		delete g_pDepthStencilView;
		g_pDepthStencilView = nullptr;
	}

	if (g_pTextureSRV)
	{
		g_pTextureSRV->Release();

		delete g_pTextureSRV;
		g_pTextureSRV = nullptr;
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

	if (g_pMainCamera)
	{
		delete g_pMainCamera;
		g_pMainCamera = nullptr;
	}
}

void Update(float fTime)
{
	float fSpeed = 0.1f;
	//W
	if (aKeys[0x57])
	{
		g_pMainCamera->Move(fSpeed, RenderDog::FPSCamera::MoveMode::FrontAndBack);
	}
	//S
	if (aKeys[0x53])
	{
		g_pMainCamera->Move(-fSpeed, RenderDog::FPSCamera::MoveMode::FrontAndBack);
	}
	//A
	if (aKeys[0x41])
	{
		g_pMainCamera->Move(-fSpeed, RenderDog::FPSCamera::MoveMode::LeftAndRight);
	}
	//D
	if (aKeys[0x44])
	{
		g_pMainCamera->Move(fSpeed, RenderDog::FPSCamera::MoveMode::LeftAndRight);
	}
	//Q
	if (aKeys[0x51])
	{
		g_pMainCamera->Move(fSpeed, RenderDog::FPSCamera::MoveMode::UpAndDown);
	}
	//E
	if (aKeys[0x45])
	{
		g_pMainCamera->Move(-fSpeed, RenderDog::FPSCamera::MoveMode::UpAndDown);
	}
	
	g_ViewMatrix = g_pMainCamera->GetViewMatrix();
}

void Render()
{
	g_pDeviceContext->OMSetRenderTarget(g_pRenderTargetView, g_pDepthStencilView);
	float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	g_pDeviceContext->ClearRenderTarget(g_pRenderTargetView, ClearColor);

	g_pDeviceContext->ClearDepthStencil(g_pDepthStencilView, 1.0f);

	/*g_pDeviceContext->IASetVertexBuffer(g_pVertexBuffer);
	g_pDeviceContext->IASetIndexBuffer(g_pIndexBuffer);*/
	g_pDeviceContext->IASetPrimitiveTopology(RenderDog::PrimitiveTopology::TRIANGLE_LIST);

	g_pDeviceContext->VSSetShader(g_pVertexShader);
	g_pDeviceContext->VSSetTransMats(&g_WorldMatrix, &g_ViewMatrix, &g_PerspProjMatrix);
	g_pDeviceContext->PSSetShader(g_pPixelShader);
	g_pDeviceContext->PSSetShaderResource(&g_pTextureSRV);

	//g_pDeviceContext->DrawIndex(36);

	g_pStaticModel->Draw(g_pDeviceContext);

#if DEBUG_RASTERIZATION
	if (g_pDeviceContext->CheckDrawPixelTiwce())
	{
		return;
	}
#endif

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

	case WM_KEYDOWN: 
		aKeys[wParam & 511] = 1; 
		break;
	case WM_KEYUP: 
		aKeys[wParam & 511] = 0; 
		break;
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
		OnMouseWheelMove(wParam);
		break;
	}

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
			fTime += 0.5f;
		}
	}

	CleanupDevice();

	return (int)msg.wParam;
}

void OnMouseDown(WPARAM btnState, int x, int y)
{
	g_LastMousePos.x = (float)x;
	g_LastMousePos.y = (float)y;

	SetCapture(g_hWnd);
}

void OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void OnMouseMove(WPARAM btnState, int x, int y)
{
	float fSpeed = 0.05f;

	if ((btnState & MK_LBUTTON) != 0)
	{
		float dx = x - g_LastMousePos.x;
		float dy = y - g_LastMousePos.y;

		g_pMainCamera->Rotate(dx, dy, fSpeed);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		//TODO: Light Dir Change
	}

	g_LastMousePos.x = (float)x;
	g_LastMousePos.y = (float)y;
}

void OnMouseWheelMove(WPARAM btnState)
{
	return;
}