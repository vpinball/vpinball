#include "StdAfx.h"


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
