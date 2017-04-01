#include "StdAfx.h"

#define MAX_SUR_PT_CACHE 1000
static POINT m_ptCache[MAX_SUR_PT_CACHE * 2];
static const std::vector<DWORD> m_ptCache_idx(MAX_SUR_PT_CACHE * 2, 2);

PaintSur::PaintSur(const HDC hdc, const float zoom, const float offx, const float offy, const int width, const int height, ISelect * const psel)
   : Sur(hdc, zoom, offx, offy, width, height)
{
   m_hbr = CreateSolidBrush(RGB(255, 255, 255));
   m_hpnOutline = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
   m_hpnLine = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
   m_psel = psel;
}

PaintSur::~PaintSur()
{
   SelectObject(m_hdc, GetStockObject(BLACK_PEN));
   SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

   DeleteObject(m_hpnLine);
   DeleteObject(m_hbr);
   DeleteObject(m_hpnOutline);
}

void PaintSur::Line(const float x, const float y, const float x2, const float y2)
{
   const int ix = SCALEXf(x);
   const int iy = SCALEYf(y);
   const int ix2 = SCALEXf(x2);
   const int iy2 = SCALEYf(y2);

   SelectObject(m_hdc, m_hpnLine);

   ::MoveToEx(m_hdc, ix, iy, NULL);
   ::LineTo(m_hdc, ix2, iy2);
   ::LineTo(m_hdc, ix, iy); // To get the last pixel drawn //!! meh
}

void PaintSur::Rectangle(const float x, const float y, const float x2, float y2)
{
   const int ix = SCALEXf(x);
   const int iy = SCALEYf(y);
   const int ix2 = SCALEXf(x2);
   const int iy2 = SCALEYf(y2);

   SelectObject(m_hdc, m_hbr);
   SelectObject(m_hdc, m_hpnOutline);

   ::Rectangle(m_hdc, ix, iy, ix2, iy2);
}

void PaintSur::Rectangle2(const int x, const int y, const int x2, const int y2)
{
   SelectObject(m_hdc, m_hbr);
   SelectObject(m_hdc, m_hpnOutline);

   if (m_fNullBorder)
   {
      ::Rectangle(m_hdc, x, y, x2 + 1, y2 + 1);
   }
   else
   {
      ::Rectangle(m_hdc, x, y, x2, y2);
   }
}

void PaintSur::Ellipse(float centerx, float centery, float radius)
{
   const int ix = SCALEXf(centerx);
   const int iy = SCALEYf(centery);
   const int ir = SCALEDf(radius);

   SelectObject(m_hdc, m_hbr);
   SelectObject(m_hdc, m_hpnOutline);

   ::Ellipse(m_hdc, ix - ir, iy - ir, ix + ir, iy + ir);
}

void PaintSur::Ellipse2(const float centerx, const float centery, const int radius)
{
   const int ix = SCALEXf(centerx);
   const int iy = SCALEYf(centery);
   const int ir = radius;

   SelectObject(m_hdc, m_hbr);
   SelectObject(m_hdc, m_hpnOutline);

   ::Ellipse(m_hdc, ix - ir, iy - ir, ix + ir + 1, iy + ir + 1);
}

void PaintSur::Polygon(const Vertex2D * const rgv, const int count)
{
   std::vector<POINT> rgpt(count);

   for (int i = 0; i < count; i++)
   {
      rgpt[i].x = SCALEXf(rgv[i].x);
      rgpt[i].y = SCALEYf(rgv[i].y);
   }

   SelectObject(m_hdc, m_hbr);
   SelectObject(m_hdc, m_hpnOutline);

   ::Polygon(m_hdc, rgpt.data(), count);
}

// copy-pasted from above
void PaintSur::Polygon(const std::vector<RenderVertex> &rgv)
{
   std::vector<POINT> rgpt(rgv.size());

   for (unsigned i = 0; i < rgv.size(); i++)
   {
      rgpt[i].x = SCALEXf(rgv[i].x);
      rgpt[i].y = SCALEYf(rgv[i].y);
   }

   SelectObject(m_hdc, m_hbr);
   SelectObject(m_hdc, m_hpnOutline);

   ::Polygon(m_hdc, rgpt.data(), (int)rgv.size());

   if (rgv.size() == 4)
   {
      POINT pnt;
      ::MoveToEx(m_hdc, rgpt[0].x, rgpt[0].y, &pnt);
      ::LineTo(m_hdc, rgpt[2].x, rgpt[2].y);
      ::MoveToEx(m_hdc, rgpt[1].x, rgpt[1].y, NULL);
      ::LineTo(m_hdc, rgpt[3].x, rgpt[3].y);
      ::MoveToEx(m_hdc, pnt.x, pnt.y, NULL);
   }
}

