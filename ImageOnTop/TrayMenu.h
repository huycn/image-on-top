#pragma once
#include "Menu.h"

namespace Swingl {

class TrayMenu : public Menu
{
public:
	TrayMenu(HINSTANCE hInstance);
	void checkItem(int idCommand, bool checked);
	bool isChecked(int idCommand) const;
	void track(HWND msgRecv, const POINT &pos, int flags);

private:
	HMENU _trayMenu;
	int _itemsState;
};

}