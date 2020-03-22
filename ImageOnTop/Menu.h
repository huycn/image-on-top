#pragma once
#include <windows.h>
#include "resource.h"

namespace Swingl {

class Menu {
public:
	virtual ~Menu()	{
		if (_handle != NULL) DestroyMenu(_handle);
	}

	virtual void checkItem(int idCommand, bool checked) {;}
	virtual bool isChecked(int idCommand) const {return false;}
	virtual void track(HWND msgRecv, const POINT &pos, int flags) {;}

protected:
	HMENU _handle;
};

}