///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Viewport.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

namespace RenderDog
{
	class Matrix4x4;

	struct Viewport
	{
		float fTopLeftX;
		float fTopLeftY;
		float fWidth;
		float fHeight;
		float fMinDepth;
		float fMaxDepth;

		Matrix4x4 GetViewportMatrix() const;
	};
}
