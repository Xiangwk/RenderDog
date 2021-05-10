///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Shader.cpp
//Written by Xiang Weikang
///////////////////////////////////

#include "Shader.h"
#include "Vertex.h"
#include "Matrix.h"

namespace RenderDog
{
	Vertex VertexShader::VSMain(const Vertex& inVertex, const Matrix4x4& matWorld, const Matrix4x4& matView, const Matrix4x4& matProj) const
	{
		Vertex VSOutput = {};
		Vector4 vInPos = Vector4(inVertex.vPostion, 1.0f);
		vInPos = vInPos * matWorld;
		vInPos = vInPos * matView;
		vInPos = vInPos * matProj;
		vInPos = vInPos / vInPos.w;

		VSOutput.vPostion = Vector3(vInPos.x, vInPos.y, vInPos.z);
		VSOutput.vColor = inVertex.vColor;

		return VSOutput;
	}

	Vector3 PixelShader::PSMain(const Vertex& VSOutput) const
	{
		return VSOutput.vColor;
	}
}

