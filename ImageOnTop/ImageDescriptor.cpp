#include "ImageDescriptor.h"
#include <string>
#include <sstream>

namespace Swingl {

ImageDescriptor::ImageDescriptor(const std::wstring &desc)
: _isClickThrough(false), _transpEnabled(false), _transpValue(0), _posLeft(0), _posTop(0)
{
	fromString(desc);
}

void
ImageDescriptor::setName(const std::wstring& name) {
	_name = name;
}

void
ImageDescriptor::setFileName(const std::wstring& fname) {
	_fileName = fname;
}

void
ImageDescriptor::enableTransparency(bool enable) {
	_transpEnabled = enable;
}

void
ImageDescriptor::setTransparency(bool enable, unsigned char value) {
	_transpEnabled = enable;
	if (enable) {
		_transpValue = value;
	}
}

void
ImageDescriptor::enableClickThrough(bool enable) {
	_isClickThrough = enable;
}

void
ImageDescriptor::setPosition(int left, int top) {
	_posLeft = left;
	_posTop = top;
}

std::wstring
ImageDescriptor::toString() const {
	std::wostringstream output;
	output << (int)_isClickThrough << std::endl
		   << (int)_transpEnabled << std::endl
		   << (int)_transpValue << std::endl
		   << _posLeft << std::endl
		   << _posTop << std::endl
		   << _name << std::endl
		   << _fileName << std::endl;
	return output.str();
}

void
ImageDescriptor::fromString(const std::wstring &desc) {
	int isClickThrough, transpEnabled, transpValue;
	std::wistringstream input(desc);
	input >> isClickThrough >> transpEnabled >> transpValue >> _posLeft >> _posTop;
	std::getline(input, _name); // to remove endl
	std::getline(input, _name);
	std::getline(input, _fileName);
	_isClickThrough = isClickThrough != 0;
	_transpEnabled = transpEnabled != 0;
	_transpValue = static_cast<unsigned char>(transpValue);
}


}