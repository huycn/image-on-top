#include "ImageDescriptor.h"
#include <string>
#include <sstream>
#include "json.hpp"

namespace Swingl {

ImageDescriptor::ImageDescriptor(const std::string &desc)
: _isClickThrough(false), _transpEnabled(false), _transpValue(0), _posLeft(0), _posTop(0), _scale(1)
{
	if (!desc.empty()) {
		fromString(desc);
	}
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

void
ImageDescriptor::setScale(double scale) {
	_scale = scale;
}

std::string
ImageDescriptor::toString() const {
	return nlohmann::json{
		{"ct", _isClickThrough},
		{"te", _transpEnabled},
		{"tv", _transpValue},
		{"px", _posLeft},
		{"py", _posTop},
		{"sc", _scale},
		{"fn", _name},
		{"fp", _fileName},
	}.dump();
}

void
ImageDescriptor::fromString(const std::string &desc) {
	auto json = nlohmann::json::parse(desc);
	json.at("ct").get_to(_isClickThrough);
	json.at("te").get_to(_transpEnabled);
	json.at("tv").get_to(_transpValue);
	json.at("px").get_to(_posLeft);
	json.at("py").get_to(_posTop);
	json.at("sc").get_to(_scale);
	json.at("fn").get_to(_name);
	json.at("fp").get_to(_fileName);
}

}
