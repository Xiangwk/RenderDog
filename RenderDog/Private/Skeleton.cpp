////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Skeleton.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "Skeleton.h"
#include "Bone.h"

namespace RenderDog
{
	Skeleton::Skeleton() :
		m_Bones(0)
	{}

	Skeleton::~Skeleton()
	{}

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
				bone.SetUpdateToRootMatrix(offsetMatrix * upToParentMatrix);
			}
			else
			{
				const Bone& parentBone = m_Bones[parentIndex];
				const Matrix4x4& parentUpToRootMatrix = parentBone.GetUpToRootMatrix();

				Matrix4x4 upToRootMatrix = offsetMatrix * upToParentMatrix * parentUpToRootMatrix;
				bone.SetUpdateToRootMatrix(upToRootMatrix);
			}
		}
	}

}// namespace RenderDog