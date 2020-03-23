#pragma once
#include "Window.h"
#include "TrayMenu.h"

#include <memory>
#include <vector>

namespace Swingl {

class WindowClass;
class ImageDescriptor;
class Dialog;

enum class TransMode {
	NoTrans,
	TransDirect,
	TransAlpha
};

// This class create un message window to process SysTray messages
class ImageManager : public Window, public std::enable_shared_from_this<ImageManager> {
public:
	static std::shared_ptr<ImageManager> newInstance(const std::shared_ptr<WindowClass>& wndClass);
	virtual ~ImageManager();

	typedef std::vector<std::shared_ptr<ImageDescriptor>> ImageList;

	std::shared_ptr<WindowClass> getWndClass() const;
	TransMode transMode() const {return _transMode;}
	double transDelay() const {return _transDelay;}
	double transDuration() const {return _transDuration;}
	void setTransMode(TransMode mode, double delay, double duration);
	std::shared_ptr<ImageDescriptor> getEntry(int index) const;
	std::shared_ptr<ImageDescriptor> popEntry(int index);
	ImageList getEntriesList() const;
	int insertEntry(const ImageDescriptor &entry, int index = -1);
	void loadPrefFromRegistry();
	void savePrefToRegistry() const;

	std::shared_ptr<Dialog> getDialog() const;
	void setDialog(const std::shared_ptr<Dialog>& dialog);

	static const int kIconSysTrayCommand = WM_USER + 50;

protected:
	static const int framesPerSecond = 10;

	virtual LRESULT wndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

	void transStart();
	void transNextFrame();

private:
	ImageManager(const std::shared_ptr<WindowClass>& wndClass);
	void openDialog();

	std::weak_ptr<WindowClass> _wndClass;
	std::shared_ptr<TrayMenu> _trayMenu;
	std::shared_ptr<Dialog> _dialog;
    TransMode _transMode;
	double _transDelay;
	double _transDuration;
	bool _transInProgress;
	int _transNbFrames;
	int _transActualImg;
	int _transNextImg;
	int _transFrame;
	
	ImageList _imgList;
};


}