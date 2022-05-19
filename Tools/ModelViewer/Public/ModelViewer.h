///////////////////////////////////////////
//ModelViewer
//FileName: ModelViewer.h
//Written by Xiang Weikang
///////////////////////////////////////////

#pragma once

#include "RenderDog.h"
#include "Scene.h"
#include "Camera.h"
#include "Light.h"
#include "StaticModel.h"
#include "GameTimer.h"

struct ModelViewerInitDesc
{
	RenderDog::WindowDesc wndDesc;
};

class ModelViewer
{
public:
	ModelViewer();
	~ModelViewer();

	bool						Init(const ModelViewerInitDesc& desc);
	void						Release();

	int							Run();

	LRESULT						MessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void						Update();

	void						CalculateFrameStats();

	void						OnMouseDown(WPARAM btnState, int x, int y);
	void						OnMouseUp(WPARAM btnState, int x, int y);
	void						OnMouseMove(WPARAM btnState, int x, int y);

private:
	RenderDog::IRenderDog*		m_pRenderDog;
	RenderDog::IScene*			m_pScene;

	RenderDog::StaticModel*		m_pModel;

	RenderDog::FPSCamera*		m_pFPSCamera;

	RenderDog::ILight*			m_pMainLight;

	RenderDog::GameTimer*		m_pGameTimer;

	static int					m_Keys[512];	// 当前键盘按下状态

	int							m_LastMousePosX;
	int							m_LastMousePosY;
};

extern ModelViewer* g_pModelViewer;