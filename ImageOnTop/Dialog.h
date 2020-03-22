#pragma once
#include "Window.h"
#include <memory>
#include <string>

namespace Swingl {

class ImageManager;
class ImageDescriptor;

class Dialog : public Window {
public:
	virtual ~Dialog();

	virtual void show(bool value = true) override;
	std::shared_ptr<ImageDescriptor> getSelectedEntry();
	void updateSelectedItemOrigin(int x, int y);

	void setItemText(int itemId, const std::wstring &text);
	std::wstring getItemText(int itemId);

	static std::shared_ptr<Dialog> instance(const std::shared_ptr<ImageManager>& wndMgr);
	static std::shared_ptr<Dialog> instance();

protected:
	Dialog(const std::shared_ptr<ImageManager> &wndMgr);
	
	static INT_PTR CALLBACK dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static std::shared_ptr<Dialog> _instance; // only one instance of the dialog exist
	static int lastSelectedEntry;

	virtual int wndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

	void updateItemsState();
	void updateImageList();
	void enableItems(bool enable, const int ids[]);
	void addEntry(const std::wstring &fileName);
	
	std::shared_ptr<ImageManager> _wndMgr;
};

}