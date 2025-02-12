// Standalone cannot use FreeImage.h because there are typedefs that get redefined.
// This file has the bare minimum needed to compile the standalone version.

#ifndef FREEIMAGE_H
#define FREEIMAGE_H

#define FI_DEFAULT(x)	= x
#define FI_ENUM(x)      enum x
#define FI_STRUCT(x)	struct x

#define FI_RGBA_RED				2
#define FI_RGBA_GREEN			1
#define FI_RGBA_BLUE			0
#define FI_RGBA_ALPHA			3

FI_ENUM(FREE_IMAGE_FORMAT) {
	FIF_UNKNOWN = -1,
	FIF_BMP		= 0,
	FIF_ICO		= 1,
	FIF_JPEG	= 2,
	FIF_JNG		= 3,
	FIF_KOALA	= 4,
	FIF_LBM		= 5,
	FIF_IFF = FIF_LBM,
	FIF_MNG		= 6,
	FIF_PBM		= 7,
	FIF_PBMRAW	= 8,
	FIF_PCD		= 9,
	FIF_PCX		= 10,
	FIF_PGM		= 11,
	FIF_PGMRAW	= 12,
	FIF_PNG		= 13,
	FIF_PPM		= 14,
	FIF_PPMRAW	= 15,
	FIF_RAS		= 16,
	FIF_TARGA	= 17,
	FIF_TIFF	= 18,
	FIF_WBMP	= 19,
	FIF_PSD		= 20,
	FIF_CUT		= 21,
	FIF_XBM		= 22,
	FIF_XPM		= 23,
	FIF_DDS		= 24,
	FIF_GIF     = 25,
	FIF_HDR		= 26,
	FIF_FAXG3	= 27,
	FIF_SGI		= 28,
	FIF_EXR		= 29,
	FIF_J2K		= 30,
	FIF_JP2		= 31,
	FIF_PFM		= 32,
	FIF_PICT	= 33,
	FIF_RAW		= 34,
	FIF_WEBP	= 35,
	FIF_JXR		= 36
};

FI_ENUM(FREE_IMAGE_TYPE) {
	FIT_UNKNOWN = 0,	//! unknown type
	FIT_BITMAP  = 1,	//! standard image			: 1-, 4-, 8-, 16-, 24-, 32-bit
	FIT_UINT16	= 2,	//! array of unsigned short	: unsigned 16-bit
	FIT_INT16	= 3,	//! array of short			: signed 16-bit
	FIT_UINT32	= 4,	//! array of unsigned long	: unsigned 32-bit
	FIT_INT32	= 5,	//! array of long			: signed 32-bit
	FIT_FLOAT	= 6,	//! array of float			: 32-bit IEEE floating point
	FIT_DOUBLE	= 7,	//! array of double			: 64-bit IEEE floating point
	FIT_COMPLEX	= 8,	//! array of FICOMPLEX		: 2 x 64-bit IEEE floating point
	FIT_RGB16	= 9,	//! 48-bit RGB image			: 3 x 16-bit
	FIT_RGBA16	= 10,	//! 64-bit RGBA image		: 4 x 16-bit
	FIT_RGBF	= 11,	//! 96-bit RGB float image	: 3 x 32-bit IEEE floating point
	FIT_RGBAF	= 12,	//! 128-bit RGBA float image	: 4 x 32-bit IEEE floating point
	FIT_RGB16F	= 13,	//! 48-bit RGB float image			: 3 x 16-bit IEEE floating point
	FIT_RGBA16F	= 14	//! 64-bit RGBA float image		: 4 x 16-bit IEEE floating point
};

FI_ENUM(FREE_IMAGE_FILTER) {
	FILTER_BOX		  = 0,	//! Box, pulse, Fourier window, 1st order (constant) b-spline
	FILTER_BICUBIC	  = 1,	//! Mitchell & Netravali's two-param cubic filter
	FILTER_BILINEAR   = 2,	//! Bilinear filter
	FILTER_BSPLINE	  = 3,	//! 4th order (cubic) b-spline
	FILTER_CATMULLROM = 4,	//! Catmull-Rom spline, Overhauser spline
	FILTER_LANCZOS3	  = 5	//! Lanczos3 filter
};

FI_STRUCT (FIBITMAP) { void *data; };
FI_STRUCT (FIMEMORY) { void *data; };

#define EXR_ALLOW_FOR_FP16	0x80000
#define PNG_Z_BEST_COMPRESSION		0x0009	//! save using ZLib level 9 compression flag (default value is 6)

extern "C" {

FIBITMAP* FreeImage_Allocate(int width, int height, int bpp, unsigned red_mask FI_DEFAULT(0), unsigned green_mask FI_DEFAULT(0), unsigned blue_mask FI_DEFAULT(0));
void FreeImage_CloseMemory(FIMEMORY *stream);
FIBITMAP* FreeImage_ConvertToRGBF(FIBITMAP *dib);
FIBITMAP* FreeImage_ConvertTo24Bits(FIBITMAP *dib);
FIBITMAP* FreeImage_ConvertTo32Bits(FIBITMAP *dib);
BOOL FreeImage_FIFSupportsReading(FREE_IMAGE_FORMAT fif);
BYTE* FreeImage_GetBits(FIBITMAP *dib);
unsigned FreeImage_GetBPP(FIBITMAP *dib);
FREE_IMAGE_FORMAT FreeImage_GetFIFFromFilename(const char *filename);
FREE_IMAGE_FORMAT FreeImage_GetFileType(const char *filename, int size FI_DEFAULT(0));
FREE_IMAGE_FORMAT FreeImage_GetFileTypeFromMemory(FIMEMORY *stream, int size FI_DEFAULT(0));
unsigned FreeImage_GetHeight(FIBITMAP *dib);
FREE_IMAGE_TYPE FreeImage_GetImageType(FIBITMAP *dib);
unsigned FreeImage_GetPitch(FIBITMAP *dib);
unsigned FreeImage_GetWidth(FIBITMAP *dib);
BOOL FreeImage_IsTransparent(FIBITMAP *dib);
FIBITMAP* FreeImage_Load(FREE_IMAGE_FORMAT fif, const char *filename, int flags FI_DEFAULT(0));
FIBITMAP* FreeImage_LoadFromMemory(FREE_IMAGE_FORMAT fif, FIMEMORY *stream, int flags FI_DEFAULT(0));
FIMEMORY* FreeImage_OpenMemory(BYTE *data FI_DEFAULT(0), DWORD size_in_bytes FI_DEFAULT(0));
FIBITMAP* FreeImage_Rescale(FIBITMAP *dib, int dst_width, int dst_height, FREE_IMAGE_FILTER filter FI_DEFAULT(FILTER_CATMULLROM));
BOOL FreeImage_Save(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, const char *filename, int flags FI_DEFAULT(0));
void FreeImage_Unload(FIBITMAP *dib);

}

#endif
