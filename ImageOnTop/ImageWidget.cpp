#include "ImageWidget.h"
#include "WindowClass.h"
#include "Dialog.h"
#include "Bitmap.h"
#include "ImageManager.h"
#include "WindowClass.h"
#include "Exception.h"

#include <iostream>

namespace Swingl {

ImageWidget::ObjList ImageWidget::_imgObj;

ImageWidget::ImageWidget(WindowClass &wndClass) {
	_manager = wndClass.imageManager();
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

ImageWidget::~ImageWidget() {
	if (_handle != NULL) {
		DestroyWindow(_handle);
		SetWindowLongPtr(_handle, GWLP_USERDATA, 0);
	}
	_imgObj.erase(this);
}

bool
ImageWidget::loadImage(const std::wstring &fileName, double scale, bool update) {
	for (ObjList::const_iterator it = _imgObj.begin(); it != _imgObj.end(); ++it) {
		if (*it != this && (*it)->fileName() == fileName && std::abs((*it)->scale() - scale) <= 0.00001) {
			_bitmap = (*it)->getBitmap();
			_fileName = fileName;
			if (update) updateImage();
			return true;
		}
	}

	_fileName = fileName;
	try {
		_bitmap = std::make_shared<Bitmap>(fileName, scale);
		return true;
	}
	catch (const std::exception& ex) {
		std::cerr << ex.what();
	}
	catch (...) {

	}
	return false;
}

bool
ImageWidget::loadByDescriptor(const ImageDescriptor &desctr) {
	std::wstring fileName = desctr.fileName();
	if (fileName.size() > 0 && loadImage(fileName, desctr.scale(), false)) {
		*(static_cast<ImageDescriptor *>(this)) = desctr;
		updateImage();
		updateClickThroughState();
		return true;
	}
	return false;
}

unsigned int
ImageWidget::width() const {
	return _bitmap->width();
}

unsigned int
ImageWidget::height() const {
	return _bitmap->height();
}

void
ImageWidget::enableTransparency(bool enable) {
	ImageDescriptor::enableTransparency(enable);
	updateImage();
}

void
ImageWidget::setTransparency(bool enable, unsigned char value) {
	ImageDescriptor::setTransparency(enable, value);
	updateImage();
}

void
ImageWidget::enableClickThrough(bool enable) {
	ImageDescriptor::enableClickThrough(enable);
	updateClickThroughState();
}

void
ImageWidget::updateClickThroughState() {
	LONG wndExStyle = GetWindowLong(_handle, GWL_EXSTYLE);
	if (_isClickThrough) {
		SetWindowLong(_handle, GWL_EXSTYLE, wndExStyle | WS_EX_TRANSPARENT);
	}
	else {
		SetWindowLong(_handle, GWL_EXSTYLE, wndExStyle & ~WS_EX_TRANSPARENT);
	}
}

void
ImageWidget::setPosition(int left, int top) {
	ImageDescriptor::setPosition(left, top);
	SetWindowPos(_handle, 0, _posLeft, _posTop, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void
ImageWidget::setScale(double scale) {
	if (std::abs(this->scale() - scale) <= 0.00001) {
		return;
	}
	ImageDescriptor::setScale(scale);
	if (_bitmap != nullptr) {
		try {
			_bitmap = std::make_shared<Bitmap>(_fileName, scale);
			updateImage();
		}
		catch (...) {
		}
	}
}

void
ImageWidget::fromString(const std::string &desc) {
	ImageDescriptor desctr(desc);
	loadByDescriptor(desctr);
}

void
ImageWidget::updateImage() {
	if (_bitmap == NULL) return;
	POINT destPt;
	destPt.x = left();
	destPt.y = top();
	SIZE wndSize;
	wndSize.cx = width();
	wndSize.cy = height();
	HDC hDC = GetDC(_handle);

	HBITMAP bitmap = _bitmap->createDIBitmap(hDC);
	
	HDC hMemDC = CreateCompatibleDC(hDC);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, bitmap);
	POINT srcPt = {0, 0};
	BLENDFUNCTION blendFunc;
	blendFunc.BlendOp = AC_SRC_OVER;
	blendFunc.BlendFlags = 0;
	blendFunc.SourceConstantAlpha = _transpEnabled ? (255 - _transpValue) : 255;
	blendFunc.AlphaFormat = AC_SRC_ALPHA;

	UpdateLayeredWindow(_handle, NULL, &destPt, &wndSize, hMemDC, &srcPt, 0, &blendFunc, ULW_ALPHA);
	
	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
	DeleteObject(bitmap);
	ReleaseDC(_handle, hDC);
}

void
ImageWidget::move(int x, int y) {
	setPosition(x, y);
	if (auto manager = _manager.lock()) {
		if (auto dialog = manager->getDialog()) {
			std::shared_ptr<ImageDescriptor> entry = dialog->getSelectedEntry();
			if (entry.get() == this) {
				dialog->updateSelectedItemOrigin(x, y);
			}
		}
	}
}

void
ImageWidget::show(bool value) {
	ShowWindow(_handle, value ? SW_SHOWNOACTIVATE : SW_HIDE);
}

LRESULT
ImageWidget::wndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
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
			if (auto manager = _manager.lock()) {
				if (auto dialog = manager->getDialog()) {
					SetForegroundWindow(dialog->getHandle());
				}
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
			return DefWindowProc(_handle, msg, wParam, lParam);
	}
	return NULL;
}

}