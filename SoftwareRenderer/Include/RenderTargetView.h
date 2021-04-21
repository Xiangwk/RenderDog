///////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: RenderTargetView.h
//Written by Xiang Weikang
///////////////////////////////////

#pragma once

#include <cstdint>

namespace RenderDog
{
	struct RenderTargetDesc
	{
		uint32_t	width;
		uint32_t	height;
		void* pFrameBuf;
	};

	class RenderTargetView
	{
	public:
		RenderTargetView() :
			m_pView(nullptr),
			m_nWidth(0),
			m_nHeight(0)
		{}

		~RenderTargetView()
		{
			m_pView = nullptr;
		}

		uint32_t*& GetView() { return m_pView; }
		const uint32_t* GetView() const { return m_pView; }

		void SetWidth(uint32_t width) { m_nWidth = width; }
		void SetHeight(uint32_t height) { m_nHeight = height; }

		uint32_t GetWidth() const { return m_nWidth; }
		uint32_t GetHeight() const { return m_nHeight; }

	private:
		uint32_t*	m_pView;
		uint32_t	m_nWidth;
		uint32_t	m_nHeight;
	};
}