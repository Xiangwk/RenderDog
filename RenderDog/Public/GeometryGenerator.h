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
		struct StandardMeshData
		{
			std::vector<StandardVertex> vertices;
			std::vector<uint32_t> indices;
		};

		struct SimpleMeshData
		{
			std::vector<SimpleVertex> vertices;
			std::vector<uint32_t> indices;
		};

		void GenerateGrid(int width, int depth, float gridUnit, StandardMeshData& outputMesh);
		//���������ߣ�ʹ��LineList����
		void GenerateGridLine(int width, int depth, float gridUnit, const Vector4& lineColor, SimpleMeshData& outputMesh);
		void GenerateBox(float width, float height, float depth, StandardMeshData& outputMesh);
		//longitudeNum: �����ϵķֿ���������֣�	latitudeNum: γ���ϵķֿ��������ݷ֣�
		void GenerateSphere(uint32_t longitudeNum, uint32_t latitudeNum, float radius, StandardMeshData& outputMesh);
	};

	extern GeometryGenerator* g_pGeometryGenerator;

}// namespace RenderDog


