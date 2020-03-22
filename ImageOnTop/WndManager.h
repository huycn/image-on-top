#pragma once
#include "Window.h"
#include "TrayMenu.h"

#include <memory>
#include <vector>

namespace Swingl {

class WndMessage;
class WndClass;
class ImageDescriptor;

enum class TransMode {
	NoTrans,
	TransDirect,
	TransAlpha
};

// This class create un message window to process SysTray messages
class WndManager : public Window {
public:
	~WndManager();

	typedef std::vector<std::shared_ptr<ImageDescriptor> > ImageList;

	std::shared_ptr<WndClass> getWndClass() const {return _wndClass;}
	TransMode transMode() const {return _transMode;}
	double transDelay() const {return _transDelay;}
	double transDuration() const {return _transDuration;}
	void setTransMode(TransMode mode, double delay, double duration);
	std::shared_ptr<ImageDescriptor> getEntry(int index) const;
	std::shared_ptr<ImageDescriptor> popEntry(int index);
	void getEntriesList(ImageList &list) const;
	int insertEntry(const ImageDescriptor &entry, int index = -1);
	void loadPrefFromRegistry();
	void savePrefToRegistry() const;

protected:
	friend class WndClass;

	static const int kIconSysTrayCommand = WM_USER + 50;
	static const int framesPerSecond = 10;

	WndManager(const std::shared_ptr<WndClass> &wndClass);

	int wndProc(const WndMessage &m);

	void transStart();
	void transNextFrame();

private:
	void openDialog();

	std::shared_ptr<WndClass> _wndClass;
	std::shared_ptr<TrayMenu> _trayMenu;
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