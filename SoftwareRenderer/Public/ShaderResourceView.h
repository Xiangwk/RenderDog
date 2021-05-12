///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: ShaderResourceView.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include <cstdint>

namespace RenderDog
{
	class ShaderResourceView
	{
	public:
		ShaderResourceView();
		~ShaderResourceView();

		bool LoadFromFile(const wchar_t* strFileName);

		void Release();

		uint32_t* GetView() { return m_pView; }
		const uint32_t* GetView() const { return m_pView; }

		uint32_t GetWidth() const { return m_nWidth; }
		uint32_t GetHeight() const { return m_nHeight; }

	private:
		uint32_t* m_pView;

		uint32_t m_nWidth;
		uint32_t m_nHeight;
	};
}