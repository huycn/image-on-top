#pragma once
#include "Menu.h"

namespace Swingl {

class TrayMenu : public Menu
{
public:
	TrayMenu(HINSTANCE hInstance);

	virtual void checkItem(int idCommand, bool checked) override;
	virtual bool isChecked(int idCommand) const override;
	virtual void track(HWND msgRecv, const POINT &pos, int flags) override;

private:
	HMENU _trayMenu;
	int _itemsState;
};

}