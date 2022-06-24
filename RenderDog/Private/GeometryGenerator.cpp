///////////////////////////////////////////
//ModelViewer
//FileName: GeometryGenerator.cpp
//Written by Xiang Weikang
///////////////////////////////////////////

#include "GeometryGenerator.h"
#include "Utility.h"

namespace RenderDog
{
	GeometryGenerator	g_GeometryGenerator;
	GeometryGenerator*	g_pGeometryGenerator = &g_GeometryGenerator;

	void GeometryGenerator::GenerateGrid(int width, int depth, float gridUnit, StandardMeshData& outputMesh)
	{
		outputMesh.vertices.clear();
		outputMesh.indices.clear();

		float WidthLen = width * gridUnit;
		float DepthLen = depth * gridUnit;

		float NearLeftX = -WidthLen * 0.5f;
		float NearLeftZ = -DepthLen * 0.5f;

		for (int i = 0; i < depth + 1; ++i)
		{
			for (int j = 0; j < width + 1; ++j)
			{
				float x = NearLeftX + j * gridUnit;
				float z = NearLeftZ + i * gridUnit;
				float u = static_cast<float>(j);
				float v = static_cast<float>(depth - i);
				Vector3 pos			= Vector3(x, 0.0f, z);
				Vector4 color		= Vector4(1.0f, 1.0f, 1.0f, 1.0f);
				Vector3 normal		= Vector3(0.0f, 1.0f, 0.0f);
				Vector4 tangent		= Vector4(1.0f, 0.0f, 0.0f, 1.0f);
				Vector2 texcoord	= Vector2(u, v);
				outputMesh.vertices.push_back(StandardVertex(pos, color, normal, tangent, texcoord));
			}
		}

		for (int i = 0; i < depth; ++i)
		{
			for (int j = 0; j < width; ++j)
			{
				outputMesh.indices.push_back(j + i * (width + 1));
				outputMesh.indices.push_back(j + (i + 1) * (width + 1));
				outputMesh.indices.push_back(j + (i + 1) * (width + 1) + 1);

				outputMesh.indices.push_back(j + i * (width + 1));
				outputMesh.indices.push_back(j + (i + 1) * (width + 1) + 1);
				outputMesh.indices.push_back((j + 1) + i * (width + 1));
			}
		}
	}

	void GeometryGenerator::GenerateGridLine(int width, int depth, float gridUnit, const Vector4& lineColor, SimpleMeshData& outputMesh)
	{
		outputMesh.vertices.clear();
		outputMesh.indices.clear();

		outputMesh.vertices.reserve(2 * width + 2);
		outputMesh.indices.reserve(2 * width + 2);

		float widthLen = width * gridUnit;
		float depthLen = depth * gridUnit;

		float nearLeftX = -widthLen * 0.5f;
		float nearLeftY = -depthLen * 0.5f;

		//横向线
		for (int i = 0; i < width + 1; ++i)
		{
			float x1 = nearLeftX + i * gridUnit;
			float y1 = nearLeftY;
			float x2 = x1;
			float y2 = nearLeftY + depth * gridUnit;

			//线端的两个端点
			outputMesh.vertices.push_back({ x1, 0, -y1, lineColor.x, lineColor.y, lineColor.z, lineColor.w });
			outputMesh.vertices.push_back({ x2, 0, -y2, lineColor.x, lineColor.y, lineColor.z, lineColor.w });

			outputMesh.indices.push_back(2 * i);
			outputMesh.indices.push_back(2 * i + 1);
		}

		int indiceNum = static_cast<int>(outputMesh.indices.size());

		//纵向线
		for (int i = 0; i < depth + 1; ++i)
		{
			float x1 = nearLeftX;
			float y1 = nearLeftY + i * gridUnit;
			float x2 = nearLeftX + width * gridUnit;
			float y2 = y1;

			outputMesh.vertices.push_back({ x1, 0, -y1, lineColor.x, lineColor.y, lineColor.z, lineColor.w });
			outputMesh.vertices.push_back({ x2, 0, -y2, lineColor.x, lineColor.y, lineColor.z, lineColor.w });

			outputMesh.indices.push_back(indiceNum + 2 * i);
			outputMesh.indices.push_back(indiceNum + 2 * i + 1);
		}
	}

