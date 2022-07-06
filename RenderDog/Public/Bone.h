////////////////////////////////////////
//RenderDog <��,��>
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
		int			m_ParentIndex;			//���ڵ�����Ϊ-1���ʾ��ǰ����Ϊ������

		Matrix4x4	m_OffsetMatrix;
		Matrix4x4	m_UpToParentMatrix;
		Matrix4x4	m_UpToRootMatrix;
	};

}// namespace RenderDog
