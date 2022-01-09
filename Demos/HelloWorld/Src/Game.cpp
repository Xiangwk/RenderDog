///////////////////////////////////
//RenderDog <·,·>
//FileName: Game.cpp
//Hello World Demo
//Written by Xiang Weikang
///////////////////////////////////

#include "SoftwareRenderer.h"
#include "ShaderResourceUtility.h"
#include "Transform.h"
#include "Camera.h"
#include "Model.h"
#include "Light.h"
#include "GameTimer.h"

#include <windowsx.h>
#include <string>
#include <sstream>
#include <unordered_map>

using RenderDog::Vector2;
using RenderDog::Vector3;
using RenderDog::Vector4;

HINSTANCE						g_hInst = nullptr;
HWND							g_WndHandle = nullptr;
uint32_t						g_WindowWidth = 1024;
uint32_t						g_WindowHeight = 768;
								
RenderDog::IDevice*				g_pDevice = nullptr;
RenderDog::IDeviceContext*		g_pDeviceContext = nullptr;
RenderDog::ISwapChain*			g_pSwapChain = nullptr;
RenderDog::IRenderTargetView*	g_pRenderTargetView = nullptr;
RenderDog::IBuffer*				g_pMVPMatrixConstantBuffer = nullptr;
RenderDog::IBuffer*				g_pMainLightConstantBuffer = nullptr;
RenderDog::IVertexShader*		g_pVertexShader = nullptr;
RenderDog::IPixelShader*		g_pPixelShader = nullptr;
RenderDog::ITexture2D*			g_pDepthTexture = nullptr;
RenderDog::IDepthStencilView*	g_pDepthStencilView = nullptr;
RenderDog::IShaderResourceView*	g_pTextureSRV = nullptr;

RenderDog::GameTimer*			g_pGameTimer = nullptr;

RenderDog::Matrix4x4			g_WorldMatrix;
RenderDog::Matrix4x4			g_ViewMatrix;
RenderDog::Matrix4x4			g_PerspProjMatrix;

Vector2							g_LastMousePos;

RenderDog::StaticModel*			g_pStaticModel;

RenderDog::DirectionalLight*	g_pMainLight;


int aKeys[512];	// 当前键盘按下状态

RenderDog::FPSCamera*			g_pMainCamera;
const float						g_fCameraSpeed = 1.0f;

#pragma region ConstantBufferStruct
struct ConstantBufferMVPMatrix
{
	RenderDog::Matrix4x4 worldMatrix;
	RenderDog::Matrix4x4 viewMatrix;
	RenderDog::Matrix4x4 projMatrix;
};

struct ConstantBufferMainLight
{
	Vector3	direction;	//从光源发射光线的方向
	Vector3	color;
	float	luminance;
};
#pragma endregion ConstantBufferStruct

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

void CalculateFrameStats();

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
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
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
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		nullptr);
	if (!g_WndHandle)
	{
		return E_FAIL;
	}

	ShowWindow(g_WndHandle, nCmdShow);

	return S_OK;
}

