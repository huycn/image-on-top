#pragma once
#include <windows.h>
#include <string>
#include <memory>

namespace Swingl {

class Window;
class ImageManager;

class WindowClass : public std::enable_shared_from_this<WindowClass> {
public:
	~WindowClass();

	WindowClass(const WindowClass&) = delete;
	WindowClass& operator=(const WindowClass&) = delete;

	const wchar_t* name() const;
	const wchar_t* appName() const;
	std::shared_ptr<ImageManager> imageManager();
	HICON getIcon() const;
	int run();
	void quit();

	HINSTANCE hInstance() const;

	static std::shared_ptr<WindowClass> newInstance(HINSTANCE hInstance);
	static LRESULT CALLBACK sWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	WindowClass(HINSTANCE hInstance);
	void init();

	HINSTANCE _hInstance;
	std::wstring _name;
	std::wstring _appName;
	bool _hasInit;
	std::shared_ptr<ImageManager> _wndManager;
};

}