	void GeometryGenerator::GenerateBox(float width, float height, float depth, StandardMeshData& outputMesh)
	{
		outputMesh.vertices.clear();
		outputMesh.indices.clear();

		float w = 0.5f * width;
		float h = 0.5f * height;
		float d = 0.5f * depth;

		StandardVertex vertices[24];
		//Front							//Position			//Color		                //Normal					//Tangent					//TexCoord
		vertices[0]		= StandardVertex(-w, -h, -d,		1.0f, 1.0f, 1.0f, 1.0f,		 0.0f,  0.0f, -1.0f,		 1.0f,  0.0f,  0.0f,		0.0f, 1.0f);
		vertices[1]		= StandardVertex(-w,  h, -d,		1.0f, 1.0f, 1.0f, 1.0f,		 0.0f,  0.0f, -1.0f,		 1.0f,  0.0f,  0.0f,		0.0f, 0.0f);
		vertices[2]		= StandardVertex( w,  h, -d,		1.0f, 1.0f, 1.0f, 1.0f,		 0.0f,  0.0f, -1.0f,		 1.0f,  0.0f,  0.0f,		1.0f, 0.0f);
		vertices[3]		= StandardVertex( w, -h, -d,		1.0f, 1.0f, 1.0f, 1.0f,		 0.0f,  0.0f, -1.0f,		 1.0f,  0.0f,  0.0f,		1.0f, 1.0f);
		//Back			 									   			   		 	  				    								   
		vertices[4]		= StandardVertex( w, -h,  d,		1.0f, 1.0f, 1.0f, 1.0f,		 0.0f,  0.0f,  1.0f,		-1.0f,  0.0f,  0.0f,		0.0f, 1.0f);
		vertices[5]		= StandardVertex( w,  h,  d,		1.0f, 1.0f, 1.0f, 1.0f,		 0.0f,  0.0f,  1.0f,		-1.0f,  0.0f,  0.0f,		0.0f, 0.0f);
		vertices[6]		= StandardVertex(-w,  h,  d,		1.0f, 1.0f, 1.0f, 1.0f,		 0.0f,  0.0f,  1.0f,		-1.0f,  0.0f,  0.0f,		1.0f, 0.0f);
		vertices[7]		= StandardVertex(-w, -h,  d,		1.0f, 1.0f, 1.0f, 1.0f,		 0.0f,  0.0f,  1.0f,		-1.0f,  0.0f,  0.0f,		1.0f, 1.0f);
		//Left										   	   					 	  					    								   
		vertices[8]		= StandardVertex(-w, -h,  d,		1.0f, 1.0f, 1.0f, 1.0f,		-1.0f,  0.0f,  0.0f,		 0.0f,  0.0f, -1.0f,		0.0f, 1.0f);
		vertices[9]		= StandardVertex(-w,  h,  d,		1.0f, 1.0f, 1.0f, 1.0f,		-1.0f,  0.0f,  0.0f,		 0.0f,  0.0f, -1.0f,		0.0f, 0.0f);
		vertices[10]	= StandardVertex(-w,  h, -d,		1.0f, 1.0f, 1.0f, 1.0f,		-1.0f,  0.0f,  0.0f,		 0.0f,  0.0f, -1.0f,		1.0f, 0.0f);
		vertices[11]	= StandardVertex(-w, -h, -d,		1.0f, 1.0f, 1.0f, 1.0f,		-1.0f,  0.0f,  0.0f,		 0.0f,  0.0f, -1.0f,		1.0f, 1.0f);
		//Right										   	   			 		 	  					    	   							   
		vertices[12]	= StandardVertex( w, -h, -d,		1.0f, 1.0f, 1.0f, 1.0f,		 1.0f,  0.0f,  0.0f,		 0.0f,  0.0f,  1.0f,		0.0f, 1.0f);
		vertices[13]	= StandardVertex( w,  h, -d,		1.0f, 1.0f, 1.0f, 1.0f,		 1.0f,  0.0f,  0.0f,		 0.0f,  0.0f,  1.0f,		0.0f, 0.0f);
		vertices[14]	= StandardVertex( w,  h,  d,		1.0f, 1.0f, 1.0f, 1.0f,		 1.0f,  0.0f,  0.0f,		 0.0f,  0.0f,  1.0f,		1.0f, 0.0f);
		vertices[15]	= StandardVertex( w, -h,  d,		1.0f, 1.0f, 1.0f, 1.0f,		 1.0f,  0.0f,  0.0f,		 0.0f,  0.0f,  1.0f,		1.0f, 1.0f);
		//Top														   	    	 	  				    								   
		vertices[16]	= StandardVertex(-w,  h, -d,		1.0f, 1.0f, 1.0f, 1.0f,		 0.0f,  1.0f,  0.0f,		 1.0f,  0.0f,  0.0f,		0.0f, 1.0f);
		vertices[17]	= StandardVertex(-w,  h,  d,		1.0f, 1.0f, 1.0f, 1.0f,		 0.0f,  1.0f,  0.0f,		 1.0f,  0.0f,  0.0f,		0.0f, 0.0f);
		vertices[18]	= StandardVertex( w,  h,  d,		1.0f, 1.0f, 1.0f, 1.0f,		 0.0f,  1.0f,  0.0f,		 1.0f,  0.0f,  0.0f,		1.0f, 0.0f);
		vertices[19]	= StandardVertex( w,  h, -d,		1.0f, 1.0f, 1.0f, 1.0f,		 0.0f,  1.0f,  0.0f,		 1.0f,  0.0f,  0.0f,		1.0f, 1.0f);
		//Bottom									    				   			  			 		 					    		   
		vertices[20]	= StandardVertex(-w, -h,  d,		1.0f, 1.0f, 1.0f, 1.0f,		 0.0f, -1.0f,  0.0f,		 1.0f,  0.0f,  0.0f,		0.0f, 1.0f);
		vertices[21]	= StandardVertex(-w, -h, -d,		1.0f, 1.0f, 1.0f, 1.0f,		 0.0f, -1.0f,  0.0f,		 1.0f,  0.0f,  0.0f,		0.0f, 0.0f);
		vertices[22]	= StandardVertex( w, -h, -d,		1.0f, 1.0f, 1.0f, 1.0f,		 0.0f, -1.0f,  0.0f,		 1.0f,  0.0f,  0.0f,		1.0f, 0.0f);
		vertices[23]	= StandardVertex( w, -h,  d,		1.0f, 1.0f, 1.0f, 1.0f,		 0.0f, -1.0f,  0.0f,		 1.0f,  0.0f,  0.0f,		1.0f, 1.0f);

		outputMesh.vertices.assign(std::begin(vertices), std::end(vertices));

		unsigned int indices[36] =
		{
			//Front
			0,  1,  2,  0,  2,  3,
			//Back
			4,  5,  6,  4,  6,  7,
			//Left
			8,  9,  10, 8, 10, 11,
			//Right
			12, 13, 14, 12, 14, 15,
			//Top
			16, 17, 18, 16, 18, 19,
			//Bottom
			20, 21, 22, 20, 22, 23
		};

		outputMesh.indices.assign(std::begin(indices), std::end(indices));
	}

