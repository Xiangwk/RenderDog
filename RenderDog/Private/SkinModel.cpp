////////////////////////////////////////
//RenderDog <·,·>
//FileName: SkinModel.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "SkinModel.h"
#include "Scene.h"
#include "Matrix.h"
#include "Transform.h"
#include "Skeleton.h"
#include "Bone.h"
#include "Shader.h"

namespace RenderDog
{
	SkinModel::SkinModel():
		m_Meshes(0),
		m_pSkeleton(nullptr),
		m_AABB(),
		m_BoundingSphere()
	{}

	SkinModel::~SkinModel()
	{
		m_Meshes.clear();

		if (m_pSkeleton)
		{
			delete m_pSkeleton;
			m_pSkeleton = nullptr;
		}
	}

	bool SkinModel::LoadFromRawMeshData(const std::vector<RDFbxImporter::RawMeshData>& rawMeshDatas, const RDFbxImporter::RawSkeletonData* pSkeletonData, 
									    const std::string& vsFile, const std::string& psFile, 
										const std::string& modelName)
	{
		for (uint32_t i = 0; i < rawMeshDatas.size(); ++i)
		{
			const RDFbxImporter::RawMeshData& meshData = rawMeshDatas[i];

			std::vector<SkinVertex> vertices;
			vertices.reserve(meshData.postions.size());

			for (uint32_t index = 0; index < meshData.postions.size(); ++index)
			{
				SkinVertex vert;
				vert.position = meshData.postions[index];
				vert.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
				vert.normal = Vector3(0.0f, 0.0f, 0.0f);
				vert.tangent = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
				vert.texcoord = meshData.texcoords[index];

				vert.weights.x = meshData.boneWeighs[index].x;
				vert.weights.y = meshData.boneWeighs[index].y;
				vert.weights.z = meshData.boneWeighs[index].z;

				vert.boneIndices[0] = (uint8_t)meshData.boneIndices[index].x;
				vert.boneIndices[1] = (uint8_t)meshData.boneIndices[index].y;
				vert.boneIndices[2] = (uint8_t)meshData.boneIndices[index].z;
				vert.boneIndices[3] = (uint8_t)meshData.boneIndices[index].w;

				vertices.push_back(vert);
			}

			std::string meshName = modelName + "_" + meshData.name;
			SkinMesh mesh(meshName);
			mesh.CalcTangentsAndGenIndices(vertices, meshData.smoothGroup);

			m_Meshes.push_back(mesh);
		}

		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			SkinMesh& mesh = m_Meshes[i];
			mesh.InitRenderData();
		}

		CalculateBoundings();

		m_pSkeleton = new Skeleton;
		m_pSkeleton->SetLocalMatrix(pSkeletonData->LocalMatrix);
		//LoadSkeleton
		for (size_t i = 0; i < pSkeletonData->bones.size(); ++i)
		{
			RDFbxImporter::RawBoneData* pRawBone = pSkeletonData->bones[i];
			Bone bone(pRawBone->name, pRawBone->parentIndex, pRawBone->offsetMatrix, pRawBone->upToParentMatrix);

			m_pSkeleton->AddBone(bone);
		}

		m_pSkeleton->Update();

