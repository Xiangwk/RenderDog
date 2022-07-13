///////////////////////////////////////////
//RenderDog <·,·>
//FileName: FbxImporter.h
//Written by Xiang Weikang
///////////////////////////////////////////

#pragma once

#include "fbxsdk.h"
#include "Vector.h"
#include "Matrix.h"

#include <vector>
#include <string>

namespace RenderDog
{
	class RDFbxImporter
	{
	public:
		struct RawMeshData 
		{
			std::string					name;
			std::vector<Vector3>		postions;
			std::vector<Vector2>		texcoords;
			std::vector<uint32_t>		smoothGroup;		//每个三角形所在的光滑组索引
			std::vector<Vector4>		boneIndices;
			std::vector<Vector4>		boneWeighs;
			
			RawMeshData() :
				name(),
				postions(0),
				texcoords(0),
				smoothGroup(0),
				boneIndices(0),
				boneWeighs(0)
			{}
		};

		struct RawBoneData
		{
			std::string					name;
			int							index;
			int							parentIndex;
			Matrix4x4					upToParentMatrix;
			Matrix4x4					offsetMatrix;

			RawBoneData() :
				name(""),
				index(0),
				parentIndex(-1),
				upToParentMatrix(),
				offsetMatrix()
			{}
		};

		struct RawSkeletonData
		{
			std::vector<RawBoneData*>	bones;

			RawSkeletonData() :
				bones(0)
			{}

			RawBoneData* GetBone(const std::string& name);
		};

	public:
		RDFbxImporter();
		~RDFbxImporter();

		bool								Init();
		void								Release();

											//FlipUV为true时，v坐标要做反转（v = 1.0f - v）
		bool								LoadFbxFile(const std::string& filePath, bool bIsSkinModel, bool bFlipUV = false);	

		std::vector<RawMeshData>&			GetRawMeshData() { return m_RawData; }
		const RawSkeletonData*				GetRawSkeletonData() const { return m_pSkeleton; }

	private:
		bool								ProcessMeshNode(FbxNode* pNode, bool bFlipUV = false);
		bool								GetMeshData(FbxNode* pNode, bool bFlipUV = false);

		void								ProcessSkeletonNode(FbxNode* pNode, RawBoneData* pParentBone);
		void								GetOffsetMatrix(FbxSkin* pSkin);

		void								GetTriangleMaterialIndices(FbxMesh* pMesh, int triNum, std::vector<uint32_t>& outputIndices);
		void								GetTriangleSmoothIndices(FbxMesh* pMesh, int triNum, std::vector<uint32_t>& outputIndices);
		void								ReadPositions(FbxMesh* pMesh, int vertexIndex, Vector3& outputPos);
		void								ReadTexcoord(FbxMesh* pMesh, int vertexIndex, int textureUVIndex, int uvLayer, Vector2& outputUV);
		void								ReadBoneSkin(FbxSkin* pSkin, int vertexIndex, std::vector<std::string>& outputBone, std::vector<float>& outputWeights);

	private:
		FbxManager*							m_pManager;
		FbxScene*							m_pScene;
		FbxImporter*						m_pImporter;

		std::vector<RawMeshData>			m_RawData;
		RawSkeletonData*					m_pSkeleton;
		bool								m_bNeedGetBoneMatrix;
	};

	extern RDFbxImporter* g_pRDFbxImporter;

}// namespace RenderDog