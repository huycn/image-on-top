#include "TrayMenu.h"

namespace Swingl {

TrayMenu::TrayMenu(HINSTANCE hInstance)
{
	_handle = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));
	_trayMenu = GetSubMenu(_handle, 0);
	_itemsState = 0;
}

void
TrayMenu::checkItem(int idCommand, bool checked) {
	int offset = idCommand - OFFSET_ID_TRAYICONMENU;
	CheckMenuItem(_trayMenu, offset, MF_BYPOSITION | (checked ? MF_CHECKED : MF_UNCHECKED));
	if (checked) {
		_itemsState |= (1 << offset);
	}
	else {
		_itemsState &= ~(1 << offset);
	}
}

bool
TrayMenu::isChecked(int idCommand) const {
	int offset = idCommand - OFFSET_ID_TRAYICONMENU;
	return (_itemsState & (1 << offset)) != 0;
}

void
TrayMenu::track(HWND msgRecv, const POINT &pos, int flags)
{
	HWND oldFgWnd = GetForegroundWindow();
	SetForegroundWindow(msgRecv);
	TrackPopupMenu(_trayMenu, flags, pos.x, pos.y, 0, msgRecv, NULL);
	PostMessage(msgRecv, WM_NULL, 0, 0);
	SetForegroundWindow(oldFgWnd);
}


}