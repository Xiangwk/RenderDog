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

struct ModelViewerInitDesc
{
	RenderDog::WindowDesc wndDesc;
};

class ModelViewer
{
public:
	ModelViewer();
	~ModelViewer();

	bool			Init(const ModelViewerInitDesc& desc);
	void			Release();

	int				Run();

	static LRESULT	MessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	RenderDog::IRenderDog*		m_pRenderDog;
	RenderDog::IScene*			m_pScene;

	RenderDog::StaticModel*		m_pModel;

	RenderDog::FPSCamera*		m_pFPSCamera;
	RenderDog::ILight*			m_pMainLight;
};

extern ModelViewer* g_pModelViewer;