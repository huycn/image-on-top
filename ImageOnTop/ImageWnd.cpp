#include "ImageWnd.h"
#include "WndMessage.h"
#include "WndClass.h"
#include "Dialog.h"
#include "Bitmap.h"
#include "Exception.h"

#include <winuser.h>

namespace Swingl {

ImageWnd::ObjList ImageWnd::_imgObj;

ImageWnd::ImageWnd(WndClass &wndClass)
{
	_mouseLeftHold = false;
	_mousePos = { 0 };
	_isClickThrough = true;
	_handle = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
							wndClass.name(),
							TEXT("Image"),
							WS_POPUP,
							0, 0, 0, 0,
							NULL, NULL, wndClass.hInstance(), NULL);
	if (_handle == NULL) {
		throw RuntimeError(TEXT("Can't create window image"));
	}

	SetWindowLongPtr(_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	ShowWindow(_handle, SW_SHOWNORMAL);
	_imgObj.insert(this);
}

ImageWnd::~ImageWnd()
{
	if (_handle != NULL) {
		DestroyWindow(_handle);
		SetWindowLong(_handle, GWLP_USERDATA, 0);
	}
	_imgObj.erase(this);
}

bool
ImageWnd::loadImage(const std::wstring &fileName, bool update) {
	for (ObjList::const_iterator it = _imgObj.begin(); it != _imgObj.end(); ++it) {
		if (*it != this && (*it)->fileName() == fileName) {
			_bitmap = (*it)->getBitmap();
			_fileName = fileName;
			if (update) updateImage();
			return true;
		}
	}

	_fileName = fileName;
	try {
		_bitmap = std::make_shared<Bitmap>(fileName);
		return true;
	}
	catch (...) {

	}
	return false;
}

bool
ImageWnd::loadByDescriptor(const ImageDescriptor &desctr) {
	std::wstring fileName = desctr.fileName();
	if (fileName.size() > 0 && loadImage(fileName, false)) {
		*(static_cast<ImageDescriptor *>(this)) = desctr;
		updateImage();
		updateClickThroughState();
		return true;
	}
	return false;
}

unsigned int
ImageWnd::width() const {
	return _bitmap->width();
}

unsigned int
ImageWnd::height() const {
	return _bitmap->height();
}

void
ImageWnd::enableTransparency(bool enable) {
	ImageDescriptor::enableTransparency(enable);
	updateImage();
}

void
ImageWnd::setTransparency(bool enable, unsigned char value) {
	ImageDescriptor::setTransparency(enable, value);
	updateImage();
}

void
ImageWnd::enableClickThrough(bool enable) {
	ImageDescriptor::enableClickThrough(enable);
	updateClickThroughState();
}

void
ImageWnd::updateClickThroughState() {
	LONG wndExStyle = GetWindowLong(_handle, GWL_EXSTYLE);
	if (_isClickThrough) {
		SetWindowLong(_handle, GWL_EXSTYLE, wndExStyle | WS_EX_TRANSPARENT);
	}
	else {
		SetWindowLong(_handle, GWL_EXSTYLE, wndExStyle & ~WS_EX_TRANSPARENT);
	}
}

void
ImageWnd::setPosition(int left, int top) {
	ImageDescriptor::setPosition(left, top);
	SetWindowPos(_handle, 0, _posLeft, _posTop, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void
ImageWnd::fromString(const std::wstring &desc) {
	ImageDescriptor desctr(desc);
	loadByDescriptor(desctr);
}

void
ImageWnd::updateImage() {
	if (_bitmap == NULL) return;
	POINT destPt;
	destPt.x = left();
	destPt.y = top();
	SIZE wndSize;
	wndSize.cx = width();
	wndSize.cy = height();
	HDC hDC = GetDC(_handle);

	bool withAlpha = false;
	HBITMAP bitmap = _bitmap->createDIBitmap(hDC, &withAlpha);
	
	HDC hMemDC = CreateCompatibleDC(hDC);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, bitmap);
	POINT srcPt = {0, 0};
	BLENDFUNCTION blendFunc;
	blendFunc.BlendOp = AC_SRC_OVER;
	blendFunc.BlendFlags = 0;
	blendFunc.SourceConstantAlpha = _transpEnabled ? (255 - _transpValue) : 255;
	blendFunc.AlphaFormat = withAlpha ? AC_SRC_ALPHA : 0;

	UpdateLayeredWindow(_handle, NULL, &destPt, &wndSize, hMemDC, &srcPt, 0, &blendFunc, ULW_ALPHA);
	
	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
	DeleteObject(bitmap);
	ReleaseDC(_handle, hDC);
}

void
ImageWnd::move(int x, int y) {
	setPosition(x, y);
	std::shared_ptr<Dialog> dialog = Dialog::instance();
	if (dialog != NULL) {
		std::shared_ptr<ImageDescriptor> entry = dialog->getSelectedEntry();
		if (entry.get() == this) {
			dialog->updateWndPos(x, y);
		}
	}
}

int
ImageWnd::wndProc(const WndMessage &wm) {
	switch (wm.msg) {
		case WM_PAINT:
		{
			PAINTSTRUCT  ps;
			BeginPaint(_handle, &ps);
			EndPaint(_handle, &ps);
			break;
		}
		case WM_LBUTTONDOWN:
			_mouseLeftHold = true;
			SetCapture(_handle);
			GetCursorPos(&_mousePos);
			break;
		case WM_LBUTTONUP:
			_mouseLeftHold = false;
			ReleaseCapture();
			if (Dialog::instance() != NULL) {
				SetForegroundWindow(Dialog::instance()->getHandle());
			}
			break;
		case WM_MOUSEMOVE:
			if (_mouseLeftHold) {
				POINT pos;
				GetCursorPos(&pos);
				int dx = pos.x - _mousePos.x;
				int dy = pos.y - _mousePos.y;
				move(left() + dx, top() + dy);
				_mousePos = pos;
			}
			break;
		default:
			return (int)DefWindowProc(_handle, wm.msg, wm.wParam, wm.lParam);
	}
	return 0;
}

}