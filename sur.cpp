#include "stdafx.h"
#include "main.h"

/*#define SCALEX(x) ((int)((x - m_offx)*m_zoom))
#define SCALEY(y) ((int)((y - m_offy)*m_zoom))
#define SCALED(d) ((int)(d*m_zoom))*/

Sur::Sur(HDC hdc, float zoom, float offx, float offy, int width, int height)
	{
	m_hdc = hdc;
	m_zoom = zoom;
	m_offx = offx - ((width*0.5f)/zoom);
	m_offy = offy - ((height*0.5f)/zoom);
	}

Sur::~Sur()
	{
	}

void Sur::ScreenToSurface(int screenx, int screeny, float *fx, float *fy)
	{
	*fx = (screenx/m_zoom) + m_offx;
	*fy = (screeny/m_zoom) + m_offy;
	}

void Sur::SurfaceToScreen(float fx, float fy, int *screenx, int *screeny)
	{
	*screenx = SCALEX(fx);
	*screeny = SCALEY(fy);
	}