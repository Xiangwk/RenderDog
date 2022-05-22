///////////////////////////////////////////
//ModelViewer
//FileName: GeometryGenerator.cpp
//Written by Xiang Weikang
///////////////////////////////////////////

#include "GeometryGenerator.h"

namespace RenderDog
{
	void GeometryGenerator::GenerateBox(float width, float height, float depth, LocalMeshData& outputMesh)
	{
		outputMesh.vertices.clear();
		outputMesh.indices.clear();

		float w = 0.5f * width;
		float h = 0.5f * height;
		float d = 0.5f * depth;

		RenderDog::LocalVertex vertices[24];
		//Front								  //Position    //Color		               //Normal		        //Tangent            //TexCoord
		vertices[0] = RenderDog::LocalVertex(-w, d, h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		vertices[1] = RenderDog::LocalVertex(w, d, h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		vertices[2] = RenderDog::LocalVertex(w, d, -h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
		vertices[3] = RenderDog::LocalVertex(-w, d, -h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		//Back		 									   		   		 	  
		vertices[4] = RenderDog::LocalVertex(w, -d, h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		vertices[5] = RenderDog::LocalVertex(-w, -d, h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		vertices[6] = RenderDog::LocalVertex(-w, -d, -h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
		vertices[7] = RenderDog::LocalVertex(w, -d, -h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		//Left										   	   				 	  
		vertices[8] = RenderDog::LocalVertex(-w, -d, h, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
		vertices[9] = RenderDog::LocalVertex(-w, d, h, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
		vertices[10] = RenderDog::LocalVertex(-w, d, -h, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);
		vertices[11] = RenderDog::LocalVertex(-w, -d, -h, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
		//Right										   	   		 		 	  
		vertices[12] = RenderDog::LocalVertex(w, d, h, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
		vertices[13] = RenderDog::LocalVertex(w, -d, h, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);
		vertices[14] = RenderDog::LocalVertex(w, -d, -h, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f);
		vertices[15] = RenderDog::LocalVertex(w, d, -h, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
		//Top												   	    	 	  
		vertices[16] = RenderDog::LocalVertex(-w, -d, h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		vertices[17] = RenderDog::LocalVertex(w, -d, h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		vertices[18] = RenderDog::LocalVertex(w, d, h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
		vertices[19] = RenderDog::LocalVertex(-w, d, h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		//Bottom								    			   			  
		vertices[20] = RenderDog::LocalVertex(-w, d, -h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		vertices[21] = RenderDog::LocalVertex(w, d, -h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		vertices[22] = RenderDog::LocalVertex(w, -d, -h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
		vertices[23] = RenderDog::LocalVertex(-w, -d, -h, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

		outputMesh.vertices.assign(std::begin(vertices), std::end(vertices));

		unsigned int indices[36] =
		{
			//Front
			0,  1,  3,  1,  2,  3,
			//Back
			4,  5,  7,  5,  6,  7,
			//Left
			8,  9, 11,  9, 10, 11,
			//Right
			12, 13, 15, 13, 14, 15,
			//Top
			16, 17, 19, 17, 18, 19,
			//Bottom
			20, 21, 23, 21, 22, 23
		};

		outputMesh.indices.assign(std::begin(indices), std::end(indices));
	}

	void GeometryGenerator::GenerateGridLine(int width, int depth, float gridUnit, const RenderDog::Vector4& lineColor, SimpleMeshData& outputMesh)
	{
		outputMesh.vertices.clear();
		outputMesh.indices.clear();

		float widthLen = width * gridUnit;
		float depthLen = depth * gridUnit;

		float nearLeftX = -widthLen * 0.5f;
		float nearLeftY = -depthLen * 0.5f;

		outputMesh.vertices.reserve(2 * width + 2);
		outputMesh.indices.reserve(2 * width + 2);
		//横向线
		for (int i = 0; i < width + 1; ++i)
		{
			float x1 = nearLeftX + i * gridUnit;
			float y1 = nearLeftY;
			float x2 = x1;
			float y2 = nearLeftY + depth * gridUnit;

			//线端的两个端点
			outputMesh.vertices.push_back({ x1, -y1, 0, lineColor.x, lineColor.y, lineColor.z, lineColor.w });
			outputMesh.vertices.push_back({ x2, -y2, 0, lineColor.x, lineColor.y, lineColor.z, lineColor.w });

			outputMesh.indices.push_back(2 * i);
			outputMesh.indices.push_back(2 * i + 1);
		}

		//纵向线
		for (int i = 0; i < depth + 1; ++i)
		{
			float x1 = nearLeftX;
			float y1 = nearLeftY + i * gridUnit;
			float x2 = nearLeftX + width * gridUnit;
			float y2 = y1;

			outputMesh.vertices.push_back({ x1, -y1, 0, lineColor.x, lineColor.y, lineColor.z, lineColor.w });
			outputMesh.vertices.push_back({ x2, -y2, 0, lineColor.x, lineColor.y, lineColor.z, lineColor.w });

			outputMesh.indices.push_back(2 * i);
			outputMesh.indices.push_back(2 * i + 1);
		}
	}

	GeometryGenerator	g_GeometryGenerator;
	GeometryGenerator*	g_pGeometryGenerator = &g_GeometryGenerator;

}// namespace RenderDog