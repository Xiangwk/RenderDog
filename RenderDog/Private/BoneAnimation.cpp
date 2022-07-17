////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: BoneAnimation.cpp
//Written by Xiang Weikang
////////////////////////////////////////

#include "BoneAnimation.h"
#include "Transform.h"

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
			Vector4 rotationQuat = m_KeyFrames.front().rotationQuat;

			outputUpToParent = GetTransformation(translation, scales, rotationQuat);
		}
		else if (timePos >= m_KeyFrames.back().timePos)
		{
			Vector3 translation = m_KeyFrames.back().translation;
			Vector3 scales = m_KeyFrames.back().scales;
			Vector4 rotationQuat = m_KeyFrames.back().rotationQuat;

			outputUpToParent = GetTransformation(translation, scales, rotationQuat);
		}
		else
		{

		}
	}

}// namespace RenderDog