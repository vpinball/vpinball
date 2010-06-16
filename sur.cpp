#include "StdAfx.h"


Sur::Sur(const HDC hdc, const float zoom, const float offx, const float offy, const int width, const int height)
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

void Sur::ScreenToSurface(const int screenx, const int screeny, float * const fx, float * const fy) const
	{
	const float inv_zoom = 1.0f/m_zoom;
	*fx = (float)screenx*inv_zoom + m_offx;
	*fy = (float)screeny*inv_zoom + m_offy;
	}

void Sur::SurfaceToScreen(const float fx, const float fy, int * const screenx, int * const screeny) const
	{
	*screenx = SCALEXf(fx);
	*screeny = SCALEYf(fy);
	}
