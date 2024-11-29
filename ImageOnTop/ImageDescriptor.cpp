#include "ImageDescriptor.h"
#include <string>
#include <sstream>
#include <windows.h>
#include "json.hpp"

namespace Swingl {

ImageDescriptor::ImageDescriptor(const std::string &desc)
: _isClickThrough(false), _transpEnabled(false), _transpValue(0), _posLeft(0), _posTop(0), _scale(1), _crop{0}
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

void
ImageDescriptor::setCropping(double left, double top, double right, double bottom) {
	_crop[0] = left;
	_crop[1] = top;
	_crop[2] = right;
	_crop[3] = bottom;
}

std::string ToUtf8(std::wstring_view utf16_string)
{
	int target_length = ::WideCharToMultiByte(
		CP_UTF8, WC_ERR_INVALID_CHARS, utf16_string.data(),
		(int)utf16_string.length(), nullptr, 0, nullptr, nullptr);
	std::string utf8_string;
	if (target_length <= 0 || target_length > utf8_string.max_size()) {
		return utf8_string;
	}
	utf8_string.resize(target_length);
	int converted_length = ::WideCharToMultiByte(
		CP_UTF8, WC_ERR_INVALID_CHARS, utf16_string.data(),
		(int)utf16_string.length(), utf8_string.data(), target_length, nullptr, nullptr);
	if (converted_length == 0) {
		return std::string();
	}
	return utf8_string;
}

std::wstring FromUtf8(std::string_view utf8_string)
{
	int target_length = ::MultiByteToWideChar(
		CP_UTF8, MB_ERR_INVALID_CHARS, utf8_string.data(),
		(int)utf8_string.length(), nullptr, 0);
	std::wstring utf16_string;
	if (target_length <= 0 || target_length > utf16_string.max_size()) {
		return utf16_string;
	}
	utf16_string.resize(target_length);
	int converted_length = ::MultiByteToWideChar(
		CP_UTF8, MB_ERR_INVALID_CHARS, utf8_string.data(),
		(int)utf8_string.length(), utf16_string.data(), target_length);
	if (converted_length == 0) {
		return std::wstring();
	}
	return utf16_string;
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
		{"bl", _crop[0]},
		{"bt", _crop[1]},
		{"br", _crop[2]},
		{"bb", _crop[3]},
		{"fn", ToUtf8(_name)},
		{"fp", ToUtf8(_fileName)},
	}.dump();
}

void
ImageDescriptor::fromString(const std::string &desc) {
	auto json = nlohmann::json::parse(desc);
	_isClickThrough, json.value("ct", false);
	_transpEnabled = json.value("te", false);
	_transpValue = json.value("tv", (unsigned char)0);
	_posLeft = json.value("px", 0);
	_posTop = json.value("py", 0);
	_scale = json.value("sc", 1.0);
	_crop[0] = json.value("bl", 0.0);
	_crop[1] = json.value("bt", 0.0);
	_crop[2] = json.value("br", 0.0);
	_crop[3] = json.value("bb", 0.0);
	_name = FromUtf8(json.value("fn", std::string()));
	_fileName = FromUtf8(json.value("fp", std::string()));
}

}
