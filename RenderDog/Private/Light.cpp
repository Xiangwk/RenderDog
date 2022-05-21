///////////////////////////////////
//RenderDog <·,·>
//FileName: Light.cpp
//Written by Xiang Weikang
///////////////////////////////////

#include "Light.h"
#include "Matrix.h"
#include "Transform.h"
#include "Scene.h"

namespace RenderDog
{
	const Vector3 g_InitLightDir = Vector3(0.0f, 0.0f, 1.0f);

	//================================================================
	//        Directional Light
	//================================================================
	class DirectionalLight : public ILight
	{
	public:
		DirectionalLight();
		virtual ~DirectionalLight() {}

		virtual		bool		Init(const LightDesc& desc) override;
		virtual		void		Release() override;

		virtual		LIGHT_TYPE	GetType() const override { return LIGHT_TYPE::DIRECTIONAL; }
		virtual		Vector3		GetDirection() const override { return m_Direction; }
		virtual		Vector3		GetEulerAngle() const override { return m_eulerAngle; }
		virtual     Vector3		GetColor() const override { return m_Color; }
		virtual		float		GetLuminance() const override { return m_Luminance; }

		virtual		void		SetDirection(float eulerX, float eulerY, float eulerZ) override;
		virtual		void		SetColor(const Vector3& color) override { m_Color = color; }
		virtual		void		SetLuminance(float luma) override { m_Luminance = luma; }

		virtual void			RegisterToScene(IScene* pScene) override;

	private:
		Vector3					m_Direction;	//从光源发射光线的方向
		Vector3					m_eulerAngle;
		Vector3					m_Color;
		float					m_Luminance;
	};

	DirectionalLight::DirectionalLight() :
		m_Direction(g_InitLightDir),
		m_Color(1.0f, 1.0f, 1.0f),
		m_Luminance(1.0f),
		m_eulerAngle(0.0f, 0.0f, 0.0f)
	{}

	bool DirectionalLight::Init(const LightDesc& desc)
	{
		Matrix4x4 rotMat = GetIdentityMatrix();
		rotMat = GetRotationMatrix(desc.eulerDir.x, desc.eulerDir.y, desc.eulerDir.z);
		Vector4 dir = Vector4(m_Direction, 0.0f);
		dir = dir * rotMat;

		m_eulerAngle = desc.eulerDir;

		m_Direction = Vector3(dir.x, dir.y, dir.z);
		m_Color = desc.color;
		m_Luminance = desc.luminance;

		return true;
	}

	void DirectionalLight::Release()
	{
		g_pILightManager->ReleaseLight(this);
	}

	void DirectionalLight::SetDirection(float eulerX, float eulerY, float eulerZ)
	{
		Matrix4x4 rotMat = GetIdentityMatrix();
		rotMat = GetRotationMatrix(eulerX, eulerY, eulerZ);
		Vector4 dir = Vector4(g_InitLightDir, 0.0f);
		dir = dir * rotMat;

		m_Direction = Vector3(dir.x, dir.y, dir.z);

		m_eulerAngle.x = eulerX;
		m_eulerAngle.y = eulerY;
		m_eulerAngle.z = eulerZ;
	}

	void DirectionalLight::RegisterToScene(IScene* pScene)
	{
		pScene->RegisterLight(this);
	}


	//================================================================
	//        Light Manager
	//================================================================

	class LightManager : public ILightManager
	{
	public:
		LightManager() = default;
		virtual ~LightManager() = default;

		virtual	ILight*		CreateLight(const LightDesc& desc) override;
		virtual void		ReleaseLight(ILight* pLight) override;
	};

	LightManager g_LightManager;
	ILightManager* g_pILightManager = &g_LightManager;

	ILight* LightManager::CreateLight(const LightDesc& desc)
	{
		ILight* pLight = nullptr;
		if (desc.type == LIGHT_TYPE::DIRECTIONAL)
		{
			pLight = new DirectionalLight();
			pLight->Init(desc);
			pLight->AddRef();
		}

		return pLight;
	}

	void LightManager::ReleaseLight(ILight* pLight)
	{
		pLight->SubRef();
	}
}