#pragma once
#include <windows.h>

namespace Swingl {

class Window {
public:
	virtual ~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	HWND getHandle() { return _handle; }

	virtual void show(bool value = true);

protected:
	friend class WindowClass;

	Window(HWND handle = NULL);
	virtual int wndProc(UINT msg, WPARAM wParam, LPARAM lParam) = 0;

	HWND _handle;
};

}
