////////////////////////////////////////
//RenderDog <·,·>
//FileName: BoneAnimation.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "BoneAnimation.h"
#include "Transform.h"

#include <cmath>

namespace RenderDog
{
	float BoneAnimation::GetStartTime() const
	{
		return m_KeyFrames.front().timePos;
	}

	float BoneAnimation::GetEndTime() const
	{
		return m_KeyFrames.back().timePos;
	}

	void BoneAnimation::Interpolate(float timePos, Matrix4x4& outputUpToParent) const
	{
		if (timePos <= m_KeyFrames.front().timePos)
		{
			Vector3 translation = m_KeyFrames.front().translation;
			Vector3 scales = m_KeyFrames.front().scales;
			Quaternion rotationQuat = m_KeyFrames.front().rotationQuat;
			//Vector3 eulers = m_KeyFrames.front().eulers;

			outputUpToParent = GetTransformation(translation, scales, rotationQuat);
		}
		else if (timePos >= m_KeyFrames.back().timePos)
		{
			Vector3 translation = m_KeyFrames.back().translation;
			Vector3 scales = m_KeyFrames.back().scales;
			Quaternion rotationQuat = m_KeyFrames.back().rotationQuat;
			//Vector3 eulers = m_KeyFrames.back().eulers;

			outputUpToParent = GetTransformation(translation, scales, rotationQuat);
		}
		else
		{
			for (size_t i = 0; i < m_KeyFrames.size() - 1; ++i)
			{
				//找到当前的时间点位于哪两个关键帧之间
				if (timePos >= m_KeyFrames[i].timePos && timePos <= m_KeyFrames[i + 1].timePos)
				{
					float lerpFactor = (timePos - m_KeyFrames[i].timePos) / (m_KeyFrames[i + 1].timePos - m_KeyFrames[i].timePos);

					Vector3 scales0 = m_KeyFrames[i].scales;
					Vector3 scales1 = m_KeyFrames[i + 1].scales;
					Vector3 currScales = Lerp(scales0, scales1, lerpFactor);

					Quaternion rotationQuat0 = m_KeyFrames[i].rotationQuat;
					Quaternion rotationQuat1 = m_KeyFrames[i + 1].rotationQuat;
					Quaternion currRotationQuat = SLerp(rotationQuat0, rotationQuat1, lerpFactor);
					
					/*Vector3 eulers0 = m_KeyFrames[i].eulers;
					Vector3 eulers1 = m_KeyFrames[i + 1].eulers;
					Vector3 currEulers = Lerp(eulers0, eulers1, lerpFactor);*/

					Vector3 translation0 = m_KeyFrames[i].translation;
					Vector3 translation1 = m_KeyFrames[i + 1].translation;
					Vector3 currTranslation = Lerp(translation0, translation1, lerpFactor);

					outputUpToParent = GetTransformation(currTranslation, currScales, currRotationQuat);

					break;
				}
			}
		}
	}

	float BoneAnimationClip::GetClipStartTime() const
	{
		float minStartTime = FLT_MAX;
		for (size_t i = 0; i < m_BoneAnimations.size(); ++i)
		{
			minStartTime = std::min(minStartTime, m_BoneAnimations[i].GetStartTime());
		}

		return minStartTime;
	}

	float BoneAnimationClip::GetClipEndTime() const
	{
		float maxEndTime = 0.0f;
		for (size_t i = 0; i < m_BoneAnimations.size(); ++i)
		{
			maxEndTime = std::max(maxEndTime, m_BoneAnimations[i].GetEndTime());
		}

		return maxEndTime;
	}

	void BoneAnimationClip::Interpolate(float timePos)
	{
		m_BoneAnimTransforms.resize(m_BoneAnimations.size());

		for (size_t i = 0; i < m_BoneAnimations.size(); ++i)
		{
			m_BoneAnimations[i].Interpolate(timePos, m_BoneAnimTransforms[i]);
		}
	}

	int	BoneAnimationClip::GetBoneAnimIndexByName(const std::string& boneName) const
	{
		int index = -1;

		auto iter = m_BoneAnimIndexMap.find(boneName);
		if (iter != m_BoneAnimIndexMap.end())
		{
			index = static_cast<int>(iter->second);
		}

		return index;
	}

	const Matrix4x4& BoneAnimationClip::GetBoneAnimTransform(int boneAnimIndex) const
	{
		return m_BoneAnimTransforms[boneAnimIndex];
	}

}// namespace RenderDog