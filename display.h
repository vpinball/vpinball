#pragma once
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

inline unsigned int RGB_TO_BGR(const unsigned int c)
{
	return (c&0x00FF00u) | ((c>>16)&0xFFu) | ((c&0xFFu)<<16);
}

#define     RGBA_TO_D3DARGB(r,g,b,a)	((((long)((a) * 255.0f)) << 24) | (((long)((r) * 255.0f)) << 16) | (((long)((g) * 255.0f)) << 8) | (long)((b) * 255.0f))
#define     RGBA_TO_D3DRGBA(r,g,b,a)	((((long)((r) * 255.0f)) << 24) | (((long)((g) * 255.0f)) << 16) | (((long)((b) * 255.0f)) << 8) | (long)((a) * 255.0f))

#endif