bool InitDevice()
{
	g_pGameTimer = new RenderDog::GameTimer();
	g_pGameTimer->Reset();

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

	RenderDog::ITexture2D* pBackBuffer = nullptr;
	if (!g_pSwapChain->GetBuffer((void**)&pBackBuffer))
	{
		return false;
	}

	if (!g_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView))
	{
		return false;
	}
	pBackBuffer->Release();

	RenderDog::Texture2DDesc depthDesc;
	depthDesc.width = g_WindowWidth;
	depthDesc.height = g_WindowWidth;
	depthDesc.format = RenderDog::RD_FORMAT::R32_FLOAT;
	if (!g_pDevice->CreateTexture2D(&depthDesc, nullptr, &g_pDepthTexture))
	{
		return false;
	}

	RenderDog::DepthStencilViewDesc dsDesc;
	dsDesc.format = depthDesc.format;
	dsDesc.viewDimension = RenderDog::RD_DSV_DIMENSION::TEXTURE2D;
	if (!g_pDevice->CreateDepthStencilView(g_pDepthTexture, &dsDesc, &g_pDepthStencilView))
	{
		return false;
	}

	RenderDog::Viewport vp;
	vp.width = (float)g_WindowWidth;
	vp.height = (float)g_WindowHeight;
	vp.minDepth = 0.0f;
	vp.maxDepth = 1.0f;
	vp.topLeftX = 0;
	vp.topLeftY = 0;
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

	RenderDog::BufferDesc cbDesc;
	cbDesc.byteWidth = sizeof(ConstantBufferMVPMatrix);
	cbDesc.bindFlag = RenderDog::RD_BIND_FLAG::BIND_CONSTANT_BUFFER;
	if (!g_pDevice->CreateBuffer(&cbDesc, nullptr, &g_pMVPMatrixConstantBuffer))
	{
		return false;
	}

	cbDesc.byteWidth = sizeof(ConstantBufferMainLight);
	if (!g_pDevice->CreateBuffer(&cbDesc, nullptr, &g_pMainLightConstantBuffer))
	{
		return false;
	}

	if (!RenderDog::CreateShaderResourceViewFromFile(g_pDevice, "Textures/PolybumpTangent_DDN.tga", &g_pTextureSRV))
	{
		return false;
	}

	RenderDog::CameraDesc camDesc;
	camDesc.position = Vector3(0, 25, -100);
	camDesc.direction = Vector3(0, 0, 1);
	camDesc.fov = 45.0f;
	camDesc.aspectRitio = (float)g_WindowWidth / g_WindowHeight;
	camDesc.nearPlane = 0.1f;
	camDesc.farPlane = 1000.0f;

	g_pMainCamera = new RenderDog::FPSCamera(camDesc);

	g_WorldMatrix = RenderDog::GetRotationMatrix(90.0f, Vector3(1.0f, 0.0f, 0.0f));
	g_ViewMatrix = g_pMainCamera->GetViewMatrix();
	g_PerspProjMatrix = g_pMainCamera->GetPerspProjectionMatrix();

	memset(aKeys, 0, sizeof(int) * 512);

	RenderDog::DirLightDesc dirLightDesc;
	dirLightDesc.color = Vector3(1.0f, 1.0f, 1.0f);
	dirLightDesc.luminance = 0.8f;
	dirLightDesc.pitch = -3.14f * 0.25f;
	dirLightDesc.yaw = 3.14f * 0.25f;
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
		g_pRenderTargetView->Release();
		g_pRenderTargetView = nullptr;
	}

	if (g_pVertexShader)
	{
		g_pVertexShader->Release();
		g_pVertexShader = nullptr;
	}

	if (g_pPixelShader)
	{
		g_pPixelShader->Release();
		g_pPixelShader = nullptr;
	}

	if (g_pStaticModel)
	{
		g_pStaticModel->Release();

		delete g_pStaticModel;
		g_pStaticModel = nullptr;
	}

	if (g_pMVPMatrixConstantBuffer)
	{
		g_pMVPMatrixConstantBuffer->Release();
		g_pMVPMatrixConstantBuffer = nullptr;
	}

	if (g_pMainLightConstantBuffer)
	{
		g_pMainLightConstantBuffer->Release();
		g_pMainLightConstantBuffer = nullptr;
	}

	if (g_pDepthTexture)
	{
		g_pDepthTexture->Release();
		g_pDepthTexture = nullptr;
	}

	if (g_pDepthStencilView)
	{
		g_pDepthStencilView->Release();
		g_pDepthStencilView = nullptr;
	}

	if (g_pTextureSRV)
	{
		g_pTextureSRV->Release();

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

	if (g_pGameTimer)
	{
		delete g_pGameTimer;
		g_pGameTimer = nullptr;
	}
}

