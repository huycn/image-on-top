#include "WindowClass.h"
#include "ImageManager.h"
#include "Window.h"
#include "TemplateUtilities.h"
#include "Exception.h"

namespace Swingl {

HINSTANCE WindowClass::_hInstance = NULL;

WindowClass::WindowClass(HINSTANCE hInstance)
{
	wchar_t buffer[50];
	LoadString(hInstance, IDS_APP_CLASS, buffer, 20);
	_name = buffer;
	LoadString(hInstance, IDS_APP_NAME, buffer, 20);
	_appName = buffer;
	_hInstance = hInstance;
	_hasInit = false;
}

WindowClass::~WindowClass()
{
}

const wchar_t*
WindowClass::name() const {
	return _name.c_str();
}

const wchar_t*
WindowClass::appName() const {
	return _appName.c_str();
}

void
WindowClass::init() {
	if (!_hasInit) {
		WNDCLASSEX wc;
		memset(&wc, 0, sizeof(WNDCLASSEX));
		wc.cbSize        = sizeof(WNDCLASSEX);
		wc.lpfnWndProc   = &WindowClass::sWndProc;
		wc.hInstance     = _hInstance;
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszClassName = name();
		wc.hIcon		 = (HICON)LoadImage(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32, LR_SHARED);
		wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
		if(!RegisterClassEx(&wc)) {
			throw RuntimeError(TEXT("Window Registration Failed"));
		}

		try {
			std::shared_ptr<WindowClass> myPtr = std::shared_ptr<WindowClass>(this, DummyDeleter<WindowClass>());
			_wndManager.reset(new ImageManager(myPtr));
		}
		catch (...) {
			throw;
		}

		_hasInit = true;
	}
}

int
WindowClass::run()
{
	init();

	NOTIFYICONDATA niData;
	memset(&niData, 0, sizeof(NOTIFYICONDATA));
	niData.cbSize = sizeof(NOTIFYICONDATA);
	niData.hWnd = _wndManager->getHandle();
	niData.uID = 1000;
	niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	niData.hIcon = (HICON)LoadImage(hInstance(), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, LR_SHARED);
	niData.uCallbackMessage = ImageManager::kIconSysTrayCommand;
	wcscpy_s(niData.szTip, appName());
	Shell_NotifyIcon(NIM_ADD, &niData);

	_wndManager->loadPrefFromRegistry();
    MSG Msg;
    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
	Shell_NotifyIcon(NIM_DELETE, &niData);
	return (int)Msg.wParam;
}

void
WindowClass::quit()
{
	_wndManager->savePrefToRegistry();
	_wndManager.reset();
}

HICON
WindowClass::getIcon() const
{
	return static_cast<HICON>(LoadImage(hInstance(), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, LR_SHARED));
}

LRESULT CALLBACK
WindowClass::sWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR winPtr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (winPtr != 0) {
		Window *win = reinterpret_cast<Window *>(winPtr);
		return win->wndProc(msg, wParam, lParam);
	}
	else if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HINSTANCE
WindowClass::hInstance() {
	return _hInstance;
}

}