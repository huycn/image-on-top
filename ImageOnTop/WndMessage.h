#pragma once
#include <windows.h>

namespace Swingl {
	
class WndMessage {
public:
	WndMessage() : msg(0), wParam(NULL), lParam(NULL) {;}
	WndMessage(UINT m, WPARAM w, LPARAM l) : msg(m), wParam(w), lParam(l) {;}
	
	UINT msg;
	WPARAM wParam;
	LPARAM lParam;
};

}