void Update(float fTime)
{
	//W
	if (aKeys[0x57])
	{
		g_pMainCamera->Move(g_fCameraSpeed, RenderDog::FPSCamera::MoveMode::FrontAndBack);
	}
	//S
	if (aKeys[0x53])
	{
		g_pMainCamera->Move(-g_fCameraSpeed, RenderDog::FPSCamera::MoveMode::FrontAndBack);
	}
	//A
	if (aKeys[0x41])
	{
		g_pMainCamera->Move(-g_fCameraSpeed, RenderDog::FPSCamera::MoveMode::LeftAndRight);
	}
	//D
	if (aKeys[0x44])
	{
		g_pMainCamera->Move(g_fCameraSpeed, RenderDog::FPSCamera::MoveMode::LeftAndRight);
	}
	//Q
	if (aKeys[0x51])
	{
		g_pMainCamera->Move(g_fCameraSpeed, RenderDog::FPSCamera::MoveMode::UpAndDown);
	}
	//E
	if (aKeys[0x45])
	{
		g_pMainCamera->Move(-g_fCameraSpeed, RenderDog::FPSCamera::MoveMode::UpAndDown);
	}
	
	g_ViewMatrix = g_pMainCamera->GetViewMatrix();

	ConstantBufferMVPMatrix mvpCB;
	mvpCB.worldMatrix = g_WorldMatrix;
	mvpCB.viewMatrix = g_ViewMatrix;
	mvpCB.projMatrix = g_PerspProjMatrix;
	g_pDeviceContext->UpdateSubresource(g_pMVPMatrixConstantBuffer, &mvpCB, 0, 0);

	ConstantBufferMainLight mainLightCB;
	mainLightCB.direction = g_pMainLight->GetDirection();
	mainLightCB.color = g_pMainLight->GetColor();
	mainLightCB.luminance = g_pMainLight->GetLuminance();
	g_pDeviceContext->UpdateSubresource(g_pMainLightConstantBuffer, &mainLightCB, 0, 0);
}

void Render()
{
	g_pDeviceContext->OMSetRenderTarget(g_pRenderTargetView, g_pDepthStencilView);
	float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	g_pDeviceContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);

	g_pDeviceContext->ClearDepthStencilView(g_pDepthStencilView, 1.0f);

	g_pDeviceContext->IASetPrimitiveTopology(RenderDog::RD_PRIMITIVE_TOPOLOGY::TRIANGLE_LIST);

	g_pDeviceContext->VSSetShader(g_pVertexShader);
	//FIXME!!! 这里暂时将slot0固定设置为MVP矩阵，不能修改
	g_pDeviceContext->VSSetConstantBuffer(0, &g_pMVPMatrixConstantBuffer);
	g_pDeviceContext->PSSetShader(g_pPixelShader);
	g_pDeviceContext->PSSetShaderResource(&g_pTextureSRV);
	//FIXME!!! 这里暂时将slot1固定设置为主光，不能修改
	g_pDeviceContext->PSSetConstantBuffer(1, &g_pMainLightConstantBuffer);

	g_pStaticModel->Draw(g_pDeviceContext);

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
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			g_pGameTimer->Tick();
			CalculateFrameStats();

			Update((float)g_pGameTimer->GetDeltaTime());
			Render();
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

void CalculateFrameStats()
{
	static int frameCnt = 0;
	static double timeElapsed = 0.0;

	++frameCnt;
	//当统计时间大于1s时，统计framecnt以计算fps
	if ((g_pGameTimer->GetTotalTime() - timeElapsed) >= 1.0f)
	{
		float framePerSecond = static_cast<float>(frameCnt);
		float millisecondsPerFrame = 1000.0f / framePerSecond;

		std::ostringstream outs;
		outs.precision(6);
		outs << "FPS: " << framePerSecond << "    "
			<< "Frame Time: " << millisecondsPerFrame << " (ms)";
		SetWindowTextA(g_WndHandle, outs.str().c_str());

		frameCnt = 0;
		timeElapsed += 1.0;
	}
}