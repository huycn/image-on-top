#pragma once
#include <string>

namespace Swingl {

class ImageDescriptor {
public:
	ImageDescriptor(const std::wstring &desc = std::wstring());
	virtual ~ImageDescriptor() {;}

	const std::wstring& name() const {return _name;}
	const std::wstring& fileName() const {return _fileName;}
	bool transparencyEnabled() const {return _transpEnabled;}
	unsigned char transparencyValue() const {return _transpValue;}
	bool isClickThrough() const {return _isClickThrough;}
	int left() const {return _posLeft;}
	int top() const {return _posTop;}

	virtual void setName(const std::wstring &name) {_name = name;}
	virtual void setFileName(const std::wstring &fname) {_fileName = fname;}
	virtual void enableTransparency(bool enable) {_transpEnabled = enable;}
	virtual void setTransparency(bool enable, unsigned char value) {_transpEnabled = enable; if (enable) _transpValue = value;}
	virtual void enableClickThrough(bool enable) {_isClickThrough = enable;}
	virtual void setPosition(int left, int top) {_posLeft = left; _posTop = top;}

	virtual void fromString(const std::wstring &desc);
	virtual std::wstring toString() const;

protected:
	bool _isClickThrough;
	bool _transpEnabled;
	unsigned char _transpValue;
	int _posLeft;
	int _posTop;
	std::wstring _name;
	std::wstring _fileName;
};

}