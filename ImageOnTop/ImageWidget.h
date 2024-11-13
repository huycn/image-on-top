#pragma once
#include "Window.h"
#include "ImageDescriptor.h"
#include <set>
#include <memory>

namespace Swingl {

class WindowClass;
class ImageManager;
class Bitmap;

class ImageWidget : public Window, public ImageDescriptor
{
public:
	ImageWidget(WindowClass &wndClass);
	virtual ~ImageWidget();
	
	bool loadImage(const std::wstring &fileName, double scale, bool update = true);
	bool loadByDescriptor(const ImageDescriptor &desctr);

	unsigned int width() const;
	unsigned int height() const;

	virtual void enableTransparency(bool enable) override;
	virtual void setTransparency(bool enable, unsigned char value) override;
	virtual void enableClickThrough(bool enable) override;
	virtual void setPosition(int left, int top) override;
	virtual void setScale(double scale) override;
	virtual void fromString(const std::string &desc) override;

	virtual void show(bool value = true) override;

protected:
	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
	void updateClickThroughState();
	void updateImage();
	std::shared_ptr<Bitmap> getBitmap() const { return _bitmap; }

protected:
	void move(int x, int y);

	typedef std::set<ImageWidget *> ObjList;
	static ObjList _imgObj;
	std::shared_ptr<Bitmap> _bitmap;
	std::weak_ptr<ImageManager> _manager;

	// temporary data
	bool _mouseLeftHold;
	POINT _mousePos;
};

}