///////////////////////////////////////////
//ModelViewer
//FileName: GeometryGenerator.h
//Written by Xiang Weikang
///////////////////////////////////////////

#pragma once

#include "Vertex.h"

#include <vector>

namespace RenderDog
{
	class GeometryGenerator
	{
	public:
		struct LocalMeshData
		{
			std::vector<RenderDog::StandardVertex> vertices;
			std::vector<unsigned int> indices;
		};

		struct SimpleMeshData
		{
			std::vector<RenderDog::SimpleVertex> vertices;
			std::vector<unsigned int> indices;
		};

		void GenerateBox(float width, float height, float depth, LocalMeshData& outputMesh);
		//生成网格线，使用LineList绘制
		void GenerateGridLine(int width, int depth, float gridUnit, const RenderDog::Vector4& lineColor, SimpleMeshData& outputMesh);
	};

	extern GeometryGenerator* g_pGeometryGenerator;

}// namespace RenderDog


