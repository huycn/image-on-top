#include "WndClass.h"
#include "WndManager.h"
#include "WndMessage.h"
#include "Window.h"
#include "TemplateUtilities.h"
#include "Exception.h"

namespace Swingl {

HINSTANCE WndClass::_hInstance = NULL;

WndClass::WndClass(HINSTANCE hInstance)
{
	wchar_t buffer[50];
	LoadString(hInstance, IDS_APP_CLASS, buffer, 20);
	_name = buffer;
	LoadString(hInstance, IDS_APP_NAME, buffer, 20);
	_appName = buffer;
	_hInstance = hInstance;
	_hasInit = false;
}

WndClass::~WndClass()
{
}

void
WndClass::init() {
	if (!_hasInit) {
		WNDCLASSEX wc;
		memset(&wc, 0, sizeof(WNDCLASSEX));
		wc.cbSize        = sizeof(WNDCLASSEX);
		wc.lpfnWndProc   = &WndClass::sWndProc;
		wc.hInstance     = _hInstance;
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszClassName = name();
		wc.hIcon		 = (HICON)LoadImage(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32, LR_SHARED);
		wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
		if(!RegisterClassEx(&wc)) {
			throw RuntimeError(TEXT("Window Registration Failed"));
		}

		try {
			std::shared_ptr<WndClass> myPtr = std::shared_ptr<WndClass>(this, DummyDeleter<WndClass>());
			_wndManager.reset(new WndManager(myPtr));
		}
		catch (...) {
			throw;
		}

		_hasInit = true;
	}
}

int
WndClass::run()
{
	init();

	NOTIFYICONDATA niData;
	memset(&niData, 0, sizeof(NOTIFYICONDATA));
	niData.cbSize = sizeof(NOTIFYICONDATA);
	niData.hWnd = _wndManager->getHandle();
	niData.uID = 1000;
	niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	niData.hIcon = (HICON)LoadImage(hInstance(), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, LR_SHARED);
	niData.uCallbackMessage = WndManager::kIconSysTrayCommand;
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
WndClass::quit()
{
	_wndManager->savePrefToRegistry();
	_wndManager.reset();
}

HICON
WndClass::getIcon() const
{
	return static_cast<HICON>(LoadImage(hInstance(), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, LR_SHARED));
}

LRESULT CALLBACK
WndClass::sWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR winPtr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (winPtr != 0) {
		Window *win = reinterpret_cast<Window *>(winPtr);
		WndMessage message(msg, wParam, lParam);
		return win->wndProc(message);
	}
	else if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

}