/*
RGBA acor(RGBA *src, rect2f &rgn)
{
   vec3f r = vec3f::org;
}

void StretchBltAlpha(BITMAP dest, int destx,int desty, int destw, int desth, BITMAP src, int srcx,int srcy,int srcw,int srch)
{
	int sample = 1;

	//BYTE *_dst =
	//memcpy(_dst, src, sizeof(unsigned int)*4 * destw * desth);
	int _srcw = src.width;
	int _srch = src.height;
	int _dstw = dest.width;

	if (sample == 1) //point sampling
	{
		const float ratiox = float(srcw) / float(destw);
		const float ratioy = float(srch) / float(desth);
		const float srcxoff = float(srcx) + 0.5f * ratiox + 0.5f;
		const float srcyoff = float(srcy) + 0.5f * ratioy + 0.5f;

		for (int y = 0; y < desth; y++)
		{
			for (int x = 0; x < destw; x++)
			{
				const int xmid = int(x * ratiox + srcxoff);
				const int ymid = int(y * ratioy + srcyoff);

				if (xmid >=0 && xmid < _srcw && ymid >=0 && ymid < _srch)
				{
					dest.rgba[(y + desty) * _dstw + x + destx] = src.rgba[ymid * _srcw + xmid];
				}
			}
		}
	}
	else if (sample == 2) // linear
	{
		for (int y = 0; y < desth; y++)
		{
			for (int x = 0; x < destw; x++)
			{
				const float xorg = linear(0.0f, float(destw), float(srcx), float(srcx + srcw), float(x + 0));
				const float xend = linear(0.0f, float(destw), float(srcx), float(srcx + srcw), float(x + 1));
				const float yorg = linear(0.0f, float(desth), float(srcy), float(srcy + srch), float(y + 0));
				const float yend = linear(0.0f, float(desth), float(srcy), float(srcy + srch), float(y + 1));

				dest.rgba[(y + desty) * _dstw + x + destx] = acor((RGBA *)&src.rgba[0], rect2f(vec2f(xorg, yorg), vec2f(xend, yend)));
			}
		}
	}
}
*/

//!! to-do : un-maximized multiple MDI child client windows

