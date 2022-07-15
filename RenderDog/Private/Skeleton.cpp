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

}// namespace RenderDog