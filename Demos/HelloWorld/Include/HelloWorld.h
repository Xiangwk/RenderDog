///////////////////////////////////
//RenderDog <·,·>
//FileName: HelloWorld.h
//Hello World Demo
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include "RenderDog.h"
#include "Scene.h"
#include "Camera.h"
#include "Light.h"
#include "SimpleModel.h"
#include "StaticModel.h"
#include "GameTimer.h"
#include "Material.h"

struct DemoInitDesc
{
	RenderDog::WindowDesc wndDesc;
};

class DemoApp
{
private:
	enum class LOAD_MODEL_TYPE
	{
		STANDARD = 0,
		CUSTOM_STATIC,
		CUSTOM_SKIN
	};

public:
	DemoApp();
	~DemoApp();

	bool						Init(const DemoInitDesc& desc);
	void						Release();

	int							Run();

	LRESULT						MessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	bool						LoadModel(const std::string& fileName, LOAD_MODEL_TYPE modelType);

	RenderDog::IMaterial*		CreateBasicMaterial(const std::string& mtlName);
	RenderDog::IMaterial*		CreateGeneratorMaterial(const std::string& mtlName);

	void						Update();

	void						CalculateFrameStats();

	void						OnMouseDown(WPARAM btnState, int x, int y);
	void						OnMouseUp(WPARAM btnState, int x, int y);
	void						OnMouseMove(WPARAM btnState, int x, int y);

private:
	RenderDog::IRenderDog*		m_pRenderDog;
	RenderDog::IScene*			m_pScene;

	RenderDog::SimpleModel*		m_pGridLine;

	RenderDog::StaticModel*		m_pModel;

	RenderDog::FPSCamera*		m_pFPSCamera;

	RenderDog::ILight*			m_pMainLight;

	RenderDog::GameTimer*		m_pGameTimer;

	static int					m_Keys[512];	// 当前键盘按下状态

	int							m_LastMousePosX;
	int							m_LastMousePosY;

	RenderDog::IMaterial*		m_pBasicMaterial;
	RenderDog::IMaterial*		m_pGeneratorMaterial;
};

extern DemoApp* g_pHelloWorldDemo;