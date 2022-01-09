///////////////////////////////////////////
//ModelViewer
//FileName: GeometryGenerator.h
//Written by Xiang Weikang
///////////////////////////////////////////

#pragma once

#include "Vertex.h"

#include <vector>

class GeometryGenerator
{
public:
	struct LocalMeshData
	{
		std::vector<RenderDog::LocalVertex> vertices;
		std::vector<unsigned int> indices;
	};

	struct SimpleMeshData
	{
		std::vector<RenderDog::SimpleVertex> vertices;
		std::vector<unsigned int> indices;
	};

	void GenerateBox(float width, float height, float depth, LocalMeshData& outputMesh);
};

extern GeometryGenerator* g_pGeometryGenerator;
