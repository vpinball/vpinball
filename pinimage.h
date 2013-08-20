#include "Texture.h"
#pragma once
#if !defined(AFX_PINIMAGE_H__74A4733B_B66C_4C24_97AE_C7E9792E0635__INCLUDED_)
#define AFX_PINIMAGE_H__74A4733B_B66C_4C24_97AE_C7E9792E0635__INCLUDED_

#ifdef RGB
#undef RGB
#endif
#define BGR(b,g,r) ((COLORREF)(((DWORD)(b)) | (((DWORD)(g))<<8) | (((DWORD)(r))<<16)))
#define RGB(r,g,b) ((COLORREF)(((DWORD)(r)) | (((DWORD)(g))<<8) | (((DWORD)(b))<<16)))

#define NOTRANSCOLOR  RGB(123,123,123)

//#define MINBLACK 0x00070707
//#define MINBLACKMASK 0x00f8f8f8

typedef int(CALLBACK *DDCreateFunction)(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter);

class PinDirectDraw
{
public:
	PinDirectDraw();
	~PinDirectDraw();

	HRESULT InitDD();

	LPDIRECTDRAW7 m_pDD;

	BaseTexture* CreateTextureOffscreen(const int width, const int height);
	BaseTexture* CreateFromFile(char *szfile, int * const pwidth, int * const pheight, int& originalWidth, int& originalHeight);
	BaseTexture* CreateFromResource(const int id, int * const pwidth, int * const pheight);
	BaseTexture* CreateFromHBitmap(HBITMAP hbm, int * const pwidth, int * const pheight);

	void SetOpaque(BaseTexture* pdds, const int width, const int height);
	void SetOpaqueBackdrop(BaseTexture* pdds, const COLORREF rgbTransparent, const COLORREF rgbBackdrop, const int width, const int height);

	BOOL SetAlpha(BaseTexture* pdds, const COLORREF rgbTransparent, const int width, const int height);

	void CreateNextMipMapLevel(BaseTexture* pdds);

	void Blur(BaseTexture* pdds, const BYTE * const pbits, const int shadwidth, const int shadheight);

	BOOL m_fHardwareAccel;
	BOOL m_fAlternateRender;

	// variables needed to initialize ddraw.dll
	HINSTANCE m_DDraw;
	DDCreateFunction m_DDCreate;
};

#endif // !defined(AFX_PINIMAGE_H__74A4733B_B66C_4C24_97AE_C7E9792E0635__INCLUDED_)
