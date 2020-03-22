#include "Bitmap.h"
#include <wincodec.h>
#include <sstream>
#include <vector>

#include "Exception.h"

// Return a string representing the specified Windows error
static std::wstring
getWindowsError(int err)
{
	LPVOID lpMsgBuf;

	if (!FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	)) {
		std::wostringstream s;
		// See http://msdn.microsoft.com/en-us/library/windows/desktop/ee719669%28v=vs.85%29.aspx
		s << L"Unknown error " << err << L" 0x" << std::hex << err;
		return s.str();
	}

	std::string r((char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
	return std::wstring(r.begin(), r.end());
}

// Exit with an error if the specified Windows API function fails
#define CHECK(x) do { \
	int ret; \
	if ((ret = (x)) != S_OK) { \
		throw RuntimeError(getWindowsError(ret)); \
	} \
} while(0)

namespace Swingl {

Bitmap::Bitmap(const std::wstring& filePath) {
	CComPtr<IWICImagingFactory> factory;
	CHECK(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)));

	CComPtr<IWICBitmapDecoder> decoder;
	CHECK(factory->CreateDecoderFromFilename(filePath.c_str(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder));

	CComPtr<IWICBitmapFrameDecode> frame;
	CHECK(decoder->GetFrame(0, &frame));
	CHECK(frame->GetSize(&_width, &_height));

	CComPtr<IWICFormatConverter> converter;
	CHECK(factory->CreateFormatConverter(&converter));
	CHECK(converter->Initialize(frame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeCustom));

	_source = converter;
}

Bitmap::~Bitmap() {
}

HBITMAP
Bitmap::createDIBitmap(HDC hDC) const {

	HBITMAP result = 0;

	unsigned int stride = (_width * 32 + 7) / 8;
	unsigned int bufferSize = stride * _height;

	std::vector<BYTE> buffer;
	buffer.resize(bufferSize);
	CHECK(_source->CopyPixels(0, stride, bufferSize, buffer.data()));

	BITMAPINFOHEADER bh;
	ZeroMemory(&bh, sizeof(BITMAPINFOHEADER));

	bh.biSize = sizeof(bh);
	bh.biWidth = _width;
	bh.biHeight = -(int)_height;	// Top-down DIB
	bh.biPlanes = 1;
	bh.biBitCount = 32;
	bh.biCompression = BI_RGB;

	BITMAPINFO bi;
	bi.bmiHeader = bh;

	return CreateDIBitmap(hDC, &bh, CBM_INIT, buffer.data(), &bi, DIB_RGB_COLORS);
}

}