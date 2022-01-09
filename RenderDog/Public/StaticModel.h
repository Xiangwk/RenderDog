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

		void LoadFromData(const std::vector<LocalVertex>& vertices, const std::vector<uint32_t>& indices);

		void RegisterToScene(IScene* pScene);

	private:
		std::vector<StaticMesh>	m_Meshes;
	};

}// namespace RenderDog
