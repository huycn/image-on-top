#include "Window.h"

namespace Swingl {

Window::Window(HWND handle) :
    _handle(handle) {
}

Window::~Window() {
}

void
Window::show(bool value) {
	ShowWindow(_handle, value ? SW_SHOW : SW_HIDE);
}

}