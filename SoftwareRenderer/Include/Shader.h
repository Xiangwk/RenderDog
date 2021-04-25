///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Shader.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

namespace RenderDog
{
	class VertexBuffer;
	class Matrix4x4;
	class Vector3;
	struct Vertex;

	class VertexShader
	{
	public:
		VertexShader() = default;
		~VertexShader() = default;

		Vertex VSMain(const Vertex& inVertex, const Matrix4x4& matWorld, const Matrix4x4& matView, const Matrix4x4& matProj) const;
	};

	class PixelShader
	{
	public:
		PixelShader() = default;
		~PixelShader() = default;

		Vector3 PSMain(const Vertex& VSOutput) const;
	};
}
