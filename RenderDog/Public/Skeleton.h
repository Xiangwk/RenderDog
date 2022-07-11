////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Skeleton.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include <vector>

namespace RenderDog
{
	class Bone;

	class Skeleton
	{
	public:
		Skeleton();
		~Skeleton();

		Bone*				GetBone(uint32_t index) { return m_Bones[index]; }

		uint32_t			GetBoneNum() const { return (uint32_t)m_Bones.size(); }

	private:
		std::vector<Bone*>	m_Bones;
	};

}// namespace RenderDog
