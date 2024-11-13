#pragma once
#include <string>

namespace Swingl {

class ImageDescriptor {
public:
	ImageDescriptor(const std::string &desc = std::string());
	virtual ~ImageDescriptor() {;}

	const std::wstring& name() const { return _name; }
	const std::wstring& fileName() const { return _fileName; }
	bool transparencyEnabled() const { return _transpEnabled; }
	unsigned char transparencyValue() const { return _transpValue; }
	bool isClickThrough() const { return _isClickThrough; }
	int left() const { return _posLeft; }
	int top() const { return _posTop; }
	double scale() const { return _scale; }

	virtual void setName(const std::wstring& name);
	virtual void setFileName(const std::wstring& fname);
	virtual void enableTransparency(bool enable);
	virtual void setTransparency(bool enable, unsigned char value);
	virtual void enableClickThrough(bool enable);
	virtual void setPosition(int left, int top);
	virtual void setScale(double scale);

	virtual void fromString(const std::string &desc);
	virtual std::string toString() const;

protected:
	bool _isClickThrough;
	bool _transpEnabled;
	unsigned char _transpValue;
	int _posLeft;
	int _posTop;
	double _scale;
	std::wstring _name;
	std::wstring _fileName;
};

}
