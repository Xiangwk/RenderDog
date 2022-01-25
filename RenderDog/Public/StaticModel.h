////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: StaticModel.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "StaticMesh.h"

namespace RenderDog
{
	class IScene;

	class StaticModel
	{
	public:
		StaticModel();
		~StaticModel();

		StaticModel(const StaticModel&) = default;
		StaticModel& operator=(const StaticModel&) = default;

		void LoadFromData(const std::vector<LocalVertex>& vertices, 
						  const std::vector<uint32_t>& indices, 
						  const std::wstring& diffuseTexturePath);

		void SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale);

		void RegisterToScene(IScene* pScene);

		void ReleaseRenderData();

	private:
		std::vector<StaticMesh>	m_Meshes;

		Vector3					m_Pos;
		Vector3					m_Dir;
		float					m_Scale;
	};

}// namespace RenderDog
