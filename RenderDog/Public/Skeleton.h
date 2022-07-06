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


	private:
		std::vector<Bone*>	m_Bones;
	};

}// namespace RenderDog
