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
#include "Buffer.h"
#include "Shader.h"
#include "Vertex.h"
#include "Transform.h"
#include "Viewport.h"
#include "Camera.h"
#include "Model.h"
#include "Light.h"

#include <windowsx.h>
#include <string>
#include <unordered_map>

#include <vld.h>

#define DRAW_BOX 0

using RenderDog::Vector2;
using RenderDog::Vector3;
using RenderDog::Vector4;
using RenderDog::Vertex;

HINSTANCE						g_hInst = nullptr;
HWND							g_WndHandle = nullptr;
uint32_t						g_WindowWidth = 1024;
uint32_t						g_WindowHeight = 768;
								
RenderDog::IDevice*				g_pDevice = nullptr;
RenderDog::IDeviceContext*		g_pDeviceContext = nullptr;
RenderDog::ISwapChain*			g_pSwapChain = nullptr;
RenderDog::RenderTargetView*	g_pRenderTargetView = nullptr;
RenderDog::VertexBuffer*		g_pVertexBuffer = nullptr;			//用于自定义的顶点数组
RenderDog::IndexBuffer*			g_pIndexBuffer = nullptr;			//用于自定义的索引数组
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

RenderDog::DirectionalLight*	g_pMainLight;


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
void	CalculateTangents(const std::vector<Vertex>& RawVertices, const std::vector<uint32_t>& RawIndices, std::vector<Vertex>& OutputVertices, std::vector<uint32_t>& OutputIndices);

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
	RECT rc = { 0, 0, (LONG)g_WindowWidth, (LONG)g_WindowHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_WndHandle = CreateWindow(L"GameWindowClass", L"Hello World",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL);
	if (!g_WndHandle)
	{
		return E_FAIL;
	}

	ShowWindow(g_WndHandle, nCmdShow);

	return S_OK;
}

