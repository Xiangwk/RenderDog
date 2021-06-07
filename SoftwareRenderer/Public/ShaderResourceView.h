///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: ShaderResourceView.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include <cstdint>

namespace RenderDog
{
	class Vector4;

	class ShaderResourceView
	{
	public:
		ShaderResourceView();
		~ShaderResourceView();

		bool LoadFromFile(const char* strFileName);

		void Release();

		Vector4* GetView() { return m_pView; }
		const Vector4* GetView() const { return m_pView; }

		uint32_t GetWidth() const { return m_nWidth; }
		uint32_t GetHeight() const { return m_nHeight; }

	private:
		Vector4* m_pView;

		uint32_t m_nWidth;
		uint32_t m_nHeight;
	};
}