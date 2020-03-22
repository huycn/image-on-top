#pragma once
#include <windows.h>
#include <string>
#include <memory>

namespace Swingl {

class Window;
class WndManager;

class WndClass {
public:
	WndClass(HINSTANCE hInstance);
	~WndClass();

	static HINSTANCE hInstance() {return _hInstance;}
	const wchar_t * name() const {return _name.c_str();}
	const wchar_t * appName() const {return _appName.c_str();}
	HICON getIcon() const;
	int run();
	void quit();

	static LRESULT CALLBACK sWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	void init();

	static HINSTANCE _hInstance;
	std::wstring _name;
	std::wstring _appName;
	bool _hasInit;
	std::shared_ptr<WndManager> _wndManager;
};

}