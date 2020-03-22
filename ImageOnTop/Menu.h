#pragma once
#include <windows.h>
#include "resource.h"

namespace Swingl {

class Menu {
public:
	explicit Menu(HMENU handle);
	virtual ~Menu();

	Menu(const Menu&) = delete;
	Menu& operator=(const Menu&) = delete;

	virtual void checkItem(int idCommand, bool checked);
	virtual bool isChecked(int idCommand) const;
	virtual void track(HWND msgRecv, const POINT& pos, int flags);

protected:
	HMENU _handle;
};

}