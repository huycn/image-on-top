#pragma once
#include "Window.h"
#include <memory>
#include <string>

namespace Swingl {

class WndManager;
class ImageDescriptor;

class Dialog : public Window {
public:
	virtual ~Dialog();
	void show(bool value = true);
	static std::shared_ptr<Dialog> instance(const std::shared_ptr<WndManager> &wndMgr);
	static std::shared_ptr<Dialog> instance();
	std::shared_ptr<ImageDescriptor> getSelectedEntry();
	void updateWndPos(int x, int y);

	void setItemText(int itemId, const std::wstring &text);
	std::wstring getItemText(int itemId);

protected:
	Dialog(const std::shared_ptr<WndManager> &wndMgr);
	static INT_PTR CALLBACK dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static std::shared_ptr<Dialog> _instance; // only one instance of the dialog exist
	static int lastSelectedEntry;

	int wndProc(const WndMessage &) {return 0;}
	void updateItemsState();
	void updateImageList();
	void enableItems(bool enable, const int ids[]);
	void addEntry(const std::wstring &fileName);
	std::shared_ptr<WndManager> _wndMgr;
};

}