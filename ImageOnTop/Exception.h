#pragma once
#include <string>

namespace Swingl {

class Exception {
public:
	Exception(const std::wstring &msg) : _msg(msg) {;}
	virtual ~Exception() {;}

	const wchar_t * what() const {return _msg.c_str();}

protected:
	std::wstring _msg;
};


class RuntimeError : public Exception {
public:
	RuntimeError(const std::wstring &msg) : Exception(msg) {;}
	virtual ~RuntimeError() {;}
};

}