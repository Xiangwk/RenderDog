///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Viewport.cpp
//Written by Xiang Weikang
///////////////////////////////////

#include "Viewport.h"

#include "Matrix.h"

namespace RenderDog
{
	Matrix4x4 Viewport::GetViewportMatrix() const
	{
		Matrix4x4 matResult = GetIdentityMatrix();
		matResult(0, 0) = fWidth / 2.0f;
		matResult(3, 0) = fWidth / 2.0f + fTopLeftX;
		matResult(1, 1) = -fHeight / 2.0f;
		matResult(3, 1) = fHeight / 2.0f + fTopLeftY;
		matResult(2, 2) = (fMaxDepth - fMinDepth);
		matResult(3, 2) = fMinDepth;

		return matResult;
	}
}