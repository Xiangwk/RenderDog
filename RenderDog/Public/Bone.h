////////////////////////////////////////
//RenderDog <·,·>
//FileName: Bone.h
//Written by Xiang Weikang
////////////////////////////////////////

#pragma once

#include "Matrix.h"

namespace RenderDog
{
	class Bone
	{
	public:
		Bone();
		~Bone();

		

	private:
		int			m_ParentIndex;			//父节点索引为-1则表示当前骨骼为根骨骼

		Matrix4x4	m_OffsetMatrix;
		Matrix4x4	m_UpToParentMatrix;
		Matrix4x4	m_UpToRootMatrix;
	};

}// namespace RenderDog
