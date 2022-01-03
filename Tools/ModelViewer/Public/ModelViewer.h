///////////////////////////////////////////
//ModelViewer
//FileName: ModelViewer.h
//Written by Xiang Weikang
///////////////////////////////////////////

#pragma once

#include "RenderDog.h"

class ModelViewer
{
public:
	ModelViewer();
	~ModelViewer();

	bool Init(const RenderDog::InitDesc& desc);
	void Release();

	int Run();

	static LRESULT MessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	RenderDog::IRenderDog*	m_pRenderDog;
};

extern ModelViewer* g_pModelViewer;