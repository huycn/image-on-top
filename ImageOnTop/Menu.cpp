#include "Menu.h"

namespace Swingl {

Menu::Menu(HMENU handle) :
    _handle(handle) {
}

Menu::~Menu() {
    if (_handle != NULL) {
        DestroyMenu(_handle);
    }
}

void
Menu::checkItem(int idCommand, bool checked) {
}

bool
Menu::isChecked(int idCommand) const {
    return false;
}

void
Menu::track(HWND msgRecv, const POINT& pos, int flags) {
}

}