#include "StdAfx.h"


Sur::Sur(HDC hdc, float zoom, float offx, float offy, int width, int height)
	{
	m_hdc = hdc;
	m_zoom = zoom;
	const float inv_zoom = 0.5f/zoom;
	m_offx = offx - (float)width*inv_zoom;
	m_offy = offy - (float)height*inv_zoom;
	}

Sur::~Sur()
	{
	}

void Sur::ScreenToSurface(int screenx, int screeny, float *fx, float *fy)
	{
	const float inv_zoom = 1.0f/m_zoom;
	*fx = (float)screenx*inv_zoom + m_offx;
	*fy = (float)screeny*inv_zoom + m_offy;
	}

void Sur::SurfaceToScreen(float fx, float fy, int *screenx, int *screeny)
	{
	*screenx = SCALEXf(fx);
	*screeny = SCALEYf(fy);
	}
