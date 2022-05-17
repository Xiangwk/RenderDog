////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Window.h
//Written by Xiang Weikang
//Desc: Application Window Interface
////////////////////////////////////////

#pragma once

#include <Windows.h>
#include <string>

namespace RenderDog
{
	struct WindowDesc
	{
		uint32_t			width;
		uint32_t			height;
		std::string			caption;
		std::string			className;

		HINSTANCE			hAppInstance;
		WNDPROC				wndProc;
	};

	class IWindow
	{
	public:
		virtual ~IWindow() = default;

		virtual bool		Init(const WindowDesc& desc) = 0;
		virtual void		Release() = 0;

		virtual HWND		GetHandle() const = 0;

		virtual uint32_t	GetWidth() const = 0;
		virtual uint32_t	GetHeight() const = 0;

		virtual void		SetWidth(uint32_t width) = 0;
		virtual void		SetHeight(uint32_t height) = 0;
	};

	extern IWindow* g_pIWindow;

}// namespace RenderDog