void PaintSur::PolygonImageRaw(const std::vector<RenderVertex> &rgv, BYTE *hbm, const float left, const float top, const float right, const float bottom, const int bitmapwidth, const int bitmapheight)
{
	const int ix = SCALEXf(left);
	const int iy = SCALEYf(top);
	const int ix2 = SCALEXf(right);
	const int iy2 = SCALEYf(bottom);
	//BLENDFUNCTION blendf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	//HWND hWnd;
	RECT rect;
	RECT rabs, rclip;

	// check if rect is on the screen otherwise don't waste cycles

	const PinTable * const m_ptable = m_psel->GetPTable();
	const VPinball * const m_pvp = m_ptable->m_pvp;

	GetClientRect(m_pvp->m_hwndWork, &rect);
	// get rect intersection of DC and source bitmap if any
	rabs.top = 0;
	rabs.left = 0;
	rabs.right = rect.right - rect.left;
	rabs.bottom = rect.bottom - rect.top;

	// calc clipping rectangle
	rclip.top = max((LONG)iy, rabs.top);
	rclip.bottom = min((LONG)iy2, rabs.bottom);
	rclip.left = max((LONG)ix, rabs.left);
	rclip.right = min((LONG)ix2, rabs.right);

	const bool bOK = (rclip.top < rclip.bottom && rclip.left < rclip.right);

	if (bOK)
	{
		// create dest stretch bitmap bits
		BYTE * const bmpx = (BYTE *)malloc((sizeof(BYTE) * 4) * (ix2 - ix) * (iy2 - iy));

		if (bmpx != NULL)
		{
			memset(bmpx, 0, (sizeof(BYTE) * 4) * (ix2 - ix) * (iy2 - iy));
			//hWnd = GetDesktopWindow();
			HDC hdcTop = GetDC(NULL);
			HDC hdcA = CreateCompatibleDC(hdcTop);
			HDC hdcB = CreateCompatibleDC(hdcTop);

			HBITMAP hbmpA, hBmpALast;
			HBITMAP hbmpB, hBmpBLast;

			// create a HBITMAP handle with src bits actual
			hbmpA = CreateCompatibleBitmap(hdcTop, bitmapwidth, bitmapheight);
			hBmpALast = (HBITMAP)SelectObject(hdcA, hbmpA);

			BITMAPINFO bmi, bmix, bmiprim;
			memset((void *)&bmi, 0, sizeof(BITMAPINFO));
			bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
			bmi.bmiHeader.biWidth = bitmapwidth;
			bmi.bmiHeader.biHeight = -bitmapheight;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biCompression = BI_RGB;
			bmi.bmiHeader.biSizeImage = (sizeof(BYTE) * 4) * bitmapwidth * bitmapheight;

			SetDIBits(hdcA, hbmpA, 0, bitmapheight, hbm, (BITMAPINFO *)&bmi, DIB_RGB_COLORS);

			// create HBITMAP with dst bits 
			hbmpB = CreateCompatibleBitmap(hdcTop, (ix2 - ix), (iy2 - iy));
			hBmpBLast = (HBITMAP)SelectObject(hdcB, hbmpB);
			// create dest stretch gdi bitmap
			memset((void *)&bmix, 0, sizeof(BITMAPINFO));
			bmix.bmiHeader.biSize = sizeof(BITMAPINFO);
			bmix.bmiHeader.biWidth = (ix2 - ix);
			bmix.bmiHeader.biHeight = (iy2 - iy);
			bmix.bmiHeader.biBitCount = 32;
			bmix.bmiHeader.biPlanes = 1;
			bmix.bmiHeader.biCompression = BI_RGB;
			bmix.bmiHeader.biSizeImage = (sizeof(BYTE) * 4) *  (ix2 - ix) * (iy2 - iy);

			SetDIBits(hdcB, hbmpB, 0, (iy2 - iy), bmpx, (BITMAPINFO *)&bmix, DIB_RGB_COLORS);

			// copy stretched bits from src to dst
			SetStretchBltMode(hdcB, COLORONCOLOR);
			StretchBlt(hdcB, 0, 0, (ix2 - ix), (iy2 - iy), hdcA, 0, 0, bitmapwidth, bitmapheight, SRCCOPY);

			// get the pix data of the primary DC
			HBITMAP hbmpprim = CreateCompatibleBitmap(hdcTop, rabs.right, rabs.bottom);

			BYTE * const bmpprim = (BYTE*)malloc(4 * rabs.right * rabs.bottom);
			memset(bmpprim, 0, 4 * rabs.right * rabs.bottom);
			if (1)
			{
				// now we have what is currently on the primary DC
				memset((void *)&bmiprim, 0, sizeof(BITMAPINFO));
				bmiprim.bmiHeader.biSize = sizeof(BITMAPINFO);
				bmiprim.bmiHeader.biWidth = rabs.right;
				bmiprim.bmiHeader.biHeight = rabs.bottom;
				bmiprim.bmiHeader.biBitCount = 32;
				bmiprim.bmiHeader.biPlanes = 1;
				bmiprim.bmiHeader.biCompression = BI_RGB;
				bmiprim.bmiHeader.biSizeImage = (sizeof(BYTE) * 4) *  rabs.right * rabs.bottom;

				// get the bits of the DC
				GetDIBits(m_hdc, hbmpprim, 0, rabs.bottom, bmpprim, (BITMAPINFO *)&bmiprim, DIB_RGB_COLORS);
				//SelectObject(m_hdc, hobj);

				// get newly stretched bits
				GetDIBits(hdcB, hbmpB, 0, (iy2 - iy), bmpx, (BITMAPINFO *)&bmix, DIB_RGB_COLORS); 
				
				// vflip src bitmap !!!annoying, to-do : only flip clipped area
				// *** it seems you don't need to do this, a negative height in biHeight int BMPinfo works, for how long who knows
				if (0)
				{
				BYTE *const flipix = (BYTE *)malloc(4 * (ix2 - ix) * (iy2 - iy));
				memset(flipix, 0, 4 * (ix2 - ix) * (iy2 - iy));
				const int width = (ix2 - ix);
				const int height = (iy2 - iy);

					for (int i = 0; i < width; ++i)
					{
						for (int j = 0; j < height; ++j)
						{
							flipix[(i + j * width) * 4 + 0] = bmpx[(i + (height - 1 - j) * width) * 4 + 0];
							flipix[(i + j * width) * 4 + 1] = bmpx[(i + (height - 1 - j) * width) * 4 + 1];
							flipix[(i + j * width) * 4 + 2] = bmpx[(i + (height - 1 - j) * width) * 4 + 2];
							flipix[(i + j * width) * 4 + 3] = bmpx[(i + (height - 1 - j) * width) * 4 + 3];
						} // endfor
					} // endfor
					//copy it back
					memcpy(bmpx, flipix, 4 * (ix2 - ix) * (iy2 - iy));
				
				free(flipix);
				} //endif vflip
			} // endif 1 pre-vflip

			// win32 AlphaBlend doesn't work for shit so voila! to hell with you M$ I will do it anyway, it maybe slow but at least mine works!!
			// now this is a little complicated because of top, left, right and bottom edge clipping, of both source and dest bitmap which may be obscured.  
			// result: a copy of primary source bits plus destination bits with clipping and pre-applied alpha, src has already been stretched

			if (0)
			{
				// calc vert clipped height if any
				int clipyrun = (iy2 - iy);
				// calc total height
				clipyrun = clipyrun > rclip.bottom ? rclip.bottom : clipyrun;
				const int pixxoff = iy > rclip.top ? rclip.top : iy;
				const int pixpxoff = rclip.top > ix ? rclip.top : ix;
				
				for (int y = 0; y < clipyrun; ++y)
				{
					// advance to top edge of clipping area(should be out of the loop ugh!)
					//top offset src 
					BYTE * pixflip = (BYTE *)bmpx + (ix2 - ix) * 4 * pixxoff;
					//top offset prim
					BYTE * pixprip = (BYTE *)bmpprim + bmiprim.bmiHeader.biWidth * 4 * pixpxoff;

					// offset to current line in y run
					pixflip = (BYTE *)pixflip + (ix2 - ix) * 4 * y;
					pixprip = (BYTE *)pixprip + bmiprim.bmiHeader.biWidth * 4 * y;
					
					// offset left clipping edge if any
					const int pixxoff = rclip.left > ix ? rclip.left : ix;
					const int pixpxoff = rclip.left > rabs.left ? rclip.left : rabs.left;

					pixflip = (BYTE *)pixflip + pixxoff;
					pixprip = (BYTE *)pixprip + pixpxoff;

					// calc horz clipped stride if any
					int clipxrun = (ix2 - ix);
					clipxrun = clipxrun > rclip.right ? rclip.right : clipxrun;

					// src * alpha + dst = result
					for (int x = 0; x < clipxrun; ++x)
					{
						pixflip[0] = (BYTE)(((float)pixflip[3] * (float)(1.0 / 255.0) * (float)pixflip[0]) + (float)pixprip[0]);
						pixflip[1] = (BYTE)(((float)pixflip[3] * (float)(1.0 / 255.0) * (float)pixflip[1]) + (float)pixprip[1]);
						pixflip[2] = (BYTE)(((float)pixflip[3] * (float)(1.0 / 255.0) * (float)pixflip[2]) + (float)pixprip[2]);
						pixflip += 4;
						pixprip += 4;
					} // end for
				} // end for
			} // end if 1 clipper

			// set it back, but only the clipped visible area
			if (0)
			{
				SelectObject(hdcB, (HGDIOBJ)hBmpBLast);
				DeleteObject(hbmpB);
				hbmpB = CreateCompatibleBitmap(m_hdc, (ix2 - ix), (iy2 - iy));
				hBmpBLast = (HBITMAP)SelectObject(hdcB, hbmpB);
				SetDIBits(hdcB, hbmpB, 0, (iy2 - iy), bmpx, (BITMAPINFO *)&bmix, DIB_RGB_COLORS);

				// calc left/top offsets into source
				const int pixxoff = iy > rclip.top ? rclip.top : iy;
				const int pixyoff = rclip.left > ix ? rclip.left : ix;

				// now simply blit it back to the primary DC in the clip area
				BitBlt(m_hdc, rclip.left, rclip.top, rclip.right, rclip.bottom, hdcB, pixxoff, pixyoff, SRCCOPY);

			//SetDIBitsToDevice(m_hdc, ix, iy, (ix2 - ix), (iy2 - iy), 0, 0, 0, (iy2 - iy), bmpx, &bmix, DIB_RGB_COLORS);
			} // end if 1 blit

			// (please leave in tact for testing purposes) : render memory DC image
			else if (1)
			{
			//SelectObject(hdcB, (HGDIOBJ)hBmpBLast);
			//DeleteObject(hbmpB);
			//hbmpB = CreateCompatibleBitmap(hdcTop, (ix2 - ix), (iy2 - iy));
			//hBmpBLast = (HBITMAP)SelectObject(hdcB, hbmpB);
			//SetDIBits(hdcB, hbmpB, 0, (iy2 - iy), bmpx, (BITMAPINFO *)&bmix, DIB_RGB_COLORS);
			//BitBlt(m_hdc, ix, iy, (ix2 - ix), (iy2 - iy), hdcB, 0, 0, SRCCOPY);

			// now simply blit it back to the primary DC
			SetDIBitsToDevice(m_hdc, ix, iy, (ix2 - ix), (iy2 - iy), 0, 0, 0, (iy2 - iy), bmpx, &bmix, DIB_RGB_COLORS);	
			} // end if test 1

			// **** (please leave in tact for testing purposes) : render primary DC image ****
			else if (0)
			{
			SelectObject(hdcB, (HGDIOBJ)hBmpBLast);
			hBmpBLast = (HBITMAP)SelectObject(hdcB, bmpprim);

			BitBlt(m_hdc, 0, 0, bmiprim.bmiHeader.biWidth, bmiprim.bmiHeader.biHeight, hdcB, 0, 0, SRCCOPY);

			} // end if test 2

			//BOOL bOk =AlphaBlend(m_hdc, ix, iy, ix2 - ix, iy2 - iy, hdcB, 0, 0, (ix2 - ix), (iy2 - iy), blendf);

			// clean up
			SelectObject(hdcA, (HGDIOBJ)hBmpALast);
			SelectObject(hdcB, (HGDIOBJ)hBmpBLast);

			DeleteObject(hbmpA);
			DeleteObject(hbmpB);
			DeleteObject(hbmpprim);

			DeleteDC(hdcA);
			DeleteDC(hdcB);

			ReleaseDC(NULL, hdcTop);

			free(bmpx);
			free(bmpprim);
		} // end if bmpx
	} // end if nx
}

