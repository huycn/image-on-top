#pragma once
#include "Window.h"
#include <memory>
#include <string>

namespace Swingl {

class ImageManager;
class ImageDescriptor;

class Dialog : public Window {
public:
	Dialog(const std::shared_ptr<ImageManager>& wndMgr);
	virtual ~Dialog();

	virtual void show(bool value = true) override;
	std::shared_ptr<ImageDescriptor> getSelectedEntry();
	void updateSelectedItemOrigin(int x, int y);

	void setItemText(int itemId, const std::wstring &text);
	std::wstring getItemText(int itemId);

private:
	static INT_PTR CALLBACK dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static int lastSelectedEntry;

	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

	void updateItemsState();
	void updateImageList();
	void addEntry(const std::wstring &fileName);
	
	std::weak_ptr<ImageManager> _wndMgr;
};

}