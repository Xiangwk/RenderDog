///////////////////////////////////////////
//ModelViewer
//FileName: ModelViewer.h
//Written by Xiang Weikang
///////////////////////////////////////////

#include "ModelViewer.h"
#include "GeometryGenerator.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	ModelViewerInitDesc initDesc;
	initDesc.wndDesc.width = 1024;
	initDesc.wndDesc.height = 768;
	initDesc.wndDesc.caption = "ModelViewer";
	initDesc.wndDesc.className = "ModelViewerWindowClass";
	initDesc.wndDesc.hAppInstance = hInstance;
	initDesc.wndDesc.wndProc = g_pModelViewer->MessageProc;
	if (!g_pModelViewer->Init(initDesc))
	{
		return 0;
	}

	g_pModelViewer->Run();

	g_pModelViewer->Release();

	return 0;
}