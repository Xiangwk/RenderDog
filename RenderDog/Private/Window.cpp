///////////////////////////////////////////
//RenderDog <¡¤,¡¤>
//FileName: Window.cpp
//Written by Xiang Weikang
//Desc: Application Window Implementation
///////////////////////////////////////////

#include "Window.h"

namespace RenderDog
{
	class AppWindow : public IWindow
	{
	public:
		AppWindow();
		virtual ~AppWindow();

		virtual bool Init(const WindowDesc& desc) override;
		virtual void Release() override;

		virtual HWND GetHandle() const override { return m_hWnd; }

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }

		virtual void SetWidth(uint32_t width) override { m_Width = width; }
		virtual void SetHeight(uint32_t height) override { m_Height = height; }

	private:
		bool RegisterWindowClass(const WindowDesc& desc);
		bool CreateAppWindow(const WindowDesc& desc);

	private:
		HWND			m_hWnd;
		uint32_t		m_Width;
		uint32_t		m_Height;
		std::wstring	m_Caption;

	};

	AppWindow g_AppWindow;
	IWindow* g_pIWindow = &g_AppWindow;

	//---------------------------------------------------------------------------
	//   Public Function Definition
	//---------------------------------------------------------------------------
	AppWindow::AppWindow() :
		m_hWnd(0),
		m_Width(0),
		m_Height(0),
		m_Caption(L"")
	{}

	AppWindow::~AppWindow()
	{}

	bool AppWindow::Init(const WindowDesc& desc)
	{
		if (!RegisterWindowClass(desc))
		{
			return false;
		}

		if (!CreateAppWindow(desc))
		{
			return false;
		}

		MoveWindow(m_hWnd, 0, 0, m_Width, m_Height, true);

		ShowWindow(m_hWnd, SW_SHOW);

		UpdateWindow(m_hWnd);

		return true;
	}

	void AppWindow::Release()
	{
		DestroyWindow(m_hWnd);
	}

	//---------------------------------------------------------------------------
	//   Private Function Definition
	//---------------------------------------------------------------------------
	bool AppWindow::RegisterWindowClass(const WindowDesc& desc)
	{
		WNDCLASSEX wndClass = { 0 };
		wndClass.cbSize = sizeof(WNDCLASSEX);
		wndClass.style = CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc = desc.wndProc;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = desc.hAppInstance;
		wndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
		wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wndClass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
		wndClass.lpszMenuName = nullptr;
		wndClass.lpszClassName = desc.className.c_str();
		wndClass.hIconSm = LoadIcon(0, IDI_APPLICATION);
		if (!RegisterClassEx(&wndClass))
		{
			MessageBox(nullptr, L"Failed to register WindowClass!", L"ERROR", MB_OK);
			return false;
		}
		else
		{
			return true;
		}
	}

	bool AppWindow::CreateAppWindow(const WindowDesc& desc)
	{
		RECT WndRect = { 0 };
		WndRect.top = 0;
		WndRect.left = 0;
		WndRect.right = desc.width - 1;
		WndRect.bottom = desc.height - 1;
		if (!AdjustWindowRect(&WndRect, WS_OVERLAPPEDWINDOW, false))
		{
			MessageBox(nullptr, L"Failed to adjust window rectangle!", L"ERROR", MB_OK);
			return false;
		}
		int width = WndRect.right - WndRect.left;
		int height = WndRect.bottom - WndRect.top;

		m_hWnd = CreateWindow(desc.className.c_str(),
			desc.caption.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			width, height,
			nullptr, nullptr,
			desc.hAppInstance,
			nullptr);

		if (!m_hWnd)
		{
			MessageBox(nullptr, L"CreateWindow Failed.", L"ERROR", MB_OK);
			return false;
		}

		m_Width = desc.width;
		m_Height = desc.height;
		m_Caption = desc.caption;

		return true;
	}

}// namespace RenderDog