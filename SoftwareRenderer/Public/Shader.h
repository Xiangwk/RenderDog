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

	struct VSOutput
	{
		VSOutput() = default;
		VSOutput(const VSOutput & v) = default;
		VSOutput& operator=(const VSOutput & v) = default;

		Vertex	vert;
		float	fInvZ;
	};

	class VertexShader
	{
	public:
		VertexShader() = default;
		~VertexShader() = default;

		VSOutput VSMain(const Vertex& inVertex, const Matrix4x4& matWorld, const Matrix4x4& matView, const Matrix4x4& matProj) const;
	};

	class PixelShader
	{
	public:
		PixelShader() = default;
		~PixelShader() = default;

		uint32_t PSMain(const VSOutput& VSOutput, const ShaderResourceView* pSRV) const;

	private:
		uint32_t Sample(const ShaderResourceView* pSRV, const Vector2& vUV) const;
	};
}
