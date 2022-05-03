///////////////////////////////////////////
//Hello World Demo
//FileName: Main.cpp
//Written by Xiang Weikang
///////////////////////////////////////////

#include "HelloWorld.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	DemoInitDesc initDesc;
	initDesc.wndDesc.width = 1024;
	initDesc.wndDesc.height = 768;
	initDesc.wndDesc.caption = "ModelViewer";
	initDesc.wndDesc.className = "ModelViewerWindowClass";
	initDesc.wndDesc.hAppInstance = hInstance;
	initDesc.wndDesc.wndProc = g_pHelloWorldDemo->MessageProc;
	if (!g_pHelloWorldDemo->Init(initDesc))
	{
		return 0;
	}

	g_pHelloWorldDemo->Run();

	g_pHelloWorldDemo->Release();

	return 0;
}