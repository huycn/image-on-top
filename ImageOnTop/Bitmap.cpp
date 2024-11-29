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

static void applyCropping(WICRect& rect, double cropping[4]) {
	double width = rect.Width;
	double height = rect.Height;
	rect.X = (INT)std::round(cropping[0] * width);
	rect.Width -= rect.X;
	if (rect.Width > std::round(cropping[2] * width)) {
		rect.Width -= (INT)std::round(cropping[2] * width);
	}

	rect.Y = (INT)std::round(cropping[1] * height);
	rect.Height -= rect.Y;
	if (rect.Height > std::round(cropping[3] * height)) {
		rect.Height -= (INT)std::round(cropping[3] * height);
	}
}

namespace Swingl {

Bitmap::Bitmap(const std::wstring& filePath, double scale, double cropping[4]) {
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

	if (std::abs(scale - 1.0) > 0.00001 && scale > 0.0) {
		CComPtr<IWICBitmapScaler> scaler;
		CHECK(factory->CreateBitmapScaler(&scaler));
		_width = (UINT)std::round(_width * scale);
		_height = (UINT)std::round(_height * scale);
		CHECK(scaler->Initialize(_source, _width, _height, scale < 1.0 ? WICBitmapInterpolationModeFant : WICBitmapInterpolationModeHighQualityCubic));
		_source = scaler;
	}

	if (std::abs(cropping[0] - 1.0) > 0.00001 && cropping[0] > 0.0
		|| std::abs(cropping[1] - 1.0) > 0.00001 && cropping[1] > 0.0
		|| std::abs(cropping[2] - 1.0) > 0.00001 && cropping[2] > 0.0
		|| std::abs(cropping[3] - 1.0) > 0.00001 && cropping[3] > 0.0) {

		WICRect cropRect = { 0, 0, (INT)_width, (INT)_height };
		applyCropping(cropRect, cropping);
		CComPtr<IWICBitmapClipper> clipper;
		CHECK(factory->CreateBitmapClipper(&clipper));
		CHECK(clipper->Initialize(_source, &cropRect));
		_source = clipper;
	}
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