#include "Dialog.h"
#include "WindowClass.h"
#include "ImageManager.h"
#include "ImageWidget.h"
#include "Exception.h"

#include <iomanip>
#include <sstream>

#ifndef TBM_SETRANGE
#define TBM_SETRANGE (WM_USER + 6)
#endif

#ifndef TBM_SETPOS
#define TBM_SETPOS (WM_USER + 5)
#endif

namespace {
	int transGroupIDs[] = {	IDC_GROUP_TRANSITION,
							IDC_STATIC_EFFECT,
							IDC_COMBO_EFFECT,
							IDC_STATIC_DELAY,
							IDC_EDIT_DELAY,
							IDC_STATIC_DURATION,
							IDC_EDIT_DURATION,
							0
							};
	int imgPropGrpIDs[] = {
							IDC_GROUP_IMGPROPER,
							IDC_CHECK_ALPHA,
							IDC_SLIDER_ALPHA,
							IDC_CHECK_CLKTHROUGH,
							IDC_STATIC_POSITION,
							IDC_STATIC_POSLEFT,
							IDC_EDIT_POSLEFT,
							IDC_STATIC_POSTOP,
							IDC_EDIT_POSTOP,
							0
							};
}

namespace Swingl {

int Dialog::lastSelectedEntry = -1;

Dialog::Dialog(const std::shared_ptr<ImageManager> &wndMgr) : _wndMgr(wndMgr) {
	auto wndClass = wndMgr->getWndClass();
	if (wndClass == nullptr) {
		throw RuntimeError(TEXT("Can't create dialog"));
	}
	_handle = CreateDialogParam(wndClass->hInstance(), MAKEINTRESOURCE(IDD_DIALOG1), wndMgr->getHandle(), &Dialog::dialogProc, NULL);
	if (_handle == NULL) {
		throw RuntimeError(TEXT("Can't create dialog"));
	}
	SetWindowLongPtr(_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	SendMessage(_handle, WM_SETICON, ICON_SMALL, (LPARAM)wndClass->getIcon());
	SendDlgItemMessage(_handle, IDC_SLIDER_ALPHA, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 255));
	//RECT rect;
	//GetWindowRect(_handle, &rect);
	//int dlgWidth = rect.right - rect.left;
	//int dlgHeight = rect.bottom - rect.top;
	//GetClientRect(_handle, &rect);
	//int clientHeight = rect.bottom - rect.top;
	//SetWindowPos(_handle, NULL, 0, 0, dlgWidth , 160 + dlgHeight - clientHeight, SWP_NOREPOSITION|SWP_NOACTIVATE|SWP_NOMOVE);
	//SendDlgItemMessage(_handle, IDC_BUTTON_EXPAND, WM_SETTEXT, NULL, (LPARAM)(_T("Expand\x891")));
	
	bool transEnabled = wndMgr->transMode() != TransMode::NoTrans;
	SendMessage(GetDlgItem(_handle, IDC_CHECK_TRANSITION), BM_SETCHECK, transEnabled ? BST_CHECKED : BST_UNCHECKED, 0);
	enableItems(transEnabled, transGroupIDs);

	HWND cbTransEffect = GetDlgItem(_handle, IDC_COMBO_EFFECT);
	SendMessage(cbTransEffect, CB_ADDSTRING, 0, (LPARAM)TEXT("No effect"));
	SendMessage(cbTransEffect, CB_ADDSTRING, 0, (LPARAM)TEXT("Alpha blending"));
	SendMessage(cbTransEffect, CB_SETCURSEL, max(0, int(wndMgr->transMode()) - 1), (LPARAM)TEXT("Alpha blending"));
 
	std::wostringstream buffer;
	buffer << wndMgr->transDelay();
	setItemText(IDC_EDIT_DELAY, buffer.str());

	buffer.str(TEXT(""));
	buffer << wndMgr->transDuration();
	setItemText(IDC_EDIT_DURATION, buffer.str());

	updateImageList();
}

Dialog::~Dialog() {
	EndDialog(_handle, 0);
	SetWindowLongPtr(_handle, GWLP_USERDATA, 0);
}

LRESULT
Dialog::wndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_CLOSE:
		if (auto wndMgr = _wndMgr.lock()) {
			for (const auto& img : wndMgr->getEntriesList()) {
				img->enableClickThrough(true);
			}
			wndMgr->setDialog(nullptr);
		}
		return TRUE;
	case WM_DROPFILES: {
		HDROP hDropFiles = (HDROP)wParam;
		unsigned nFiles = DragQueryFile(hDropFiles, 0xFFFFFFFF, NULL, 0);
		for (unsigned i = 0; i < nFiles; ++i) {
			unsigned bufferSize = 1 + DragQueryFile(hDropFiles, i, NULL, 0);
			std::wstring buffer(bufferSize, std::wstring::value_type());
			DragQueryFile(hDropFiles, i, &buffer[0], bufferSize);
			addEntry(buffer);
		}
		return TRUE;
	}
	case WM_CHAR:
		if (wParam == VK_RETURN) {
			SetFocus(_handle);
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_ADD:
			if (HIWORD(wParam) == BN_CLICKED) {
				const int maxFileNameLength = 512;
				static TCHAR fileName[512];
				fileName[0] = (TCHAR)0;
				OPENFILENAME openFileName;
				memset(&openFileName, 0, sizeof(OPENFILENAME));
				openFileName.lStructSize = sizeof(OPENFILENAME);
				openFileName.hwndOwner = _handle;
				openFileName.lpstrFile = fileName;
				openFileName.nMaxFile = maxFileNameLength;
				openFileName.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

				if (GetOpenFileName(&openFileName)) {
					addEntry(fileName);
				}
			}
			break;
		case IDC_BUTTON_REMOVE:
			if (HIWORD(wParam) == BN_CLICKED) {
				HWND hItem = GetDlgItem(_handle, IDC_LIST_IMAGES);
				LRESULT lResult = SendMessage(hItem, LB_GETCURSEL, 0, 0);
				if (lResult != LB_ERR) {
					if (auto wndMgr = _wndMgr.lock()) {
						int index = (int)lResult;
						wndMgr->popEntry(index);
						SendMessage(hItem, LB_DELETESTRING, (WPARAM)index, 0);
						std::shared_ptr<ImageDescriptor> entry = wndMgr->getEntry(index);
						if (entry == NULL && lResult > 0) {
							entry = wndMgr->getEntry(--index);
						}
						if (entry != NULL) {
							SendMessage(hItem, LB_SETCURSEL, (WPARAM)index, 0);
						}
						updateItemsState();
					}
				}
			}
			break;
		case IDC_LIST_IMAGES:
			// It's our listbox, check the notification code
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:
				updateItemsState();
				break;
			default:
				break;
			}
			break;
		case IDC_CHECK_ALPHA:
			if (HIWORD(wParam) == BN_CLICKED && lParam != NULL) {
				std::shared_ptr<ImageDescriptor> entry = getSelectedEntry();
				if (entry != NULL) {
					bool enabled = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
					entry->enableTransparency(enabled);
					HWND hSlider = GetDlgItem(_handle, IDC_SLIDER_ALPHA);
					EnableWindow(hSlider, enabled ? TRUE : FALSE);
				}
			}
			break;
		case IDC_CHECK_CLKTHROUGH:
			if (HIWORD(wParam) == BN_CLICKED && lParam != NULL) {
				std::shared_ptr<ImageDescriptor> entry = getSelectedEntry();
				if (entry != NULL) {
					bool enabled = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
					entry->enableClickThrough(enabled);
				}
			}
			break;
		case IDC_EDIT_POSLEFT:
			if (HIWORD(wParam) == EN_CHANGE && lParam != NULL) {
				std::shared_ptr<ImageDescriptor> entry = getSelectedEntry();
				if (entry != NULL) {
					wchar_t buffer[51];
					GetWindowText((HWND)lParam, buffer, 50);
					int left = _wtoi(buffer);
					entry->setPosition(left, entry->top());
				}
			}
			break;
		case IDC_EDIT_POSTOP:
			if (HIWORD(wParam) == EN_CHANGE && lParam != NULL) {
				std::shared_ptr<ImageDescriptor> entry = getSelectedEntry();
				if (entry != NULL) {
					wchar_t buffer[51];
					GetWindowText((HWND)lParam, buffer, 50);
					int top = _wtoi(buffer);
					entry->setPosition(entry->left(), top);
				}
			}
			break;
		case IDC_CHECK_TRANSITION:
			if (HIWORD(wParam) == BN_CLICKED && lParam != NULL) {
				bool enable = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
				enableItems(enable, transGroupIDs);
				//_instance->_wndMgr->setTransMode(enable?eTransDirect: eNoTrans, 5.0, 0.0);
			}
			break;
		case IDC_BUTTON_APPLY:
			if (HIWORD(wParam) == BN_CLICKED && lParam != NULL) {
				bool transEnabled = SendMessage(GetDlgItem(_handle, IDC_CHECK_TRANSITION), BM_GETCHECK, 0, 0) == BST_CHECKED;
				TransMode transMode = TransMode::NoTrans;
				if (transEnabled) {
					LRESULT curSel = SendMessage(GetDlgItem(_handle, IDC_COMBO_EFFECT), CB_GETCURSEL, 0, 0);
					transMode = static_cast<TransMode>((int)curSel + 1);
				}
				std::wistringstream strbuf(getItemText(IDC_EDIT_DELAY));
				double delay;
				strbuf >> delay;
				strbuf.str(getItemText(IDC_EDIT_DURATION));
				double duration;
				strbuf >> duration;

				if (auto wndMgr = _wndMgr.lock()) {
					wndMgr->setTransMode(transMode, delay, duration);
				}
			}
			break;
		default:
			break;
		}
		return TRUE;
	case WM_HSCROLL:
		if (LOWORD(wParam) == SB_THUMBTRACK && lParam != NULL) {
			if (auto wndMgr = _wndMgr.lock()) {
				int pos = HIWORD(wParam);
				LRESULT index = SendDlgItemMessage(_handle, IDC_LIST_IMAGES, LB_GETCURSEL, 0, 0);
				std::shared_ptr<ImageDescriptor> entry = wndMgr->getEntry((int)index);
				if (entry != NULL) {
					entry->setTransparency(true, pos);
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}

void
Dialog::setItemText(int itemId, const std::wstring &text) {
	HWND edit = GetDlgItem(_handle, itemId);
	SetWindowText(edit, text.c_str());
}

std::wstring
Dialog::getItemText(int itemId) {
	HWND edit = GetDlgItem(_handle, itemId);
	TCHAR buffer[256];
	GetWindowText(edit, buffer, 256);
	return buffer;
}

void
Dialog::enableItems(bool enable, const int ids[]) {
	for (int i=0; ids[i] > 0; ++i) {
		HWND hItem = GetDlgItem(_handle, ids[i]);
		EnableWindow(hItem, enable);
	}
}

void
Dialog::updateImageList() {
	if (auto wndMgr = _wndMgr.lock()) {
		HWND hListItem = GetDlgItem(_handle, IDC_LIST_IMAGES);
		auto imgList = wndMgr->getEntriesList();
		for (const auto& img : imgList) {
			SendMessage(hListItem, LB_ADDSTRING, 0, (LPARAM)img->name().c_str());
		}
		if (lastSelectedEntry >= 0 && lastSelectedEntry < static_cast<int>(imgList.size())) {
			SendMessage(hListItem, LB_SETCURSEL, (WPARAM)lastSelectedEntry, 0);
		}
		updateItemsState();
	}
}

std::shared_ptr<ImageDescriptor>
Dialog::getSelectedEntry() {
	std::shared_ptr<ImageDescriptor> result;
	if (auto wndMgr = _wndMgr.lock()) {
		LRESULT index = SendDlgItemMessage(_handle, IDC_LIST_IMAGES, LB_GETCURSEL, 0, 0);
		if (index != LB_ERR) {
			result = wndMgr->getEntry(lastSelectedEntry = (int)index);
		}
	}
	return result;
}

void
Dialog::updateItemsState() {
	std::shared_ptr<ImageDescriptor> entry = getSelectedEntry();
	HWND btnRemove = GetDlgItem(_handle, IDC_BUTTON_REMOVE);
	if (entry != NULL) {
		HWND hSlider = GetDlgItem(_handle, IDC_SLIDER_ALPHA);
		bool hasAlpha = entry->transparencyEnabled();
		SendDlgItemMessage(_handle, IDC_CHECK_ALPHA, BM_SETCHECK, (WPARAM)(hasAlpha ? BST_CHECKED : BST_UNCHECKED), 0);
		EnableWindow(hSlider, hasAlpha ? TRUE : FALSE);
		SendMessage(hSlider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)entry->transparencyValue());
		SendDlgItemMessage(_handle, IDC_CHECK_CLKTHROUGH, BM_SETCHECK, (WPARAM)(entry->isClickThrough() ? BST_CHECKED : BST_UNCHECKED), 0);
		updateSelectedItemOrigin(entry->left(), entry->top());
		EnableWindow(btnRemove, TRUE);
		enableItems(true, imgPropGrpIDs);
	}
	else {
		EnableWindow(btnRemove, FALSE);
		enableItems(false, imgPropGrpIDs);
	}
}

void
Dialog::updateSelectedItemOrigin(int x, int y) {
	wchar_t buffer[50];
	wsprintf(buffer, TEXT("%d"), x);
	setItemText(IDC_EDIT_POSLEFT, buffer);
	wsprintf(buffer, TEXT("%d"), y);
	setItemText(IDC_EDIT_POSTOP, buffer);
}

void
Dialog::show(bool) {
	ShowWindow(_handle, SW_SHOW);
	SetForegroundWindow(_handle);

	if (auto wndMgr = _wndMgr.lock()) {
		for (const auto& img : wndMgr->getEntriesList()) {
			img->enableClickThrough(false);
		}
	}

	updateItemsState();
}

void
Dialog::addEntry(const std::wstring &fileName) {
	if (auto wndMgr = _wndMgr.lock()) {
		ImageDescriptor entry;
		auto ind = fileName.find_last_of('\\');
		std::wstring entryName;
		if (ind != std::wstring::npos) {
			entryName = fileName.substr(ind + 1);
		}
		else {
			entryName = fileName;
		}
		entry.setName(entryName);
		entry.setFileName(fileName);
		entry.setTransparency(true, 128);
		//entry.enableClickThrough(true);
		HWND hListItem = GetDlgItem(_handle, IDC_LIST_IMAGES);
		LRESULT index = SendMessage(hListItem, LB_ADDSTRING, 0, (LPARAM)entry.name().c_str());
		if (index != LB_ERR) {
			if (wndMgr->insertEntry(entry, (int)index) >= 0) {
				SendMessage(hListItem, LB_SETCURSEL, (WPARAM)index, 0);
				updateItemsState();
			}
			else {
				SendMessage(hListItem, LB_DELETESTRING, (WPARAM)index, 0);
				MessageBox(_handle, TEXT("The image format is probably unknown"), TEXT("Can't load image"), MB_ICONEXCLAMATION | MB_OK);
			}
		}
	}
}

INT_PTR CALLBACK
Dialog::dialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return (INT_PTR)WindowClass::sWndProc(hwndDlg, uMsg, wParam, lParam);
}

}