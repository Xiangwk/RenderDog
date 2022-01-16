///////////////////////////////////////////
//ModelViewer
//FileName: ModelViewer.h
//Written by Xiang Weikang
///////////////////////////////////////////

#include "ModelViewer.h"
#include "GeometryGenerator.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	RenderDog::InitDesc renderDogDesc;
	renderDogDesc.wndDesc.width = 1024;
	renderDogDesc.wndDesc.height = 768;
	renderDogDesc.wndDesc.caption = "ModelViewer";
	renderDogDesc.wndDesc.className = "ModelViewerWindowClass";
	renderDogDesc.wndDesc.hAppInstance = hInstance;
	renderDogDesc.wndDesc.wndProc = g_pModelViewer->MessageProc;
	if (!g_pModelViewer->Init(renderDogDesc))
	{
		return 0;
	}

	g_pModelViewer->Run();

	g_pModelViewer->Release();

	return 0;
}