///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: ShaderResourceView.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#define USE_FREE_IMAGE 0
#define USE_STB_IMAGE 1

#include <cstdint>

namespace RenderDog
{
	class ShaderResourceView
	{
	public:
		ShaderResourceView();
		~ShaderResourceView();

#if USE_DIRECTX_TEX
		bool LoadFromFile(const wchar_t* strFileName);
#else
		bool LoadFromFile(const char* strFileName);
#endif

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