////////////////////////////////////////
//RenderDog <·,·>
//FileName: BoneAnimation.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "Vector.h"
#include "Matrix.h"

#include <vector>

namespace RenderDog
{
	struct BoneKeyFrame
	{
		float		timePos;
		Vector3		translation;
		Vector3		scales;
		Vector4		rotationQuat;

		BoneKeyFrame() :
			timePos(0.0f),
			translation(0.0f),
			scales(1.0f),
			rotationQuat(0.0f)
		{}
	};

	class BoneAnimation
	{
	public:
		BoneAnimation() :
			m_KeyFrames(0)
		{}

		~BoneAnimation() = default;

		float						GetStartTime() const;
		float						GetEndTime() const;

		void						Interpolate(float timePos, Matrix4x4& outputUpToParent) const;
		
		void						AddKeyFrame(const BoneKeyFrame& keyFrame) { m_KeyFrames.push_back(keyFrame); }

	private:
		std::vector<BoneKeyFrame>	m_KeyFrames;
	};

	//对应一个模型中所有骨骼的动画合集，表示某一个动作的动画
	class BoneAnimationClip
	{
	public:
		BoneAnimationClip() :
			m_BoneAnimations(0)
		{}

		~BoneAnimationClip() = default;

		float						GetClipStartTime() const;
		float						GetClipEndTime() const;

		void						Interpolate(float timePos, std::vector<Matrix4x4>& boneTransforms) const;

	private:
		std::vector<BoneAnimation>	m_BoneAnimations;
	};

}// namespace RenderDog
