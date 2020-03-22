#pragma once
#include <windows.h>
#include <string>
#include <memory>

namespace Swingl {

class Window;
class ImageManager;

class WindowClass {
public:
	WindowClass(HINSTANCE hInstance);
	~WindowClass();

	WindowClass(const WindowClass&) = delete;
	WindowClass& operator=(const WindowClass&) = delete;

	const wchar_t* name() const;
	const wchar_t* appName() const;
	HICON getIcon() const;
	int run();
	void quit();

	static HINSTANCE hInstance();
	static LRESULT CALLBACK sWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	void init();

	static HINSTANCE _hInstance;
	std::wstring _name;
	std::wstring _appName;
	bool _hasInit;
	std::unique_ptr<ImageManager> _wndManager;
};

}