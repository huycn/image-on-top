#pragma once
#include <windows.h>

namespace Swingl {

class WndMessage;

class Window {
public:
	virtual ~Window();
	HWND getHandle() {return _handle;}

	virtual void show(bool value = true) {
		ShowWindow(_handle, value ? SW_SHOW : SW_HIDE);
	}

protected:
	friend class WndClass;

	Window();
	virtual int wndProc(const WndMessage &) = 0;

	HWND _handle;
};

}
