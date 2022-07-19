////////////////////////////////////////
//RenderDog <·,·>
//FileName: BoneAnimation.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "Vector.h"
#include "Matrix.h"

#include <vector>
#include <unordered_map>

namespace RenderDog
{
	struct BoneKeyFrame
	{
		float		timePos;
		Vector3		translation;
		Vector3		scales;
		//Vector4		rotationQuat;
		Vector3		eulers;				//x, y, z

		BoneKeyFrame() :
			timePos(0.0f),
			translation(0.0f),
			scales(1.0f),
			//rotationQuat(0.0f)
			eulers(0.0f)
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

		BoneAnimationClip(const std::string& name,
						  const std::vector<BoneAnimation>& boneAnimations,
						  const std::unordered_map<std::string, size_t>& boneAnimIndexMap) :
			m_Name(name),
			m_BoneAnimations(boneAnimations),
			m_BoneAnimIndexMap(boneAnimIndexMap)
		{
			m_AnimTimeLength = GetClipEndTime() - GetClipStartTime();
		}

		~BoneAnimationClip() = default;

		float										GetClipStartTime() const;
		float										GetClipEndTime() const;

		float										GetAnimTimeLength() const { return m_AnimTimeLength; }

		void										Interpolate(float timePos);

		int											GetBoneAnimIndexByName(const std::string& boneName) const;
		const Matrix4x4&							GetBoneAnimTransform(int boneAnimIndex) const;

	private:
		std::string									m_Name;
		float										m_AnimTimeLength;
		std::vector<BoneAnimation>					m_BoneAnimations;
		std::unordered_map<std::string, size_t>		m_BoneAnimIndexMap;

		std::vector<Matrix4x4>						m_BoneAnimTransforms;	//某一个时刻下所有带动画的骨骼的UpToParent矩阵
	};

}// namespace RenderDog
