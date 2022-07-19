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
	public:
		enum class FBX_LOAD_TYPE
		{
			UNKNOWN = 0,
			STATIC_MODEL,
			SKIN_MODEL,
			ANIMATION
		};

		struct FbxLoadParam
		{
			bool bIsFlipTexcoordV;

			FbxLoadParam() :
				bIsFlipTexcoordV(false)
			{}
		};

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

		struct RawKeyFrameData
		{
			float		timePos;
			Vector3		translation;
			Vector3		scales;
			//Vector4		rotationQuat;
			Vector3		eulers;					//x, y, z

			RawKeyFrameData() :
				timePos(0.0f),
				translation(0.0f),
				scales(1.0f),
				//rotationQuat(0.0f)
				eulers(0.0f)
			{}
		};

		//某一根骨骼的动画
		struct RawBoneAnimation
		{
			std::string						boneName;
			std::vector<RawKeyFrameData>	keyFrames;

			RawBoneAnimation() :
				boneName(""),
				keyFrames(0)
			{}

			void Clear()
			{
				keyFrames.clear();
			}
		};

		struct RawAnimation
		{
			std::string									name;					//动画名
			std::vector<RawBoneAnimation>				boneAnimations;			//每一根骨骼的动画合集

			RawAnimation() :
				name(""),
				boneAnimations(0)
			{}
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
		bool								LoadFbxFile(const std::string& filePath, FBX_LOAD_TYPE loadType, const FbxLoadParam& loadParam = FbxLoadParam());

		std::vector<RawMeshData>&			GetRawMeshData() { return m_RawData; }
		const RawSkeletonData*				GetRawSkeletonData() const { return m_pSkeleton; }
		const RawAnimation&					GetRawBoneAnimation() const { return m_BoneAnimations; }

	private:
		bool								ProcessMeshNode(FbxNode* pNode, bool bIsFlipTexcoordV = false);
		bool								GetMeshData(FbxNode* pNode, bool bIsFlipTexcoordV = false);

		void								GetTriangleMaterialIndices(FbxMesh* pMesh, int triNum, std::vector<uint32_t>& outputIndices);
		void								GetTriangleSmoothIndices(FbxMesh* pMesh, int triNum, std::vector<uint32_t>& outputIndices);
		void								ReadPositions(FbxMesh* pMesh, int vertexIndex, Vector3& outputPos);
		void								ReadTexcoord(FbxMesh* pMesh, int vertexIndex, int textureUVIndex, int uvLayer, Vector2& outputUV);
		
		void								ProcessSkeletonNode(FbxNode* pNode, RawBoneData* pParentBone);
		void								GetBoneSkins(FbxSkin* pSkin, std::unordered_map<int, VertexBones>& vertBonesMap);
		void								GetOffsetMatrix(FbxSkin* pSkin);
		void								ReadBoneSkin(int vertexIndex, const std::unordered_map<int, VertexBones>& vertBonesMap, std::vector<std::string>& outputBone, std::vector<float>& outputWeights);

		void								ProcessAnimation(FbxNode* pNode);

	private:
		FbxManager*							m_pManager;
		FbxScene*							m_pScene;
		FbxImporter*						m_pImporter;

		bool								m_bNeedBoneSkin;
		std::vector<RawMeshData>			m_RawData;
		RawSkeletonData*					m_pSkeleton;
		RawAnimation						m_BoneAnimations;
	};

	extern RDFbxImporter* g_pRDFbxImporter;

}// namespace RenderDog