void PaintSur::PolygonImage(const std::vector<RenderVertex> &rgv, HBITMAP hbm, const float left, const float top, const float right, const float bottom, const int bitmapwidth, const int bitmapheight)
{
   const int ix = SCALEXf(left);
   const int iy = SCALEYf(top);
   const int ix2 = SCALEXf(right);
   const int iy2 = SCALEYf(bottom);

   HDC hdcNew = CreateCompatibleDC(m_hdc);
   HBITMAP hbmOld = (HBITMAP)SelectObject(hdcNew, hbm);

   SetStretchBltMode(m_hdc, COLORONCOLOR);
   StretchBlt(m_hdc, ix, iy, ix2 - ix, iy2 - iy, hdcNew, 0, 0, bitmapwidth, bitmapheight, SRCCOPY);

   //all code below "seems" unnecessary
   /*
   SetStretchBltMode(m_hdc, HALFTONE); // somehow enables filtering
   StretchBlt(m_hdc, ix, iy, ix2 - ix, iy2 - iy, hdcNew, 0, 0, bitmapwidth, bitmapheight, SRCINVERT);

   SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));
   SelectObject(m_hdc, GetStockObject(NULL_PEN));

   std::vector<POINT> rgpt(rgv.size());

   for (unsigned i = 0; i < rgv.size(); i++)
   {
      rgpt[i].x = SCALEXf(rgv[i].x);
      rgpt[i].y = SCALEYf(rgv[i].y);
   }

   ::Polygon(m_hdc, rgpt.data(), (int)rgv.size());

   SetStretchBltMode(m_hdc, HALFTONE); // somehow enables filtering
   StretchBlt(m_hdc, ix, iy, ix2 - ix, iy2 - iy, hdcNew, 0, 0, bitmapwidth, bitmapheight, SRCINVERT);
   */

   SelectObject(hdcNew, hbmOld);
   DeleteDC(hdcNew);
}

