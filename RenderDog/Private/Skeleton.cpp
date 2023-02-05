////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Skeleton.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Skeleton.h"
#include "Bone.h"
#include "BoneAnimation.h"

namespace RenderDog
{
	Skeleton::Skeleton() :
		m_Bones(0),
		m_LocalMatrix()
	{}

	Skeleton::Skeleton(const Skeleton& skel) :
		m_Bones(skel.m_Bones),
		m_LocalMatrix(skel.m_LocalMatrix)
	{}

	Skeleton::~Skeleton()
	{}

	Skeleton& Skeleton::operator=(const Skeleton& skel)
	{
		m_Bones = skel.m_Bones;
		m_LocalMatrix = skel.m_LocalMatrix;

		return *this;
	}

	void Skeleton::Update()
	{
		for (size_t i = 0; i < m_Bones.size(); ++i)
		{
			Bone& bone = m_Bones[i];
			const Matrix4x4& upToParentMatrix = bone.GetUpToParentMatrix();
			const Matrix4x4& offsetMatrix = bone.GetOffsetMatrix();

			int parentIndex = bone.GetParentIndex();
			if (parentIndex == -1)
			{
				Matrix4x4 upToRootMatrix = upToParentMatrix * m_LocalMatrix;
				bone.SetUpdateToRootMatrix(upToRootMatrix);

				bone.SetFinalTransformMatrix(upToRootMatrix);
			}
			else
			{
				const Bone& parentBone = m_Bones[parentIndex];
				const Matrix4x4& parentToRootMatrix = parentBone.GetUpToRootMatrix();

				Matrix4x4 upToRootMatrix = upToParentMatrix * parentToRootMatrix;
				bone.SetUpdateToRootMatrix(upToRootMatrix);

				Matrix4x4 finalTranformMatrix = offsetMatrix * upToRootMatrix;
				bone.SetFinalTransformMatrix(finalTranformMatrix);
			}
		}
	}

	void Skeleton::UpdateByAnimation(float timePos, BoneAnimationClip& animClip)
	{
		animClip.Interpolate(timePos);

		//Update UpToRootMatrix
		for (size_t i = 0; i < m_Bones.size(); ++i)
		{
			Bone& bone = m_Bones[i];

			int boneAnimIndex = animClip.GetBoneAnimIndexByName(bone.GetName());
			Matrix4x4 upToParentMatrix;
			if (boneAnimIndex != -1)
			{
				upToParentMatrix = animClip.GetBoneAnimTransform(boneAnimIndex);
			}
			else
			{
				upToParentMatrix = bone.GetUpToParentMatrix();
			}

			int parentIndex = bone.GetParentIndex();
			if (parentIndex == -1)
			{
				Matrix4x4 upToRootMatrix = upToParentMatrix * m_LocalMatrix;
				bone.SetUpdateToRootMatrix(upToRootMatrix);
			}
			else
			{
				const Bone& parentBone = m_Bones[parentIndex];
				const Matrix4x4& parentToRootMatrix = parentBone.GetUpToRootMatrix();

				Matrix4x4 upToRootMatrix = upToParentMatrix * parentToRootMatrix;
				bone.SetUpdateToRootMatrix(upToRootMatrix);
			}
		}

		for (size_t i = 0; i < m_Bones.size(); ++i)
		{
			Bone& bone = m_Bones[i];
			
			const Matrix4x4& offsetMatrix = bone.GetOffsetMatrix();
			Matrix4x4 upToRootMatrix = bone.GetUpToRootMatrix();

			Matrix4x4 finalTranformMatrix = offsetMatrix * upToRootMatrix;
			bone.SetFinalTransformMatrix(finalTranformMatrix);
		}
	}

}// namespace RenderDog