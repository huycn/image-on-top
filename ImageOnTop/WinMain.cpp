#include <windows.h>
#include "resource.h"
#include "WindowClass.h"
#include "Exception.h"

#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' "\
  "name='Microsoft.Windows.Common-Controls' "\
  "version='6.0.0.0' "\
  "processorArchitecture='*' "\
  "publicKeyToken='6595b64144ccf1df' "\
  "language='*'\"")

#pragma comment(lib, "ComCtl32.lib")


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	Swingl::WindowClass wndClass(hInstance);
	if (HWND hPrevWnd = FindWindow(wndClass.name(), NULL)) { 
		return 0;
	}

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	int returnCode = 0;
	try {
		returnCode = wndClass.run();
	}
	catch (const Swingl::Exception &e) {
		MessageBox(NULL, e.what(), TEXT("ERROR!"), MB_OK|MB_ICONERROR);
	}
	catch (...) {
	}

	return 0;
}
