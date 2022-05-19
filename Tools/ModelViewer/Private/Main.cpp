///////////////////////////////////////////
//ModelViewer
//FileName: Main.cpp
//Written by Xiang Weikang
///////////////////////////////////////////

#include "ModelViewer.h"

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return g_pModelViewer->MessageProc(hWnd, Msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	ModelViewerInitDesc initDesc;
	initDesc.wndDesc.width = 1024;
	initDesc.wndDesc.height = 768;
	initDesc.wndDesc.caption = "ModelViewer";
	initDesc.wndDesc.className = "ModelViewerWindowClass";
	initDesc.wndDesc.hAppInstance = hInstance;
	initDesc.wndDesc.wndProc = MainWndProc;
	if (!g_pModelViewer->Init(initDesc))
	{
		return 0;
	}

	g_pModelViewer->Run();

	g_pModelViewer->Release();

	return 0;
}