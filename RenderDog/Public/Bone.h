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

		Bone(int parentIndex, const Matrix4x4& offsetMatrix, const Matrix4x4& upToParentMatrix) :
			m_ParentIndex(parentIndex),
			m_OffsetMatrix(offsetMatrix),
			m_UpToParentMatrix(upToParentMatrix)
		{}

		int					GetParentIndex() const { return m_ParentIndex; }

		void				SetUpdateToParentMatrix(const Matrix4x4& matrix) { m_UpToParentMatrix = matrix; }
		void				SetUpdateToRootMatrix(const Matrix4x4& matrix) { m_UpToRootMatrix = matrix; }
		
		const Matrix4x4&	GetUpToParentMatrix() const { return m_UpToParentMatrix; }
		const Matrix4x4&	GetUpToRootMatrix() const { return m_UpToRootMatrix; }
		const Matrix4x4&	GetOffsetMatrix() const { return m_OffsetMatrix; }

	private:
		int					m_ParentIndex;			//父节点索引为-1则表示当前骨骼为根骨骼

		Matrix4x4			m_OffsetMatrix;
		Matrix4x4			m_UpToParentMatrix;
		Matrix4x4			m_UpToRootMatrix;
	};

}// namespace RenderDog
