#pragma once
#include <string>
#include <memory>
#include <windows.h>
#include <atlbase.h>

struct IWICBitmapSource;

namespace Swingl {

class Bitmap {
public:
    Bitmap(const std::wstring& filePath, double scale);
    ~Bitmap();

    unsigned int width() const { return _width; }
    unsigned int height() const { return _height; }

    HBITMAP createDIBitmap(HDC hDC) const;

private:
    CComPtr<IWICBitmapSource> _source;
    UINT _width = 0;
    UINT _height = 0;
};

}
