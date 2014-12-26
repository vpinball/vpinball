#pragma once

#ifdef RGB
 #undef RGB
#endif
#define RGB(r,g,b) ((COLORREF)(((DWORD)(r)) | (((DWORD)(g))<<8) | (((DWORD)(b))<<16)))

inline D3DCOLOR COLORREF_to_D3DCOLOR(const COLORREF c)
{
    // COLORREF: 0x00BBGGRR
    // D3DCOLOR: 0xAARRGGBB
    const COLORREF r = (c & 0x000000ff);
    const COLORREF g = (c & 0x0000ff00) >> 8;
    const COLORREF b = (c & 0x00ff0000) >> 16;

    return b | (g << 8) | (r << 16) | 0xff000000;
}

inline D3DXVECTOR4 COLORREF_to_D3DXVECTOR4(const COLORREF c)
{
    D3DXVECTOR4 cv;
    cv.x = (float)(c & 16711680) * (float)(1.0/16711680.0);
    cv.y = (float)(c & 65280) * (float)(1.0/65280.0);
    cv.z = (float)(c & 255) * (float)(1.0/255.0);
    cv.w = 1.0f;
    return cv;
}

inline D3DXVECTOR4 convertColor(const COLORREF c)
{
	const float r = (float)(c & 255) * (float)(1.0/255.0);
	const float g = (float)(c & 65280) * (float)(1.0/65280.0);
	const float b = (float)(c & 16711680) * (float)(1.0/16711680.0);
	D3DXVECTOR4 d(r,g,b,1.0f);
	return d;
}