void PaintSur::Polyline(const Vertex2D * const rgv, const int count)
{
   SelectObject(m_hdc, m_hpnLine);

   /*
    * There seems to be a known GDI bug where drawing very large polylines in one
    * call freezes the system shortly, so we batch them into groups of MAX_SUR_PT_CACHE.
    */
   //m_ptCache.resize(min(count,MAX_SUR_PT_CACHE+1));

   for (int i = 0; i < count; i += MAX_SUR_PT_CACHE)
   {
      const int batchSize = std::min(count - i, MAX_SUR_PT_CACHE + 1);

      for (int i2 = 0; i2 < batchSize; i2++)
      {
         m_ptCache[i2].x = SCALEXf(rgv[i + i2].x);
         m_ptCache[i2].y = SCALEYf(rgv[i + i2].y);
      }

      ::Polyline(m_hdc, m_ptCache, batchSize);
   }
}

void PaintSur::Lines(const Vertex2D * const rgv, const int count)
{
   SelectObject(m_hdc, m_hpnLine);

   /*
    * There seems to be a known GDI bug where drawing very large polylines in one
    * call freezes the system shortly, so we batch them into groups of MAX_SUR_PT_CACHE.
    */
   //m_ptCache.resize(min(count,MAX_SUR_PT_CACHE)*2);
   //std::vector<DWORD> m_ptCache_idx(min(count,MAX_SUR_PT_CACHE),2);

   for (int i = 0; i < count; i += MAX_SUR_PT_CACHE)
   {
      const int batchSize = std::min(count - i, MAX_SUR_PT_CACHE);

      for (int i2 = 0; i2 < batchSize * 2; i2++)
      {
         m_ptCache[i2].x = SCALEXf(rgv[i * 2 + i2].x);
         m_ptCache[i2].y = SCALEYf(rgv[i * 2 + i2].y);
      }

      ::PolyPolyline(m_hdc, m_ptCache, m_ptCache_idx.data(), batchSize);
   }
}