bool InitDevice()
{
	RenderDog::SwapChainDesc swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.width = g_WindowWidth;
	swapChainDesc.height = g_WindowHeight;
	swapChainDesc.format = RenderDog::RD_FORMAT::R8G8B8A8_UNORM;
	swapChainDesc.hOutputWindow = g_WndHandle;

	if (!RenderDog::CreateDeviceAndSwapChain(&g_pDevice, &g_pDeviceContext, &g_pSwapChain, &swapChainDesc))
	{
		return false;
	}

	RenderDog::Texture2D* pBackBuffer = nullptr;
	if (!g_pSwapChain->GetBuffer((void**)&pBackBuffer))
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
	depthDesc.width = g_WindowWidth;
	depthDesc.height = g_WindowWidth;
	depthDesc.format = RenderDog::RD_FORMAT::R32_FLOAT;
	if (!g_pDevice->CreateTexture2D(&depthDesc, &g_pDepthTexture))
	{
		return false;
	}

	if (!g_pDevice->CreateDepthStencilView(g_pDepthTexture, &g_pDepthStencilView))
	{
		return false;
	}

	RenderDog::Viewport vp;
	vp.fWidth = (float)g_WindowWidth;
	vp.fHeight = (float)g_WindowHeight;
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

	std::vector<Vertex> RawBoxVertices =
	{
		//Left Box
		//TOP
		{Vector3(-2.0f, 1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f),  Vector3(0, 1, 0), Vector4(0, 0, 0, 1),	Vector2(1.0f, 1.0f)},
		{Vector3( 0.0f, 1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 1, 0), Vector4(0, 0, 0, 1),	Vector2(0.0f, 1.0f)},
		{Vector3( 2.0f, 1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 1, 0), Vector4(0, 0, 0, 1),	Vector2(1.0f, 1.0f)},
		{Vector3( 2.0f, 1.0f,  1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 1, 0), Vector4(0, 0, 0, 1),	Vector2(1.0f, 0.0f)},
		{Vector3( 0.0f, 1.0f,  1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 1, 0), Vector4(0, 0, 0, 1),	Vector2(0.0f, 0.0f)},
		{Vector3(-2.0f, 1.0f,  1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 1, 0), Vector4(0, 0, 0, 1),	Vector2(1.0f, 0.0f)},

		//BOTTOM
		{Vector3(-2.0f, -1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f),  Vector3(0, -1, 0), Vector4(0, 0, 0, 1),	Vector2(1.0f, 1.0f)},
		{Vector3(0.0f, -1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, -1, 0), Vector4(0, 0, 0, 1),	Vector2(0.0f, 1.0f)},
		{Vector3(2.0f, -1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, -1, 0), Vector4(0, 0, 0, 1),	Vector2(1.0f, 1.0f)},
		{Vector3(2.0f, -1.0f,  1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, -1, 0), Vector4(0, 0, 0, 1),	Vector2(1.0f, 0.0f)},
		{Vector3(0.0f, -1.0f,  1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, -1, 0), Vector4(0, 0, 0, 1),	Vector2(0.0f, 0.0f)},
		{Vector3(-2.0f, -1.0f,  1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, -1, 0), Vector4(0, 0, 0, 1),	Vector2(1.0f, 0.0f)},

		//LEFT
		{Vector3(-2.0f, -1.0f,  1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(-1, 0, 0), Vector4(0, 0, 0, 1),	Vector2(1.0f, 1.0f)},
		{Vector3(-2.0f, -1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(-1, 0, 0), Vector4(0, 0, 0, 1),	Vector2(0.0f, 1.0f)},
		{Vector3(-2.0f,  1.0f, -1.0f), 	Vector3(1.0f, 1.0f, 1.0f),	Vector3(-1, 0, 0), Vector4(0, 0, 0, 1),	Vector2(0.0f, 0.0f)},
		{Vector3(-2.0f,  1.0f,  1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(-1, 0, 0), Vector4(0, 0, 0, 1),	Vector2(1.0f, 0.0f)},

		//RIGHT
		{Vector3(2.0f, -1.0f,  1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(1, 0, 0), Vector4(0, 0, 0, 1),	Vector2(1.0f, 1.0f)},
		{Vector3(2.0f, -1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(1, 0, 0), Vector4(0, 0, 0, 1),	Vector2(0.0f, 1.0f)},
		{Vector3(2.0f,  1.0f, -1.0f), 	Vector3(1.0f, 1.0f, 1.0f),	Vector3(1, 0, 0), Vector4(0, 0, 0, 1),	Vector2(0.0f, 0.0f)},
		{Vector3(2.0f,  1.0f,  1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(1, 0, 0), Vector4(0, 0, 0, 1),	Vector2(1.0f, 0.0f)},

		//FRONT
		{Vector3(-2.0f, -1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 0, -1), Vector4(0, 0, 0, 1),	Vector2(1.0f, 1.0f)},
		{Vector3( 0.0f, -1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 0, -1), Vector4(0, 0, 0, 1),	Vector2(0.0f, 1.0f)},
		{Vector3( 2.0f, -1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 0, -1), Vector4(0, 0, 0, 1),	Vector2(1.0f, 1.0f)},
		{Vector3( 2.0f,  1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 0, -1), Vector4(0, 0, 0, 1),	Vector2(1.0f, 0.0f)},
		{Vector3( 0.0f,  1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 0, -1), Vector4(0, 0, 0, 1),	Vector2(0.0f, 0.0f)},
		{Vector3(-2.0f,  1.0f, -1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 0, -1), Vector4(0, 0, 0, 1),	Vector2(1.0f, 0.0f)},

		//BACK
		{Vector3(-2.0f, -1.0f, 1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 0, 1), Vector4(0, 0, 0, 1),	Vector2(1.0f, 1.0f)},
		{Vector3( 0.0f, -1.0f, 1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 0, 1), Vector4(0, 0, 0, 1),	Vector2(0.0f, 1.0f)},
		{Vector3( 2.0f, -1.0f, 1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 0, 1), Vector4(0, 0, 0, 1),	Vector2(1.0f, 1.0f)},
		{Vector3( 2.0f,  1.0f, 1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 0, 1), Vector4(0, 0, 0, 1),	Vector2(1.0f, 0.0f)},
		{Vector3( 0.0f,  1.0f, 1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 0, 1), Vector4(0, 0, 0, 1),	Vector2(0.0f, 0.0f)},
		{Vector3(-2.0f,  1.0f, 1.0f),	Vector3(1.0f, 1.0f, 1.0f),	Vector3(0, 0, 1), Vector4(0, 0, 0, 1),	Vector2(1.0f, 0.0f)},
	};

	std::vector<uint32_t> RawBoxIndices =
	{
		0, 5, 1, 1, 5, 4,
		1, 4, 2, 2, 4, 3,
		11, 6, 10, 10, 6, 7,
		10, 7, 9, 9, 7, 8,
		12, 15, 13, 13, 15, 14,
		17, 18, 16, 16, 18, 19,
		20, 25, 21, 21, 25, 24,
		21, 24, 22, 22, 24, 23,
		28, 29, 27, 27, 29, 30,
		27, 30, 26, 26, 30, 31
	};

	std::vector<Vertex> aBoxVertices;
	std::vector<uint32_t> aBoxIndices;

	CalculateTangents(RawBoxVertices, RawBoxIndices, aBoxVertices, aBoxIndices);

	RenderDog::VertexBufferDesc vbDesc;
	vbDesc.nVertexNum = (uint32_t)aBoxVertices.size();
	vbDesc.pInitData = &aBoxVertices[0];
	if (!g_pDevice->CreateVertexBuffer(vbDesc, &g_pVertexBuffer))
	{
		return false;
	}

	RenderDog::IndexBufferDesc ibDesc;
	ibDesc.nIndexNum = (uint32_t)aBoxIndices.size();
	ibDesc.pInitData = &aBoxIndices[0];
	if (!g_pDevice->CreateIndexBuffer(ibDesc, &g_pIndexBuffer))
	{
		return false;
	}

	g_pTextureSRV = new RenderDog::ShaderResourceView();

	if (!g_pTextureSRV->LoadFromFile("Textures/PolybumpTangent_DDN.tga"))
	{
		return false;
	}

	RenderDog::CameraDesc camDesc;
	camDesc.vPosition = Vector3(0, 25, -100);
	camDesc.vDirection = Vector3(0, 0, 1);
	camDesc.fFov = 45.0f;
	camDesc.fAspect = (float)g_WindowWidth / g_WindowHeight;
	camDesc.fNear = 0.1f;
	camDesc.fFar = 1000.0f;

	g_pMainCamera = new RenderDog::FPSCamera(camDesc);

	g_WorldMatrix = RenderDog::GetRotationMatrix(90.0f, Vector3(1.0f, 0.0f, 0.0f));
	g_ViewMatrix = g_pMainCamera->GetViewMatrix();
	g_PerspProjMatrix = g_pMainCamera->GetPerspProjectionMatrix();

	memset(aKeys, 0, sizeof(int) * 512);

	RenderDog::DirLightDesc dirLightDesc;
	dirLightDesc.color = Vector3(1.0f, 1.0f, 1.0f);
	dirLightDesc.luminance = 1.0f;
	dirLightDesc.fPhi = -3.14f * 0.25f;
	dirLightDesc.fTheta = 3.14f * 0.25f;
	g_pMainLight = new RenderDog::DirectionalLight(dirLightDesc);

	return true;
}

void CleanupDevice()
{
	if (g_pSwapChain)
	{
		g_pSwapChain->Release();
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
		g_pDevice->Release();
		g_pDevice = nullptr;
	}

	if (g_pDeviceContext)
	{
		g_pDeviceContext->Release();
		g_pDeviceContext = nullptr;
	}

	if (g_pMainCamera)
	{
		delete g_pMainCamera;
		g_pMainCamera = nullptr;
	}

	if (g_pMainLight)
	{
		delete g_pMainLight;
		g_pMainLight = nullptr;
	}
}

void Update(float fTime)
{
	float fSpeed = 1.0f;
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
	Vector4 ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
	g_pDeviceContext->ClearRenderTarget(g_pRenderTargetView, ClearColor);

	g_pDeviceContext->ClearDepthStencil(g_pDepthStencilView, 1.0f);

#if DRAW_BOX
	g_pDeviceContext->IASetVertexBuffer(g_pVertexBuffer);
	g_pDeviceContext->IASetIndexBuffer(g_pIndexBuffer);
#endif
	g_pDeviceContext->IASetPrimitiveTopology(RenderDog::PrimitiveTopology::TRIANGLE_LIST);

	g_pDeviceContext->VSSetShader(g_pVertexShader);
	g_pDeviceContext->VSSetTransMats(&g_WorldMatrix, &g_ViewMatrix, &g_PerspProjMatrix);
	g_pDeviceContext->PSSetShader(g_pPixelShader);
	g_pDeviceContext->PSSetShaderResource(&g_pTextureSRV);
	g_pDeviceContext->PSSetMainLight(g_pMainLight);

#if DRAW_BOX
	g_pDeviceContext->DrawIndex(60);
#else
	g_pStaticModel->Draw(g_pDeviceContext);
#endif

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

	SetCapture(g_WndHandle);
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

void CalculateTangents(const std::vector<Vertex>& RawVertices, const std::vector<uint32_t>& RawIndices, std::vector<Vertex>& OutputVertices, std::vector<uint32_t>& OutputIndices)
{
	OutputVertices.resize(RawVertices.size());
	OutputIndices.resize(RawIndices.size());

	std::vector<Vector3> vPositions(RawVertices.size(), Vector3());
	std::vector<Vector3> vColors(RawVertices.size(), Vector3());
	std::vector<Vector2> vTexcoords(RawVertices.size(), Vector2());
	std::vector<Vector3> vTangents(RawVertices.size(), Vector3());
	std::vector<Vector3> vBiTangents(RawVertices.size(), Vector3());
	std::vector<Vector3> vNormals(RawVertices.size(), Vector3());
	std::vector<float> vHandPartys(RawVertices.size(), 0.0f);

	std::unordered_map<uint32_t, uint32_t> IndexMap; //用于拆点时做索引映射，key：旧索引，value：新索引

	for (uint32_t i = 0; i < RawIndices.size(); i += 3)
	{
		uint32_t nIndex0 = RawIndices[i];
		uint32_t nIndex1 = RawIndices[i + 1];
		uint32_t nIndex2 = RawIndices[i + 2];

		const Vertex& v0 = RawVertices[nIndex0];
		const Vertex& v1 = RawVertices[nIndex1];
		const Vertex& v2 = RawVertices[nIndex2];

		const Vector3& vPos0 = v0.vPosition;
		const Vector3& vPos1 = v1.vPosition;
		const Vector3& vPos2 = v2.vPosition;

		const Vector2& vTex0 = v0.vTexcoord;
		const Vector2& vTex1 = v1.vTexcoord;
		const Vector2& vTex2 = v2.vTexcoord;

		float x1 = vPos1.x - vPos0.x;
		float x2 = vPos2.x - vPos0.x;
		float y1 = vPos1.y - vPos0.y;
		float y2 = vPos2.y - vPos0.y;
		float z1 = vPos1.z - vPos0.z;
		float z2 = vPos2.z - vPos0.z;

		float s1 = vTex1.x - vTex0.x;
		float s2 = vTex2.x - vTex0.x;
		float t1 = vTex1.y - vTex0.y;
		float t2 = vTex2.y - vTex0.y;

		float fInv = 1.0f / (s1 * t2 - s2 * t1);

		float tx = fInv * (t2 * x1 - t1 * x2);
		float ty = fInv * (t2 * y1 - t1 * y2);
		float tz = fInv * (t2 * z1 - t1 * z2);

		float bx = fInv * (s1 * x2 - s2 * x1);
		float by = fInv * (s1 * y2 - s2 * y1);
		float bz = fInv * (s1 * z2 - s2 * z1);

		Vector3 vRawTangent = Vector3(tx, ty, tz);
		Vector3 vRawBiTangent = Vector3(bx, by, bz);

		const Vector3& vRawNormal0 = v0.vNormal;
		const Vector3& vRawNormal1 = v1.vNormal;
		const Vector3& vRawNormal2 = v2.vNormal;

		float fRawHandParty0 = vHandPartys[nIndex0];
		float fRawHandParty1 = vHandPartys[nIndex1];
		float fRawHandParty2 = vHandPartys[nIndex2];

		float fHandParty0 = DotProduct(CrossProduct(vRawTangent, vRawBiTangent), vRawNormal0) > 0.0f ? 1.0f : -1.0f;
		float fHandParty1 = DotProduct(CrossProduct(vRawTangent, vRawBiTangent), vRawNormal1) > 0.0f ? 1.0f : -1.0f;
		float fHandParty2 = DotProduct(CrossProduct(vRawTangent, vRawBiTangent), vRawNormal2) > 0.0f ? 1.0f : -1.0f;

		if (fRawHandParty0 != 0 && fHandParty0 != fRawHandParty0)
		{
			if (IndexMap.find(nIndex0) != IndexMap.end())
			{
				//与已经拆分过的顶点合并
				uint32_t nNewIndex = IndexMap[nIndex0];
				vNormals[nNewIndex] = vRawNormal0;
				vTangents[nNewIndex] += vRawTangent;
				vBiTangents[nNewIndex] += vRawBiTangent;

				OutputIndices[i] = nNewIndex;
			}
			else
			{
				//拆分新的顶点
				vPositions.push_back(vPos0);
				vColors.push_back(v0.vColor);
				vTexcoords.push_back(vTex0);
				vNormals.push_back(vRawNormal0);
				vTangents.push_back(vRawTangent);
				vBiTangents.push_back(vRawBiTangent);
				vHandPartys.push_back(fHandParty0);

				OutputIndices[i] = ((uint32_t)OutputVertices.size());
				OutputVertices.push_back(Vertex());

				IndexMap.insert({ nIndex0, OutputIndices[i] });
			}
		}
		else
		{
			vPositions[nIndex0] = vPos0;
			vColors[nIndex0] = v0.vColor;
			vTexcoords[nIndex0] = vTex0;
			vNormals[nIndex0] = vRawNormal0;
			vTangents[nIndex0] += vRawTangent;
			vBiTangents[nIndex0] += vRawBiTangent;
			vHandPartys[nIndex0] = fHandParty0;

			OutputIndices[i] = nIndex0;
		}

		if (fRawHandParty1 != 0 && fHandParty1 != fRawHandParty1)
		{
			if (IndexMap.find(nIndex1) != IndexMap.end())
			{
				//与已经拆分过的顶点合并
				uint32_t nNewIndex = IndexMap[nIndex1];
				vNormals[nNewIndex] = vRawNormal1;
				vTangents[nNewIndex] += vRawTangent;
				vBiTangents[nNewIndex] += vRawBiTangent;

				OutputIndices[i + 1] = nNewIndex;
			}
			else
			{
				//拆分新的顶点
				vPositions.push_back(vPos1);
				vColors.push_back(v1.vColor);
				vTexcoords.push_back(vTex1);
				vNormals.push_back(vRawNormal1);
				vTangents.push_back(vRawTangent);
				vBiTangents.push_back(vRawBiTangent);
				vHandPartys.push_back(fHandParty1);

				OutputIndices[i + 1] = ((uint32_t)OutputVertices.size());
				OutputVertices.push_back(Vertex());


				IndexMap.insert({ nIndex1, OutputIndices[i + 1] });
			}
		}
		else
		{
			vPositions[nIndex1] = vPos1;
			vColors[nIndex1] = v1.vColor;
			vTexcoords[nIndex1] = vTex1;
			vNormals[nIndex1] = vRawNormal1;
			vTangents[nIndex1] += vRawTangent;
			vBiTangents[nIndex1] += vRawBiTangent;
			vHandPartys[nIndex1] = fHandParty1;

			OutputIndices[i + 1] = nIndex1;
		}

		if (fRawHandParty2 != 0 && fHandParty2 != fRawHandParty2)
		{
			if (IndexMap.find(nIndex2) != IndexMap.end())
			{
				//与已经拆分过的顶点合并
				uint32_t nNewIndex = IndexMap[nIndex2];
				vNormals[nNewIndex] = vRawNormal2;
				vTangents[nNewIndex] += vRawTangent;
				vBiTangents[nNewIndex] += vRawBiTangent;

				OutputIndices[i + 2] = nNewIndex;
			}
			else
			{
				//拆分新的顶点
				vPositions.push_back(vPos2);
				vColors.push_back(v2.vColor);
				vTexcoords.push_back(vTex2);
				vNormals.push_back(vRawNormal2);
				vTangents.push_back(vRawTangent);
				vBiTangents.push_back(vRawBiTangent);
				vHandPartys.push_back(fHandParty2);

				OutputIndices[i + 2] = ((uint32_t)OutputVertices.size());
				OutputVertices.push_back(Vertex());

				IndexMap.insert({ nIndex2, OutputIndices[i + 2] });
			}
		}
		else
		{
			vPositions[nIndex2] = vPos2;
			vColors[nIndex2] = v2.vColor;
			vTexcoords[nIndex2] = vTex2;
			vNormals[nIndex2] = vRawNormal2;
			vTangents[nIndex2] += vRawTangent;
			vBiTangents[nIndex2] += vRawBiTangent;
			vHandPartys[nIndex2] = fHandParty2;

			OutputIndices[i + 2] = nIndex2;
		}
	}

	for (uint32_t i = 0; i < OutputVertices.size(); ++i)
	{
		Vector3 vPos = vPositions[i];
		Vector3 vColor = vColors[i];
		Vector2 vTexcoord = vTexcoords[i];
		float fHandParty = vHandPartys[i];
		//Vector3 vTangent = Normalize(vTangents[i]);
		//Vector3 vBiTangent = Normalize(vBiTangents[i]);
		//Vector3 vNormal = CrossProduct(vTangent, vBiTangent) * fHandParty;
		Vector3 vNormal = Normalize(vNormals[i]);
		//Vector3 vTangent = Normalize((vTangents[i] - vNormal * DotProduct(vNormal, vTangents[i])));
		Vector3 vTangent = Normalize(vTangents[i]);

		OutputVertices[i] = { vPos, vColor, vNormal, Vector4(vTangent.x, vTangent.y, vTangent.z, fHandParty), vTexcoord };
	}
}