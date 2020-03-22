#include "ImageManager.h"
#include "resource.h"
#include "Exception.h"
#include "WindowClass.h"
#include "ImageWidget.h"
#include "Dialog.h"
#include "TemplateUtilities.h"

namespace Swingl {

ImageManager::ImageManager(const std::shared_ptr<WindowClass> &wndClass)
:  _wndClass(wndClass), _trayMenu(), _transMode(TransMode::NoTrans), _transInProgress(false)
{
	_handle = CreateWindowEx(0, _wndClass->name(), TEXT("WndManager"), 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, _wndClass->hInstance(), NULL);
	if (_handle == NULL) {
		throw RuntimeError(TEXT("Can't create window manager"));
	}
	SetWindowLongPtr(_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	_trayMenu = std::make_shared<TrayMenu>(wndClass->hInstance());
	_trayMenu->checkItem(ID_TRAYICONMENU_ALWAYSONTOP, true);

	_transMode = TransMode::NoTrans;
	_transDelay = 300;
	_transDuration = 3;
	_transActualImg = -1;
	_transNextImg = -1;
}

ImageManager::~ImageManager() {
	if (_handle != NULL) {
		DestroyWindow(_handle);
		SetWindowLongPtr(_handle, GWLP_USERDATA, 0);
	}
}

std::shared_ptr<ImageDescriptor>
ImageManager::getEntry(int index) const {
	if (index >= 0 && index < static_cast<int>(_imgList.size())) {
		return _imgList[index];
	}
	return std::shared_ptr<ImageDescriptor>();
}

std::shared_ptr<ImageDescriptor>
ImageManager::popEntry(int index) {
	std::shared_ptr<ImageDescriptor> result = getEntry(index);
	if (result != NULL) {
		_imgList.erase(_imgList.begin() + index);
	}
	return result;
}

void
ImageManager::setTransMode(TransMode mode, double delay, double duration) {
	_transMode = mode;
	_transDelay = delay;
	_transDuration = duration;
	if (mode != TransMode::NoTrans) {
		if (not _imgList.empty()) {
			std::shared_ptr<ImageWidget> img = std::dynamic_pointer_cast<ImageWidget>(_imgList.front());
			if (img == NULL) {
				std::shared_ptr<ImageWidget> imgWnd = std::make_shared<ImageWidget>(*_wndClass);
				if (imgWnd->loadByDescriptor(*_imgList.front())) {
					_imgList.front() = imgWnd;
				}
			}
		}

		if (_imgList.size() > 1) {
			for (ImageList::iterator it = _imgList.begin() + 1; it != _imgList.end(); ++it) {
				*it = std::make_shared<ImageDescriptor>(**it);
			}
			SetTimer(_handle, 2, static_cast<unsigned>(1000*_transDelay), NULL);
		}
	}
	else {
		ImageList imgListCopy = _imgList;
		_imgList.clear();
		for (ImageList::iterator it = imgListCopy.begin(); it != imgListCopy.end(); ++it) {
			insertEntry(**it);
		}
		KillTimer(_handle, 2);
	}
}


void
ImageManager::getEntriesList(ImageList &list) const {
	list.assign(_imgList.begin(), _imgList.end());
}

void
ImageManager::transStart() {
	//if (!_transInProgress && _imgList.size() > 1) {
	//	if (_transActualImg < 0 || _transActualImg >= static_cast<int>(_imgList.size())) {
	//		_transActualImg = 0;
	//		_transNextImg = 1;
	//	}
	//	else if (_transNextImg < 0 || _transNextImg >= static_cast<int>(_imgList.size())) {
	//		_transNextImg = (_transActualImg + 1) % _imgList.size();
	//	}
	//	_transFrame = 0;
	//	SetTimer(_handle, 2, static_cast<unsigned>(1.0/framesPerSecond), NULL);
	//}
}

void
ImageManager::transNextFrame() {
	if (_imgList.size() > 1) {
		for (ImageList::iterator it = _imgList.begin(); it != _imgList.end(); ++it) {
			std::shared_ptr<ImageWidget> img = std::dynamic_pointer_cast<ImageWidget>(*it);
			if (img != NULL) {
				*it = std::shared_ptr<ImageDescriptor>(new ImageDescriptor(*img));
				++it;
				if (it == _imgList.end()) it = _imgList.begin();
				std::shared_ptr<ImageWidget> imgWnd(new ImageWidget(*_wndClass));
				if (imgWnd->loadByDescriptor(**it)) {
					*it = imgWnd;
				}
				break;
			}
		}
	}
}

int
ImageManager::insertEntry(const ImageDescriptor &entry, int index) {
	std::shared_ptr<ImageDescriptor> img;
	if (_transMode == TransMode::NoTrans) {
		std::shared_ptr<ImageWidget> imgWnd = std::make_shared<ImageWidget>(*_wndClass);
		if (imgWnd->loadByDescriptor(entry)) {
			img = imgWnd;
		}
		else {
			return -1;
		}
	}
	else {
		img = std::make_shared<ImageDescriptor>(entry);
	}
	if (index < 0 || index >= static_cast<int>(_imgList.size())) {
		_imgList.push_back(img);
		return (int)_imgList.size() - 1;
	}
	else {
		_imgList.insert(_imgList.begin() + index, img);
		return index;
	}
}

void
ImageManager::loadPrefFromRegistry() {
	HKEY keySoftware;
	LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software"), 0, KEY_QUERY_VALUE, &keySoftware);
	if (result != ERROR_SUCCESS) return;
	HKEY myKey;
	result = RegOpenKeyEx(keySoftware, _wndClass->appName(), 0, KEY_QUERY_VALUE, &myKey);
	if (result == ERROR_SUCCESS) {
		DWORD size = sizeof(int);
		unsigned transMode = 0;
		RegQueryValueEx(myKey, TEXT("TransMode"), 0, NULL, (LPBYTE)&transMode, &size);
		unsigned transDelay = 0;
		RegQueryValueEx(myKey, TEXT("TransDelay"), 0, NULL, (LPBYTE)&transDelay, &size);
		unsigned transDuration = 0;
		RegQueryValueEx(myKey, TEXT("TransDuration"), 0, NULL, (LPBYTE)&transDuration, &size);
		int nbEntries = 0;
		result = RegQueryValueEx(myKey, TEXT("NbEntries"), 0, NULL, (LPBYTE)&nbEntries, &size);
		if (result == ERROR_SUCCESS) {
			const int bufferSize = MAX_PATH + 100;
			wchar_t buffer[bufferSize];
			wchar_t entryName[10];
			for (int i=0; i<nbEntries; ++i) {
				wsprintf(entryName, TEXT("%d"), i);
				DWORD bytesRead = (bufferSize - 1) * sizeof(wchar_t);
				result = RegQueryValueEx(myKey, entryName, 0, NULL, (LPBYTE)buffer, &bytesRead);
				if (result == ERROR_SUCCESS) {
					buffer[bytesRead] = wchar_t(0);
					_imgList.push_back(ImageList::value_type(new ImageDescriptor(buffer)));
				}
				result = ERROR_SUCCESS;
			}
		}
		setTransMode(static_cast<TransMode>(transMode), double(transDelay)/1000.0, double(transDuration)/1000.0);
		RegCloseKey(myKey);
	}
	RegCloseKey(keySoftware);
}

void
ImageManager::savePrefToRegistry() const {
	HKEY keySoftware;
	LONG result = RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software"), 0, NULL,
						REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL,	&keySoftware, NULL);
	if (result == ERROR_SUCCESS) {
		RegDeleteKey(keySoftware, _wndClass->appName());
		if (_imgList.size() > 0) {
			HKEY myKey;
			result = RegCreateKeyEx(keySoftware, _wndClass->appName(), 0, NULL,
						REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &myKey, NULL);
			if (result == ERROR_SUCCESS) {
				unsigned transValue = static_cast<unsigned>(_transMode);
				RegSetValueEx(myKey, TEXT("TransMode"), 0, REG_DWORD, (BYTE *)&transValue, sizeof(transValue));
				transValue = static_cast<unsigned>(_transDelay * 1000);
				RegSetValueEx(myKey, TEXT("TransDelay"), 0, REG_DWORD, (BYTE *)&transValue, sizeof(transValue));
				transValue = static_cast<unsigned>(_transDuration * 1000);
				RegSetValueEx(myKey, TEXT("TransDuration"), 0, REG_DWORD, (BYTE *)&transValue, sizeof(transValue));
				int nbEntries = (int)_imgList.size();
				wchar_t entryName[10];
				RegSetValueEx(myKey, TEXT("NbEntries"), 0, REG_DWORD, (BYTE *)&nbEntries, sizeof(nbEntries));
				for (int i=0; i<nbEntries; ++i) {
					wsprintf(entryName, TEXT("%d"), i);
					std::wstring value = _imgList[i]->toString();
					result = RegSetValueEx(myKey, entryName, 0, REG_BINARY, (BYTE *)value.c_str(), ((int)value.size() + 1) * sizeof(wchar_t));
				}
				RegCloseKey(myKey);
			}
		}
		RegCloseKey(keySoftware);
	}
}


void
ImageManager::openDialog() {
	std::shared_ptr<Dialog> dial = Dialog::instance(std::shared_ptr<ImageManager>(const_cast<ImageManager*>(this), DummyDeleter<ImageManager>()));
	dial->show();
}

int
ImageManager::wndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		case WM_TIMER:
		{
			if ((unsigned)wParam == 1) {
				transStart();
			}
			else if ((unsigned)wParam == 2) {
				transNextFrame();
			}
			break;
		}
		case kIconSysTrayCommand:
		{
			switch (lParam) {
				case WM_LBUTTONDBLCLK:
					openDialog();
					break;
				case WM_RBUTTONUP:
				{
					POINT point;
					GetCursorPos(&point);
					_trayMenu->track(_handle, point, TPM_RIGHTALIGN | TPM_BOTTOMALIGN);
					break;
				}
				default:
					break;
			}
			break;
		}
		case WM_COMMAND:
		{
			if (HIWORD(wParam) == 0) { // From menu
				switch (LOWORD(wParam)) {
					case ID_TRAYICONMENU_IMAGES:
					{
						openDialog();
						break;
					}
					case ID_TRAYICONMENU_ALWAYSONTOP:
						if (_trayMenu->isChecked(ID_TRAYICONMENU_ALWAYSONTOP)) {
							if (_transMode == TransMode::NoTrans) {
								for (ImageList::iterator it = _imgList.begin(); it != _imgList.end(); ++it) {
									SetWindowPos(dynamic_cast<ImageWidget *>(it->get())->getHandle(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
								}
							}
							_trayMenu->checkItem(ID_TRAYICONMENU_ALWAYSONTOP, false);
						}
						else {
							if (_transMode == TransMode::NoTrans) {
								for (ImageList::iterator it = _imgList.begin(); it != _imgList.end(); ++it) {
									SetWindowPos(dynamic_cast<ImageWidget *>(it->get())->getHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
								}
							}
							_trayMenu->checkItem(ID_TRAYICONMENU_ALWAYSONTOP, true);
						}
						break;
					case ID_TRAYICONMENU_HIDESHOW:
						if (_trayMenu->isChecked(ID_TRAYICONMENU_HIDESHOW)) {
							if (_transMode == TransMode::NoTrans) {
								for (ImageList::iterator it = _imgList.begin(); it != _imgList.end(); ++it) {
									dynamic_cast<ImageWidget *>(it->get())->show(true);
								}
							}
							_trayMenu->checkItem(ID_TRAYICONMENU_HIDESHOW, false);
						}
						else {
							if (_transMode == TransMode::NoTrans) {
								for (ImageList::iterator it = _imgList.begin(); it != _imgList.end(); ++it) {
									dynamic_cast<ImageWidget *>(it->get())->show(false);
								}
							}
							_trayMenu->checkItem(ID_TRAYICONMENU_HIDESHOW, true);
						}
						break;
					case ID_TRAYICONMENU_EXIT:
						_wndClass->quit();
						break;
					default:
						break;
				}
			}
			break;
		}
		default:
			return (int)DefWindowProc(_handle, msg, wParam, lParam);
	}
	return 0;
}


}