		return true;
	}

	bool SkinModel::LoadBoneAnimation(const RDFbxImporter::RawAnimation& rawAnimation)
	{
		std::vector<BoneAnimation> boneAnimations;
		boneAnimations.resize(rawAnimation.boneAnimations.size());
		std::unordered_map<std::string, size_t> boneAnimIndexMap;

		for (size_t i = 0; i < rawAnimation.boneAnimations.size(); ++i)
		{
			const RDFbxImporter::RawBoneAnimation* pRawBoneAnimation = rawAnimation.boneAnimations[i];
			BoneAnimation& newBoneAnimation = boneAnimations[i];
			for (size_t j = 0; j < pRawBoneAnimation->keyFrames.size(); ++j)
			{
				const RDFbxImporter::RawKeyFrameData& keyFrameData = pRawBoneAnimation->keyFrames[j];
				BoneKeyFrame boneKeyFrame;
				boneKeyFrame.timePos = keyFrameData.timePos;
				boneKeyFrame.translation = keyFrameData.translation;
				boneKeyFrame.scales = keyFrameData.scales;
				boneKeyFrame.rotationQuat = keyFrameData.rotationQuat;
				//boneKeyFrame.eulers = keyFrameData.eulers;

				newBoneAnimation.AddKeyFrame(boneKeyFrame);
			}

			boneAnimIndexMap.insert({ pRawBoneAnimation->boneName, i });
		}

		BoneAnimationClip boneAnimClip(rawAnimation.name, boneAnimations, boneAnimIndexMap);
		m_BoneAnimationClips.push_back(boneAnimClip);

		return true;
	}

	bool SkinModel::LoadTextureFromFile(const std::wstring& diffuseTexturePath, const std::wstring& normalTexturePath)
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			SkinMesh& mesh = m_Meshes[i];
			if (!mesh.LoadTextureFromFile(diffuseTexturePath, normalTexturePath))
			{
				return false;
			}
		}

		return true;
	}

	void SkinModel::RegisterToScene(IScene* pScene)
	{
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			IPrimitive* pMesh = &(m_Meshes[i]);
			pScene->RegisterPrimitive(pMesh);
		}

		//注册模型时要更新场景的包围球，默认场景的中心点在世界空间的原点
		BoundingSphere& sceneBoundingSphere = pScene->GetBoundingSphere();
		float modelMaxDisToSceneCenter = m_BoundingSphere.center.Length() + m_BoundingSphere.radius;
		sceneBoundingSphere.radius = sceneBoundingSphere.radius > modelMaxDisToSceneCenter ? sceneBoundingSphere.radius : modelMaxDisToSceneCenter;
	}

	void SkinModel::SetPosGesture(const Vector3& pos, const Vector3& euler, const Vector3& scale)
	{
		m_WorldPosition = pos;
		m_EulerAngle = euler;
		m_Scale = scale;

		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			SkinMesh* pMesh = &(m_Meshes[i]);
			pMesh->SetPosGesture(pos, euler, scale);
		}

		UpdateBoundings();
	}

	void SkinModel::Tick(float deltaTime)
	{
		SkinModelPerObjectTransform perModelTransform;
		
		if (m_pSkeleton->GetBoneNum() > g_MaxBoneNum)
		{
			return;
		}

		//FIXME!!! 以后在这里选择要播放的动作
		BoneAnimationClip& boneAnimClips = m_BoneAnimationClips[0];
		float animTimeLength = boneAnimClips.GetAnimTimeLength();
		static float animTime = 0.0f;
		if (animTime > animTimeLength)
		{
			animTime = 0.0f;
		}
		m_pSkeleton->UpdateByAnimation(animTime, boneAnimClips);
		animTime += (deltaTime * 1000.0f);	//deltaTime的单位是s，而动画时间的单位是ms

		for (uint32_t i = 0; i < m_pSkeleton->GetBoneNum(); ++i)
		{
			perModelTransform.BoneFinalTransformMatrix[i] = m_pSkeleton->GetBone(i).GetFinalTransformMatrix();
		}

		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			m_Meshes[i].Update(perModelTransform);
		}
	}

	void SkinModel::CalculateBoundings()
	{
		m_AABB.Reset();
		m_BoundingSphere.Reset();
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			m_Meshes[i].CalculateAABB();
			const AABB& meshAABB = m_Meshes[i].GetAABB();

			m_AABB.minPoint.x = m_AABB.minPoint.x < meshAABB.minPoint.x ? m_AABB.minPoint.x : meshAABB.minPoint.x;
			m_AABB.minPoint.y = m_AABB.minPoint.y < meshAABB.minPoint.y ? m_AABB.minPoint.y : meshAABB.minPoint.y;
			m_AABB.minPoint.z = m_AABB.minPoint.z < meshAABB.minPoint.z ? m_AABB.minPoint.z : meshAABB.minPoint.z;

			m_AABB.maxPoint.x = m_AABB.maxPoint.x > meshAABB.maxPoint.x ? m_AABB.maxPoint.x : meshAABB.maxPoint.x;
			m_AABB.maxPoint.y = m_AABB.maxPoint.y > meshAABB.maxPoint.y ? m_AABB.maxPoint.y : meshAABB.maxPoint.y;
			m_AABB.maxPoint.z = m_AABB.maxPoint.z > meshAABB.maxPoint.z ? m_AABB.maxPoint.z : meshAABB.maxPoint.z;
		}

		m_BoundingSphere.center = (m_AABB.minPoint + m_AABB.maxPoint) * 0.5f;
		m_BoundingSphere.radius = (m_AABB.maxPoint - m_AABB.minPoint).Length() * 0.5f;
	}
	
	void SkinModel::UpdateBoundings()
	{
		m_AABB.Reset();
		m_BoundingSphere.Reset();
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
		{
			const AABB& meshAABB = m_Meshes[i].GetAABB();

			m_AABB.minPoint.x = m_AABB.minPoint.x < meshAABB.minPoint.x ? m_AABB.minPoint.x : meshAABB.minPoint.x;
			m_AABB.minPoint.y = m_AABB.minPoint.y < meshAABB.minPoint.y ? m_AABB.minPoint.y : meshAABB.minPoint.y;
			m_AABB.minPoint.z = m_AABB.minPoint.z < meshAABB.minPoint.z ? m_AABB.minPoint.z : meshAABB.minPoint.z;

			m_AABB.maxPoint.x = m_AABB.maxPoint.x > meshAABB.maxPoint.x ? m_AABB.maxPoint.x : meshAABB.maxPoint.x;
			m_AABB.maxPoint.y = m_AABB.maxPoint.y > meshAABB.maxPoint.y ? m_AABB.maxPoint.y : meshAABB.maxPoint.y;
			m_AABB.maxPoint.z = m_AABB.maxPoint.z > meshAABB.maxPoint.z ? m_AABB.maxPoint.z : meshAABB.maxPoint.z;
		}

		m_BoundingSphere.center = (m_AABB.minPoint + m_AABB.maxPoint) * 0.5f;
		m_BoundingSphere.radius = (m_AABB.maxPoint - m_AABB.minPoint).Length() * 0.5f;
	}

}// namespace RenderDog