// PinImage.h: interface for the PinImage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PINIMAGE_H__74A4733B_B66C_4C24_97AE_C7E9792E0635__INCLUDED_)
#define AFX_PINIMAGE_H__74A4733B_B66C_4C24_97AE_C7E9792E0635__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
	void EnsureBackdrop(COLORREF color);
	void FreeStuff();
	void SetTransparentColor(COLORREF color);
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

	BOOL m_fUnneededAfterCache; // if true; this image does not need to be loaded when we are playing from a render cache

	COLORREF m_rgbBackdropCur;

	HBITMAP m_hbmGDIVersion; // HBitmap at screen depth so GDI draws it fast

	PinBinary *m_ppb;  // if this image should be saved as a binary stream, otherwise just LZW compressed from the live bitmap
	//BOOL m_fBinary; // whether to save this image as a binary stream, or LZW compressed from the live bitmap

	//char *m_pdata; // Copy of the buffer data so we can save it out
	//int m_cdata;
};

typedef int(CALLBACK *DDCreateFunction)(GUID FAR *lpGUID, LPVOID *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter);

class PinDirectDraw
{
public:
	PinDirectDraw();
	~PinDirectDraw();

	HRESULT InitDD();

	LPDIRECTDRAW7 m_pDD;

	LPDIRECTDRAWSURFACE7 CreateTextureOffscreen(int width, int height);
	LPDIRECTDRAWSURFACE7 CreateFromFile(char *szfile, int *pwidth, int *pheight);
	LPDIRECTDRAWSURFACE7 CreateFromResource(int id, int *pwidth, int *pheight);
	LPDIRECTDRAWSURFACE7 CreateFromHBitmap(HBITMAP hbm, int *pwidth, int *pheight);

	LPDIRECTDRAWSURFACE7 DecompressJPEG(PinImage *ppi/*char *szfile*/, PinBinary *ppb, int *pwidth, int *pheight);

	void SetOpaque(LPDIRECTDRAWSURFACE7 pdds, int width, int height);
	void SetOpaqueBackdrop(LPDIRECTDRAWSURFACE7 pdds, COLORREF rgbTransparent, COLORREF rgbBackdrop, int width, int height);

	BOOL SetAlpha(LPDIRECTDRAWSURFACE7 pdds, COLORREF rgbTransparent, int width, int height);

	void CreateNextMipMapLevel(LPDIRECTDRAWSURFACE7 pdds);

	void Blur(LPDIRECTDRAWSURFACE7 pdds, BYTE *pbits, int shadwidth, int shadheight);

	void BlurAlpha(LPDIRECTDRAWSURFACE7 pdds);

	BOOL m_fHardwareAccel;

	// variables needed to initialize ddraw.dll
	HINSTANCE m_DDraw; 
	DDCreateFunction m_DDCreate;
};

#endif // !defined(AFX_PINIMAGE_H__74A4733B_B66C_4C24_97AE_C7E9792E0635__INCLUDED_)