	void GeometryGenerator::GenerateSphere(uint32_t longitudeNum, uint32_t latitudeNum, float radius, StandardMeshData& outputMesh)
	{
		outputMesh.vertices.clear();
		outputMesh.indices.clear();

		//Vertice
		//Top Pole
		Vector3 topPolePos = Vector3(0.0f, radius, 0.0f);

		StandardVertex topPoleVert;
		topPoleVert.position	= topPolePos;
		topPoleVert.color		= Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		topPoleVert.normal		= Vector3(0.0f, 1.0f, 0.0f);
		topPoleVert.tangent		= Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		for (uint32_t i = 0; i < longitudeNum; ++i)
		{
			topPoleVert.texcoord = Vector2((float)i / longitudeNum, 0.0f);
			outputMesh.vertices.push_back(topPoleVert);
		}

		//Rings
		for (uint32_t i = 0; i < latitudeNum - 1; ++i)
		{
			float alpha = (i + 1) * (RD_PI / latitudeNum);
			float ringRadius = radius * sinf(alpha);
			float ringY = radius * cosf(alpha);
			for (uint32_t j = 0; j < longitudeNum + 1; ++j)
			{
				float beta = j * (2.0f * RD_PI / longitudeNum);

				Vector3 position = Vector3(ringRadius * cosf(beta), ringY, ringRadius * sinf(beta));
				Vector4 color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
				Vector3 normal = Normalize(position);
				Vector3 tangent = Vector3(-position.z, 0.0f, position.x);
				tangent = Normalize(tangent);
				Vector2 texcoord = Vector2((float)j / longitudeNum, (float)(i + 1) / latitudeNum);

				outputMesh.vertices.push_back({ position, color, normal, Vector4(tangent, 1.0f), texcoord });
			}
		}

		//Bottom
		Vector3 bottomPolePos = Vector3(0.0f, -radius, 0.0f);

		StandardVertex bottomPoleVert;
		bottomPoleVert.position = bottomPolePos;
		bottomPoleVert.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		bottomPoleVert.normal = Vector3(0.0f, -1.0f, 0.0f);
		bottomPoleVert.tangent = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		for (uint32_t i = 0; i < longitudeNum; ++i)
		{
			bottomPoleVert.texcoord = Vector2((float)i / longitudeNum, 1.0f);
			outputMesh.vertices.push_back(bottomPoleVert);
		}
		

		//Indices
		//Top Cap
		for (uint32_t i = 0; i < longitudeNum; ++i)
		{
			outputMesh.indices.push_back(i);
			outputMesh.indices.push_back(i + longitudeNum + 1);
			outputMesh.indices.push_back(i + longitudeNum);
		}

		//Mid
		for (uint32_t i = 0; i < latitudeNum - 2; ++i)
		{
			for (uint32_t j = 0; j < longitudeNum; ++j)
			{
				outputMesh.indices.push_back(j + longitudeNum + (i + 1) * (longitudeNum + 1));
				outputMesh.indices.push_back(j + longitudeNum + i * (longitudeNum + 1));
				outputMesh.indices.push_back(j + longitudeNum + i * (longitudeNum + 1) + 1);

				outputMesh.indices.push_back(j + longitudeNum + (i + 1) * (longitudeNum + 1));
				outputMesh.indices.push_back(j + longitudeNum + i * (longitudeNum + 1) + 1);
				outputMesh.indices.push_back(j + longitudeNum + (i + 1) * (longitudeNum + 1) + 1);
			}
		}

		//Bottom
		for (uint32_t i = 0; i < longitudeNum; ++i)
		{
			outputMesh.indices.push_back((uint32_t)outputMesh.vertices.size() - 1 - longitudeNum + 1 + i);
			outputMesh.indices.push_back((uint32_t)outputMesh.vertices.size() - 1 - longitudeNum - longitudeNum + i);
			outputMesh.indices.push_back((uint32_t)outputMesh.vertices.size() - 1 - longitudeNum - longitudeNum + i + 1);
		}
	}

}// namespace RenderDog