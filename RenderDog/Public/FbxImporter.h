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
#include <unordered_map>

namespace RenderDog
{
	class RDFbxImporter
	{
		enum class FBX_LOAD_TYPE
		{
			UNKNOWN = 0,
			STATIC_MODEL,
			SKIN_MODEL,
			ANIMATION
		};

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
			{
				upToParentMatrix.Identity();
				offsetMatrix.Identity();
			}
		};

		struct RawSkeletonData
		{
			std::vector<RawBoneData*>						bones;
			std::unordered_map<std::string, RawBoneData*>	boneMap;

			Matrix4x4										LocalMatrix;

			RawSkeletonData() :
				bones(0)
			{}

			RawBoneData* GetBone(const std::string& name);
		};

		//影响该顶点的骨骼以及权重
		struct VertexBones
		{
			std::vector<std::string>	boneNames;
			std::vector<float>			boneWeights;
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
		void								GetBoneSkins(FbxSkin* pSkin, std::unordered_map<int, VertexBones>& vertBonesMap);
		void								GetOffsetMatrix(FbxSkin* pSkin);

		void								GetTriangleMaterialIndices(FbxMesh* pMesh, int triNum, std::vector<uint32_t>& outputIndices);
		void								GetTriangleSmoothIndices(FbxMesh* pMesh, int triNum, std::vector<uint32_t>& outputIndices);
		void								ReadPositions(FbxMesh* pMesh, int vertexIndex, Vector3& outputPos);
		void								ReadTexcoord(FbxMesh* pMesh, int vertexIndex, int textureUVIndex, int uvLayer, Vector2& outputUV);
		void								ReadBoneSkin(int vertexIndex, const std::unordered_map<int, VertexBones>& vertBonesMap, std::vector<std::string>& outputBone, std::vector<float>& outputWeights);

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