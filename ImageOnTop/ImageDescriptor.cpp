#include "ImageDescriptor.h"
#include <string>
#include <sstream>
#include <windows.h>
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
		{"fn", ToUtf8(_name)},
		{"fp", ToUtf8(_fileName)},
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
	_name = FromUtf8(json.at("fn").get<std::string>());
	_fileName = FromUtf8(json.at("fp").get<std::string>());
}

}
