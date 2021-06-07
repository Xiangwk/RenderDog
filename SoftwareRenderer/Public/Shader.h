///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Shader.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include <cstdint>

#include "Vertex.h"

namespace RenderDog
{
	class VertexBuffer;
	class Matrix4x4;
	class Vector2;
	class Vector3;
	class Vector4;
	class ShaderResourceView;

	struct VSOutputVertex
	{
		VSOutputVertex() = default;
		VSOutputVertex(const VSOutputVertex & v) = default;
		VSOutputVertex& operator=(const VSOutputVertex & v) = default;

		Vector4 SVPosition;
		Vector4 Color;
		Vector3 Normal;
		Vector4 Tangent;
		Vector2 Texcoord;
	};

	class VertexShader
	{
	public:
		VertexShader() = default;
		~VertexShader() = default;

		VSOutputVertex VSMain(const Vertex& inVertex, const Matrix4x4& matWorld, const Matrix4x4& matView, const Matrix4x4& matProj) const;
	};

	class PixelShader
	{
	public:
		PixelShader() = default;
		~PixelShader() = default;

		Vector4 PSMain(const VSOutputVertex& VSOutput, const ShaderResourceView* pSRV) const;

	private:
		Vector4 Sample(const ShaderResourceView* pSRV, const Vector2& vUV) const;
	};
}
