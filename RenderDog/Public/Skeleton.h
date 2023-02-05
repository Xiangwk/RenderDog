////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Skeleton.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "Matrix.h"

#include <vector>

namespace RenderDog
{
	class Bone;
	class BoneAnimationClip;

	class Skeleton
	{
	public:
		Skeleton();
		Skeleton(const Skeleton& skel);
		~Skeleton();

		Skeleton& operator=(const Skeleton& skel);

		void				SetLocalMatrix(const Matrix4x4& matrix) { m_LocalMatrix = matrix; }

		Bone&				GetBone(uint32_t index) { return m_Bones[index]; }
		const Bone&			GetBone(uint32_t index) const { return m_Bones[index]; }

		void				AddBone(const Bone& bone) { m_Bones.push_back(bone); }

		uint32_t			GetBoneNum() const { return (uint32_t)m_Bones.size(); }

		void				Update();
		void				UpdateByAnimation(float timePos, BoneAnimationClip& animClip);

	private:
		std::vector<Bone>	m_Bones;

		Matrix4x4			m_LocalMatrix;
	};

}// namespace RenderDog
