#pragma once
#include "Window.h"
#include "ImageDescriptor.h"
#include <set>
#include <memory>

namespace Swingl {

class WndClass;
class Bitmap;

class ImageWnd : public Window, public ImageDescriptor
{
public:
	ImageWnd(WndClass &wndClass);
	virtual ~ImageWnd();
	bool loadImage(const std::wstring &fileName, bool update = true);
	bool loadByDescriptor(const ImageDescriptor &desctr);

	unsigned int width() const;
	unsigned int height() const;

	virtual void enableTransparency(bool enable);
	virtual void setTransparency(bool enable, unsigned char value);
	virtual void enableClickThrough(bool enable);
	virtual void setPosition(int left, int top);
	virtual void fromString(const std::wstring &desc);

	void show(bool value = true) {ShowWindow(_handle, value ? SW_SHOWNOACTIVATE : SW_HIDE);}

protected:
	virtual int wndProc(const WndMessage &wm);
	void updateClickThroughState();
	void updateImage();
	std::shared_ptr<Bitmap> getBitmap() const { return _bitmap; }

protected:
	void move(int x, int y);

	typedef std::set<ImageWnd *> ObjList;
	static ObjList _imgObj;
	std::shared_ptr<Bitmap> _bitmap;

	// temporary data
	bool _mouseLeftHold;
	POINT _mousePos;

private:
	ImageWnd(const ImageWnd &);
};

}