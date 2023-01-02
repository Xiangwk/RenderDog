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
#include "SimpleModel.h"
#include "StaticModel.h"
#include "SkinModel.h"
#include "Sky.h"
#include "GameTimer.h"
#include "Material.h"

struct ModelViewerInitDesc
{
	RenderDog::WindowDesc wndDesc;
};

class ModelViewer
{
private:
	enum class LOAD_MODEL_TYPE
	{
		STANDARD = 0,
		CUSTOM_STATIC,
		CUSTOM_SKIN
	};

public:
	ModelViewer();
	~ModelViewer();

	bool						Init(const ModelViewerInitDesc& desc);
	void						Release();

	int							Run();

	LRESULT						MessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	bool						LoadFloor(uint32_t width, uint32_t depth, float unit);
	bool						LoadSkyBox();
	bool						LoadFbxModel(const std::string& fileName, LOAD_MODEL_TYPE modelType);
	bool						LoadFbxAnimation(const std::string& fileName, RenderDog::SkinModel* pSkinModel);

	//FIXME!!! 这里只是临时添加此函数，后续应将此函数转移到Model相关的类中
	RenderDog::IMaterial*		CreateBasicMaterial(const std::string& mtlName);
	RenderDog::IMaterial*		CreateSkyMaterial(const std::string& mtlName);

	bool						LoadStaticModelMaterialParams(std::vector<RenderDog::MaterialParam>& params);
	bool						LoadSkinModelMaterialParams(std::vector<RenderDog::MaterialParam>& params);

	void						Update();
	void						RegisterObjectToScene();

	void						CalculateFrameStats();

	void						OnMouseDown(WPARAM btnState, int x, int y);
	void						OnMouseUp(WPARAM btnState, int x, int y);
	void						OnMouseMove(WPARAM btnState, int x, int y);

private:
	RenderDog::IRenderDog*		m_pRenderDog;
	RenderDog::IScene*			m_pScene;

	RenderDog::SimpleModel*		m_pGridLine;
	RenderDog::StaticModel*		m_pFloor;
	RenderDog::StaticModel*		m_pStaticModel;
	RenderDog::SkinModel*		m_pSkinModel;
	RenderDog::SkyBox*			m_pSkyBox;

	RenderDog::FPSCamera*		m_pFPSCamera;

	RenderDog::ILight*			m_pMainLight;

	RenderDog::GameTimer*		m_pGameTimer;

	static int					m_Keys[512];	// 当前键盘按下状态

	int							m_LastMousePosX;
	int							m_LastMousePosY;

	bool						m_bShowUnitGrid;
	bool						m_bModelMoved;

	RenderDog::IMaterial*		m_pBasicMaterial;
	RenderDog::IMaterial*		m_pSkyMaterial;
};

extern ModelViewer* g_pModelViewer;