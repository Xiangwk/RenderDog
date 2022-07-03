///////////////////////////////////////////
//RenderDog <·,·>
//FileName: FbxImporter.h
//Written by Xiang Weikang
///////////////////////////////////////////

#pragma once

#include "fbxsdk.h"
#include "Vector.h"

#include <vector>

namespace RenderDog
{
	class RDFbxImporter
	{
	public:
		struct RawMeshData 
		{
			std::vector<Vector3>		postions;
			std::vector<Vector2>		texcoords;
			std::vector<uint32_t>		smoothGroup;		//每个三角形所在的光滑组索引

			RawMeshData() :
				postions(0),
				texcoords(0),
				smoothGroup(0)
			{}
		};

	public:
		RDFbxImporter();
		~RDFbxImporter();

		bool								Init();
		void								Release();

		bool								LoadFbxFile(const std::string& filePath);

		std::vector<RawMeshData>&			GetRawMeshData() { return m_RawData; }

	private:
		bool								ProcessNode(FbxNode* pNode);
		bool								ProcessMesh(FbxNode* pNode);

		void								GetTriangleMaterialIndices(FbxMesh* pMesh, int triNum, std::vector<uint32_t>& outputIndices);
		void								GetTriangleSmoothIndices(FbxMesh* pMesh, int triNum, std::vector<uint32_t>& outputIndices);
		void								ReadPositions(FbxMesh* pMesh, int ctrlPointIndex, Vector3& outputPos);
		void								ReadTexcoord(FbxMesh* pMesh, int ctrlPointIndex, int textureUVIndex, int uvLayer, Vector2& OutputUV);

	private:
		FbxManager*							m_pManager;
		FbxScene*							m_pScene;
		FbxImporter*						m_pImporter;

		std::vector<RawMeshData>			m_RawData;
	};

	extern RDFbxImporter* g_pRDFbxImporter;

}// namespace RenderDog