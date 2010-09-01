#pragma once
// PinImage.h: interface for the PinImage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PINIMAGE_H__74A4733B_B66C_4C24_97AE_C7E9792E0635__INCLUDED_)
#define AFX_PINIMAGE_H__74A4733B_B66C_4C24_97AE_C7E9792E0635__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef RGB
#undef RGB
#endif
#define BGR(b,g,r) ((COLORREF)(((DWORD)(b)) | (((DWORD)(g))<<8) | (((DWORD)(r))<<16)))
#define RGB(r,g,b) ((COLORREF)(((DWORD)(r)) | (((DWORD)(g))<<8) | (((DWORD)(b))<<16)))

#define NOTRANSCOLOR  RGB(123,123,123)

#define MINBLACK 0x00070707
#define MINBLACKMASK 0x00f8f8f8

class PinImage : public ILoadable 
{
public:
	PinImage();
	virtual ~PinImage();

	LPDIRECTDRAWSURFACE7 m_pdsBuffer;

	LPDIRECTDRAWSURFACE7 m_pdsBufferColorKey;

	LPDIRECTDRAWSURFACE7 m_pdsBufferBackdrop;

	HRESULT SaveToStream(IStream *pstream, PinTable *pt);
	HRESULT LoadFromStream(IStream *pstream, int version, PinTable *pt);

	// ILoadable callback
	virtual BOOL LoadToken(int id, BiffReader *pbr);

	void EnsureColorKey();
	void EnsureBackdrop(const COLORREF color);
	void FreeStuff();
	void SetTransparentColor(const COLORREF color);
	void EnsureMaxTextureCoordinates();

	void EnsureHBitmap();
	void CreateGDIVersion();

	// width and height of texture can be different than width and height
	// of dd surface, since the surface has to be in powers of 2
	int m_width, m_height;

	// Filled at runtime, accounts for buffer space to meet the power of 2
	// requirement
	float m_maxtu, m_maxtv;

	char m_szName[MAXTOKEN];
	char m_szInternalName[MAXTOKEN];
	char m_szPath[MAX_PATH];

	COLORREF m_rgbTransparent;
	BOOL m_fTransparent; // Whether this picture actually contains transparent bits

	COLORREF m_rgbBackdropCur;

	HBITMAP m_hbmGDIVersion; // HBitmap at screen depth so GDI draws it fast

	PinBinary *m_ppb;  // if this image should be saved as a binary stream, otherwise just LZW compressed from the live bitmap
};

typedef int(CALLBACK *DDCreateFunction)(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter);

class PinDirectDraw
{
public:
	PinDirectDraw();
	~PinDirectDraw();

	HRESULT InitDD();

	LPDIRECTDRAW7 m_pDD;

	LPDIRECTDRAWSURFACE7 CreateTextureOffscreen(const int width, const int height);
	LPDIRECTDRAWSURFACE7 CreateFromFile(char *szfile, int * const pwidth, int * const pheight);
	LPDIRECTDRAWSURFACE7 CreateFromResource(const int id, int * const pwidth, int * const pheight);
	LPDIRECTDRAWSURFACE7 CreateFromHBitmap(HBITMAP hbm, int * const pwidth, int * const pheight);

	LPDIRECTDRAWSURFACE7 DecompressJPEG(PinImage * const ppi/*char *szfile*/, PinBinary * const ppb, int * const pwidth, int * const pheight);

	void SetOpaque(LPDIRECTDRAWSURFACE7 pdds, const int width, const int height);
	void SetOpaqueBackdrop(LPDIRECTDRAWSURFACE7 pdds, const COLORREF rgbTransparent, const COLORREF rgbBackdrop, const int width, const int height);

	BOOL SetAlpha(LPDIRECTDRAWSURFACE7 pdds, const COLORREF rgbTransparent, const int width, const int height);

	void CreateNextMipMapLevel(LPDIRECTDRAWSURFACE7 pdds);

	void Blur(LPDIRECTDRAWSURFACE7 pdds, const BYTE * const pbits, const int shadwidth, const int shadheight);

	void BlurAlpha(LPDIRECTDRAWSURFACE7 pdds);

	BOOL m_fHardwareAccel;
	BOOL m_fAlternateRender;
	BOOL m_fUseD3DBlit;

	// variables needed to initialize ddraw.dll
	HINSTANCE m_DDraw; 
	DDCreateFunction m_DDCreate;
};

#endif // !defined(AFX_PINIMAGE_H__74A4733B_B66C_4C24_97AE_C7E9792E0635__INCLUDED_)