void PaintSur::Arc(const float x, const float y, const float radius, const float pt1x, const float pt1y, const float pt2x, const float pt2y)
{
   const int ix = SCALEXf(x);
   const int iy = SCALEYf(y);
   const int ir = SCALEDf(radius);

   const int x1 = SCALEXf(pt1x);
   const int y1 = SCALEYf(pt1y);
   const int x2 = SCALEXf(pt2x);
   const int y2 = SCALEYf(pt2y);

   SelectObject(m_hdc, m_hpnLine);

   ::Arc(m_hdc, ix - ir, iy - ir, ix + ir, iy + ir, x1, y1, x2, y2);
}

void PaintSur::Image(const float x, const float y, const float x2, const float y2, HDC hdcSrc, const int width, const int height)
{
   const int ix = SCALEXf(x);
   const int iy = SCALEYf(y);
   const int ix2 = SCALEXf(x2);
   const int iy2 = SCALEYf(y2);

   SetStretchBltMode(m_hdc, HALFTONE); // somehow enables filtering
   StretchBlt(m_hdc, ix, iy, ix2 - ix, iy2 - iy, hdcSrc, 0, 0, width, height, SRCCOPY);
}

void PaintSur::SetObject(ISelect *psel)
{
   if ((m_psel != NULL) && (psel != NULL)) // m_psel can be null when rendering a blueprint or other item which has no selection feedback
   {
      if (psel->m_selectstate == eSelected)
      {
         psel->SetSelectFormat(this);
      }
      else if (psel->m_selectstate == eMultiSelected)
      {
         psel->SetMultiSelectFormat(this);
      }
      else if (psel->m_fLocked)
      {
         psel->SetLockedFormat(this);
      }
   }
}

void PaintSur::SetFillColor(const int rgb)
{
   SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));
   DeleteObject(m_hbr);

   if (rgb == -1)
   {
      LOGBRUSH lbr;
      lbr.lbStyle = BS_NULL;
      m_hbr = CreateBrushIndirect(&lbr);
   }
   else
   {
      m_hbr = CreateSolidBrush(rgb);
   }
}

void PaintSur::SetBorderColor(const int rgb, const bool fDashed, const int width)
{
   SelectObject(m_hdc, GetStockObject(BLACK_PEN));
   DeleteObject(m_hpnOutline);

   if (rgb == -1)
   {
      m_hpnOutline = CreatePen(PS_NULL, width, rgb);
      m_fNullBorder = true;
   }
   else
   {
      const int style = fDashed ? PS_DOT : PS_SOLID;
      m_hpnOutline = CreatePen(style, width, rgb); //!! claims to be leaking mem
      m_fNullBorder = false;
   }
}

void PaintSur::SetLineColor(const int rgb, const bool fDashed, const int width)
{
   SelectObject(m_hdc, GetStockObject(BLACK_PEN));
   DeleteObject(m_hpnLine);

   SetBkMode(m_hdc, TRANSPARENT);

   if (rgb == -1)
   {
      m_hpnLine = CreatePen(PS_NULL, width, rgb);
   }
   else
   {
      const int style = fDashed ? PS_DOT : PS_SOLID;
      m_hpnLine = CreatePen(style, width, rgb);
   }
}
