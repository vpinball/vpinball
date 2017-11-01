// Decal.cpp: implementation of the Decal class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 

#define AUTOLEADING (tm.tmAscent - tm.tmInternalLeading/4)

Decal::Decal()
{
   m_pIFont = NULL;
   vertexBuffer = NULL;
   m_textImg = NULL;
   memset(m_d.m_szImage, 0, MAXTOKEN);
   memset(m_d.m_szMaterial, 0, 32);
   memset(m_d.m_szSurface, 0, MAXTOKEN);
   m_ptable = NULL;
   m_leading = 0.0f;
   m_descent = 0.0f;
   m_realheight = 0.0f;
   m_realwidth = 0.0f;
}

Decal::~Decal()
{
   m_pIFont->Release();
   if (m_textImg)
      delete m_textImg;
   if (vertexBuffer)
   {
      vertexBuffer->release();
      vertexBuffer = 0;
   }
}

HRESULT Decal::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;

   SetDefaults(fromMouseClick);

   InitVBA(fTrue, 0, NULL);

   EnsureSize();

   return S_OK;
}

void Decal::SetDefaults(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;
   int iTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Decal", "Width", &fTmp);
   m_d.m_width = (hr == S_OK) && fromMouseClick ? fTmp : 100.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Decal", "Height", &fTmp);
   m_d.m_height = (hr == S_OK) && fromMouseClick ? fTmp : 100.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Decal", "Rotation", &fTmp);
   m_d.m_rotation = (hr == S_OK) && fromMouseClick ? fTmp : 0;

   hr = GetRegString("DefaultProps\\Decal", "Image", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage[0] = 0;
   hr = GetRegString("DefaultProps\\Decal", "Surface", m_d.m_szSurface, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface[0] = 0;

   hr = GetRegInt("DefaultProps\\Decal", "DecalType", &iTmp);
   m_d.m_decaltype = (hr == S_OK) && fromMouseClick ? (enum DecalType)iTmp : DecalImage;

   hr = GetRegString("DefaultProps\\Decal", "Text", m_d.m_sztext, MAXSTRING);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_sztext[0] = '\0';

   hr = GetRegInt("DefaultProps\\Decal", "Sizing", &iTmp);
   m_d.m_sizingtype = (hr == S_OK) && fromMouseClick ? (enum SizingType)iTmp : ManualSize;

   hr = GetRegInt("DefaultProps\\Decal", "Color", &iTmp);
   m_d.m_color = (hr == S_OK) && fromMouseClick ? iTmp : RGB(0, 0, 0);

   hr = GetRegInt("DefaultProps\\Decal", "VerticalText", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fVerticalText = iTmp == 0 ? false : true;
   else
      m_d.m_fVerticalText = false;

   if (!m_pIFont)
   {
      FONTDESC fd;
      fd.cbSizeofstruct = sizeof(FONTDESC);

      hr = GetRegStringAsFloat("DefaultProps\\Decal", "FontSize", &fTmp);
      fd.cySize.int64 = (hr == S_OK) && fromMouseClick ? (LONGLONG)(fTmp * 10000.0) : 142500;

      char tmp[256];
      hr = GetRegString("DefaultProps\\Decal", "FontName", tmp, 256);
      if ((hr != S_OK) || !fromMouseClick)
         fd.lpstrName = L"Arial Black";
      else
      {
         int len = lstrlen(tmp) + 1;
         fd.lpstrName = (LPOLESTR)malloc(len*sizeof(WCHAR));
         memset(fd.lpstrName, 0, len*sizeof(WCHAR));
         UNICODE_FROM_ANSI(fd.lpstrName, tmp, len);
      }

      hr = GetRegInt("DefaultProps\\Decal", "FontWeight", &iTmp);
      fd.sWeight = (hr == S_OK) && fromMouseClick ? iTmp : FW_NORMAL;

      hr = GetRegInt("DefaultProps\\Decal", "FontCharSet", &iTmp);
      fd.sCharset = (hr == S_OK) && fromMouseClick ? iTmp : 0;

      hr = GetRegInt("DefaultProps\\Decal", "FontItalic", &iTmp);
      if ((hr == S_OK) && fromMouseClick)
         fd.fItalic = iTmp == 0 ? false : true;
      else
         fd.fItalic = 0;

      hr = GetRegInt("DefaultProps\\Decal", "FontUnderline", &iTmp);
      if ((hr == S_OK) && fromMouseClick)
         fd.fUnderline = iTmp == 0 ? false : true;
      else
         fd.fUnderline = 0;

      hr = GetRegInt("DefaultProps\\Decal", "FontStrikeThrough", &iTmp);
      if ((hr == S_OK) && fromMouseClick)
         fd.fStrikethrough = iTmp == 0 ? false : true;
      else
         fd.fStrikethrough = 0;

      OleCreateFontIndirect(&fd, IID_IFont, (void **)&m_pIFont);
   }
}

void Decal::WriteRegDefaults()
{
   SetRegValueFloat("DefaultProps\\Decal", "Width", m_d.m_width);
   SetRegValueFloat("DefaultProps\\Decal", "Height", m_d.m_height);
   SetRegValueFloat("DefaultProps\\Decal", "Rotation", m_d.m_rotation);
   SetRegValue("DefaultProps\\Decal", "Image", REG_SZ, &m_d.m_szImage, lstrlen(m_d.m_szImage));
   SetRegValue("DefaultProps\\Decal", "DecalType", REG_DWORD, &m_d.m_decaltype, 4);
   SetRegValue("DefaultProps\\Decal", "Text", REG_SZ, &m_d.m_sztext, lstrlen(m_d.m_sztext));
   SetRegValue("DefaultProps\\Decal", "Sizing", REG_DWORD, &m_d.m_sizingtype, 4);
   SetRegValue("DefaultProps\\Decal", "Color", REG_DWORD, &m_d.m_color, 4);
   SetRegValueBool("DefaultProps\\Decal", "VerticalText", m_d.m_fVerticalText);
   SetRegValue("DefaultProps\\Decal", "Surface", REG_SZ, m_d.m_szSurface, lstrlen(m_d.m_szSurface));

   if (m_pIFont)
   {
      FONTDESC fd;
      fd.cbSizeofstruct = sizeof(FONTDESC);
      m_pIFont->get_Size(&fd.cySize);
      m_pIFont->get_Name(&fd.lpstrName);
      m_pIFont->get_Weight(&fd.sWeight);
      m_pIFont->get_Charset(&fd.sCharset);
      m_pIFont->get_Italic(&fd.fItalic);
      m_pIFont->get_Underline(&fd.fUnderline);
      m_pIFont->get_Strikethrough(&fd.fStrikethrough);

      const float fTmp = (float)(fd.cySize.int64 / 10000.0);
      SetRegValueFloat("DefaultProps\\Decal", "FontSize", fTmp);

      size_t charCnt = wcslen(fd.lpstrName) + 1;
      char strTmp[MAXTOKEN];
      WideCharToMultiByte(CP_ACP, 0, fd.lpstrName, (int)charCnt, strTmp, (int)(2 * charCnt), NULL, NULL);
      SetRegValue("DefaultProps\\Decal", "FontName", REG_SZ, &strTmp, lstrlen(strTmp));
      const int weight = fd.sWeight;
      const int charset = fd.sCharset;
      SetRegValueInt("DefaultProps\\Decal", "FontWeight", weight);
      SetRegValueInt("DefaultProps\\Decal", "FontCharSet", charset);
      SetRegValue("DefaultProps\\Decal", "FontItalic", REG_DWORD, &fd.fItalic, 4);
      SetRegValue("DefaultProps\\Decal", "FontUnderline", REG_DWORD, &fd.fUnderline, 4);
      SetRegValue("DefaultProps\\Decal", "FontStrikeThrough", REG_DWORD, &fd.fStrikethrough, 4);
   }
}


void Decal::PreRender(Sur * const psur)
{
   if (!m_fBackglass || GetPTable()->GetDecalsEnabled())
   {
      psur->SetBorderColor(-1, false, 0);
      psur->SetFillColor(m_ptable->RenderSolid() ? RGB(0, 0, 255) : -1);
      psur->SetObject(this);

      const float halfwidth = m_realwidth/*m_d.m_width*/ * 0.5f;
      const float halfheight = m_realheight/*m_d.m_height*/ * 0.5f;

      const float radangle = ANGTORAD(m_d.m_rotation);
      const float sn = sinf(radangle);
      const float cs = cosf(radangle);

      const Vertex2D rgv[4] = {
         Vertex2D(m_d.m_vCenter.x + sn*halfheight - cs*halfwidth,
         m_d.m_vCenter.y - cs*halfheight - sn*halfwidth),

         Vertex2D(m_d.m_vCenter.x + sn*halfheight + cs*halfwidth,
         m_d.m_vCenter.y - cs*halfheight + sn*halfwidth),

         Vertex2D(m_d.m_vCenter.x - sn*halfheight + cs*halfwidth,
         m_d.m_vCenter.y + cs*halfheight + sn*halfwidth),

         Vertex2D(m_d.m_vCenter.x - sn*halfheight - cs*halfwidth,
         m_d.m_vCenter.y + cs*halfheight - sn*halfwidth) };

      psur->Polygon(rgv, 4);
   }
}

void Decal::Render(Sur * const psur)
{
   if (!m_fBackglass || GetPTable()->GetDecalsEnabled())
   {
      psur->SetBorderColor(RGB(0, 0, 0), false, 0);
      psur->SetFillColor(-1);
      psur->SetObject(this);
      psur->SetObject(NULL);

      const float halfwidth = m_realwidth * 0.5f;
      const float halfheight = m_realheight * 0.5f;

      const float radangle = ANGTORAD(m_d.m_rotation);
      const float sn = sinf(radangle);
      const float cs = cosf(radangle);

      const Vertex2D rgv[4] = {
         Vertex2D(m_d.m_vCenter.x + sn*halfheight - cs*halfwidth,
         m_d.m_vCenter.y - cs*halfheight - sn*halfwidth),

         Vertex2D(m_d.m_vCenter.x + sn*halfheight + cs*halfwidth,
         m_d.m_vCenter.y - cs*halfheight + sn*halfwidth),

         Vertex2D(m_d.m_vCenter.x - sn*halfheight + cs*halfwidth,
         m_d.m_vCenter.y + cs*halfheight + sn*halfwidth),

         Vertex2D(m_d.m_vCenter.x - sn*halfheight - cs*halfwidth,
         m_d.m_vCenter.y + cs*halfheight - sn*halfwidth) };

      psur->Polygon(rgv, 4);
   }
}

void Decal::GetTimers(Vector<HitTimer> * const pvht)
{
   IEditable::BeginPlay();
}

void Decal::GetTextSize(int * const px, int * const py)
{
   const int len = lstrlen(m_d.m_sztext);
   HFONT hFont = GetFont();
   const int alignment = DT_LEFT;

   HDC hdcNull = GetDC(NULL);
   HFONT hFontOld = (HFONT)SelectObject(hdcNull, hFont);

   TEXTMETRIC tm;
   GetTextMetrics(hdcNull, &tm);

   if (m_d.m_fVerticalText)
   {
      // Do huge amounts of work to get rid of the descent and internal ascent of the font, because it leaves ugly spaces
      *py = AUTOLEADING * len;
      *px = 0;
      for (int i = 0; i < len; i++)
      {
         RECT rcOut;
         rcOut.left = 0;
         rcOut.top = 0;		//-tm.tmInternalLeading + 2; // Leave a pixel for anti-aliasing;
         rcOut.right = 0x1;
         rcOut.bottom = 0x1;
         DrawText(hdcNull, &m_d.m_sztext[i], 1, &rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT);

         *px = max(*px, rcOut.right);
      }
   }
   else
   {
      *py = tm.tmAscent;

      RECT rcOut;
      rcOut.left = 0;
      rcOut.top = 0;			//-tm.tmInternalLeading + 2; // Leave a pixel for anti-aliasing;
      rcOut.right = 0x1;
      rcOut.bottom = 0x1;
      DrawText(hdcNull, m_d.m_sztext, len, &rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT);

      *px = rcOut.right;
   }

   SelectObject(hdcNull, hFontOld);
   ReleaseDC(NULL, hdcNull);

   DeleteObject(hFont);
}

void Decal::RenderText()
{
   if (m_d.m_decaltype != DecalText)
      return;

   RECT rcOut = { 0 };
   const int len = lstrlen(m_d.m_sztext);
   HFONT hFont, hFontOld;
   hFont = GetFont();
   int alignment = DT_LEFT;

   HDC hdcNull;
   hdcNull = GetDC(NULL);
   hFontOld = (HFONT)SelectObject(hdcNull, hFont);

   TEXTMETRIC tm;
   GetTextMetrics(hdcNull, &tm);

   float charheight;
   if (m_d.m_fVerticalText)
   {
      int maxwidth = 0;

      for (int i = 0; i < len; i++)
      {
         rcOut.left = 0;
         rcOut.top = 0;//-tm.tmInternalLeading + 2; // Leave a pixel for anti-aliasing;
         rcOut.right = 1;
         rcOut.bottom = 1;
         DrawText(hdcNull, &m_d.m_sztext[i], 1, &rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT);
         maxwidth = max(maxwidth, rcOut.right);
      }

      rcOut.bottom += AUTOLEADING * (len - 1);
      rcOut.right = maxwidth;

      charheight = m_realheight / (float)len;
   }
   else
   {
      rcOut.left = 0;
      rcOut.top = 0;//-tm.tmInternalLeading + 2; // Leave a pixel for anti-aliasing;
      rcOut.right = 1;
      rcOut.bottom = 1;
      DrawText(hdcNull, m_d.m_sztext, len, &rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT);

      charheight = m_realheight;
   }

   SelectObject(hdcNull, hFontOld);
   ReleaseDC(NULL, hdcNull);

   // Calculate the percentage of the texture which is for oomlats and commas.
   const float invascent = charheight / (float)tm.tmAscent;
   m_leading = (float)tm.tmInternalLeading * invascent /*m_d.m_height*/;
   m_descent = (float)tm.tmDescent * invascent;

   m_textImg = new BaseTexture(rcOut.right, rcOut.bottom);

   if (m_d.m_color == RGB(255, 255, 255))
      m_d.m_color = RGB(254, 255, 255); //m_pinimage.SetTransparentColor(RGB(0,0,0));
   else if (m_d.m_color == RGB(0, 0, 0))
      m_d.m_color = RGB(0, 0, 1);

   BITMAPINFO bmi;
   ZeroMemory(&bmi, sizeof(bmi));
   bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth = m_textImg->width();
   bmi.bmiHeader.biHeight = -m_textImg->height();
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biBitCount = 32;
   bmi.bmiHeader.biCompression = BI_RGB;
   bmi.bmiHeader.biSizeImage = 0;

   void *bits;
   HBITMAP hbm = CreateDIBSection(0, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
   assert(hbm);

   HDC hdc = CreateCompatibleDC(NULL);
   HBITMAP oldBmp = (HBITMAP)SelectObject(hdc, hbm);

   //m_pinimage.GetTextureDC(&hdc);
   /*if (m_d.m_color == RGB(255,255,255))
     {
     SelectObject(hdc, GetStockObject(BLACK_BRUSH));
     }
     else*/
   {
      SelectObject(hdc, GetStockObject(WHITE_BRUSH));
   }

   PatBlt(hdc, 0, 0, rcOut.right, rcOut.bottom, PATCOPY);
   hFontOld = (HFONT)SelectObject(hdc, hFont);

   SetTextColor(hdc, m_d.m_color);
   SetBkMode(hdc, TRANSPARENT);
   SetTextAlign(hdc, TA_LEFT | TA_TOP | TA_NOUPDATECP);
   alignment = DT_CENTER;

   if (m_d.m_fVerticalText)
   {
      for (int i = 0; i < len; i++)
      {
         rcOut.top = AUTOLEADING*i;//-tm.tmInternalLeading + 2; // Leave a pixel for anti-aliasing;
         rcOut.bottom = rcOut.top + 100;
         DrawText(hdc, &m_d.m_sztext[i], 1, &rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK);
      }
   }
   else
      DrawText(hdc, m_d.m_sztext, len, &rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK);

   m_textImg->CopyFrom_Raw(bits);
   Texture::SetOpaque(m_textImg);

   SelectObject(hdc, hFontOld);
   SelectObject(hdc, oldBmp);
   DeleteDC(hdc);
   DeleteObject(hFont);
   DeleteObject(hbm);
}

void Decal::GetHitShapes(Vector<HitObject> * const pvho)
{
}

void Decal::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Decal::EndPlay()
{
   if (m_textImg)
   {
      delete m_textImg;
      m_textImg = 0;
   }

   if (vertexBuffer)
   {
      vertexBuffer->release();
      vertexBuffer = 0;
   }
}

void Decal::PostRenderStatic(RenderDevice* pd3dDevice)
{
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   if (!m_fBackglass //!! should just check if material has opacity enabled, but this is crucial for HV setup performance like-is
      && mat && mat->m_bOpacityActive)
      RenderObject(pd3dDevice);
}

static const WORD rgi0123[4] = { 0, 1, 2, 3 };

void Decal::RenderSetup(RenderDevice* pd3dDevice)
{
   RenderText();

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y) * m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

   float leading, descent; // For fonts
   if (m_d.m_decaltype != DecalImage)
   {
      leading = m_leading;
      descent = m_descent;
   }
   else
   {
      leading = 0.f;
      descent = 0.f;
   }

   const float halfwidth = m_realwidth * 0.5f;
   const float halfheight = m_realheight * 0.5f;

   const float radangle = ANGTORAD(m_d.m_rotation);
   const float sn = sinf(radangle);
   const float cs = cosf(radangle);

   Vertex3D_NoTex2 vertices[4];
   vertices[0].x = m_d.m_vCenter.x + sn*(halfheight + leading) - cs*halfwidth;
   vertices[0].y = m_d.m_vCenter.y - cs*(halfheight + leading) - sn*halfwidth;

   vertices[1].x = m_d.m_vCenter.x + sn*(halfheight + leading) + cs*halfwidth;
   vertices[1].y = m_d.m_vCenter.y - cs*(halfheight + leading) + sn*halfwidth;

   vertices[2].x = m_d.m_vCenter.x - sn*(halfheight + descent) + cs*halfwidth;
   vertices[2].y = m_d.m_vCenter.y + cs*(halfheight + descent) + sn*halfwidth;

   vertices[3].x = m_d.m_vCenter.x - sn*(halfheight + descent) - cs*halfwidth;
   vertices[3].y = m_d.m_vCenter.y + cs*(halfheight + descent) - sn*halfwidth;

   for (int l = 0; l < 4; l++)
      vertices[l].z = height + 0.2f;

   if (!m_fBackglass)
      SetNormal(vertices, rgi0123, 4);
   else
      SetHUDVertices(vertices, 4);

   vertices[0].tu = 0;
   vertices[0].tv = 0;
   vertices[1].tu = 1.0f;
   vertices[1].tv = 0;
   vertices[2].tu = 1.0f;
   vertices[2].tv = 1.0f;
   vertices[3].tu = 0;
   vertices[3].tv = 1.0f;

   if (vertexBuffer)
      vertexBuffer->release();
   const DWORD vertexType = (m_fBackglass && GetPTable()->GetDecalsEnabled()) ? MY_D3DTRANSFORMED_NOTEX2_VERTEX : MY_D3DFVF_NOTEX2_VERTEX;
   pd3dDevice->CreateVertexBuffer(4, 0, vertexType, &vertexBuffer);

   Vertex3D_NoTex2 *buf;
   vertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   memcpy(buf, vertices, 4 * sizeof(Vertex3D_NoTex2));
   vertexBuffer->unlock();
}

float Decal::GetDepth(const Vertex3Ds& viewDir)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y) * m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
   return !m_fBackglass ? (viewDir.x * m_d.m_vCenter.x + viewDir.y * m_d.m_vCenter.y + viewDir.z*height) : 0.f;
}

bool Decal::IsTransparent()
{
   if (m_fBackglass)
      return false;
   else
      return true;
}

void Decal::RenderObject(RenderDevice* pd3dDevice)
{
   if (m_fBackglass && !GetPTable()->GetDecalsEnabled())
      return;

   if (m_fBackglass && (g_pplayer->m_ptable->m_tblMirrorEnabled^g_pplayer->m_ptable->m_fReflectionEnabled))
      pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
   else
      pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   pd3dDevice->basicShader->SetMaterial(mat);

   //pd3dDevice->basicShader->SetFloat("fmaterialAlpha",1.0f);

   if (m_d.m_decaltype != DecalImage)
   {
      pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_isMetal" : "basic_with_texture_isNotMetal");
      pd3dDevice->basicShader->SetTexture("Texture0", pd3dDevice->m_texMan.LoadTexture(m_textImg));
      pd3dDevice->basicShader->SetAlphaTestValue(-1.0f);
   }
   else
   {
      Texture *pin = m_ptable->GetImage(m_d.m_szImage);
      if (pin)
      {
         pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_isMetal" : "basic_with_texture_isNotMetal");
         pd3dDevice->basicShader->SetTexture("Texture0", pin);
         pd3dDevice->basicShader->SetAlphaTestValue(pin->m_alphaTestValue * (float)(1.0 / 255.0));
      }
      else
         pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");
   }

   // Set texture to mirror, so the alpha state of the texture blends correctly to the outside
   //!!   pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_MIRROR);

   //ppin3d->SetTextureFilter ( 0, TEXTURE_MODE_TRILINEAR );
   g_pplayer->m_pin3d.EnableAlphaBlend(false);

   if (!m_fBackglass)
   {
      const float depthbias = -5.f * BASEDEPTHBIAS;
      pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, *((DWORD*)&depthbias));
   }
   else
   {
      pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
      const D3DXVECTOR4 staticColor(1.0f, 1.0f, 1.0f, 1.0f);
      pd3dDevice->basicShader->SetVector("cBase_Alpha", &staticColor);
   }

   pd3dDevice->basicShader->Begin(0);
   pd3dDevice->DrawPrimitiveVB(D3DPT_TRIANGLEFAN, (m_fBackglass && GetPTable()->GetDecalsEnabled()) ? MY_D3DTRANSFORMED_NOTEX2_VERTEX : MY_D3DFVF_NOTEX2_VERTEX, vertexBuffer, 0, 4);
   pd3dDevice->basicShader->End();

   // Set the render state.
   //pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_WRAP);
   //g_pplayer->m_pin3d.DisableAlphaBlend(); //!! not necessary anymore

   //if(m_fBackglass && (g_pplayer->m_ptable->m_tblMirrorEnabled^g_pplayer->m_ptable->m_fReflectionEnabled))
   //   pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
}

void Decal::RenderStatic(RenderDevice* pd3dDevice)
{
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   if (m_fBackglass //!! should just check if material has no opacity enabled, but this is crucial for HV setup performance like-is
      || !mat || !mat->m_bOpacityActive)
      RenderObject(pd3dDevice);
}

void Decal::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Decal::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;

   m_ptable->SetDirtyDraw();
}

void Decal::GetCenter(Vertex2D * const pv) const
{
   *pv = m_d.m_vCenter;
}

void Decal::PutCenter(const Vertex2D * const pv)
{
   m_d.m_vCenter = *pv;

   m_ptable->SetDirtyDraw();
}

void Decal::Rotate(float ang, Vertex2D *pvCenter, const bool useElementCenter)
{
   ISelect::Rotate(ang, pvCenter, useElementCenter);

   m_d.m_rotation += ang;
}

HRESULT Decal::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex2D));
   bw.WriteFloat(FID(WDTH), m_d.m_width);
   bw.WriteFloat(FID(HIGH), m_d.m_height);
   bw.WriteFloat(FID(ROTA), m_d.m_rotation);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteString(FID(TEXT), m_d.m_sztext);
   bw.WriteInt(FID(TYPE), m_d.m_decaltype);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteInt(FID(COLR), m_d.m_color);
   bw.WriteInt(FID(SIZE), m_d.m_sizingtype);

   bw.WriteBool(FID(VERT), m_d.m_fVerticalText);

   bw.WriteBool(FID(BGLS), m_fBackglass);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

   bw.WriteTag(FID(FONT));
   IPersistStream * ips;
   m_pIFont->QueryInterface(IID_IPersistStream, (void **)&ips);
   ips->Save(pstm, TRUE);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Decal::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

BOOL Decal::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(VCEN))
   {
      pbr->GetStruct(&m_d.m_vCenter, sizeof(Vertex2D));
   }
   else if (id == FID(WDTH))
   {
      pbr->GetFloat(&m_d.m_width);
   }
   else if (id == FID(HIGH))
   {
      pbr->GetFloat(&m_d.m_height);
   }
   else if (id == FID(ROTA))
   {
      pbr->GetFloat(&m_d.m_rotation);
   }
   else if (id == FID(IMAG))
   {
      pbr->GetString(m_d.m_szImage);
   }
   else if (id == FID(SURF))
   {
      pbr->GetString(m_d.m_szSurface);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(TEXT))
   {
      pbr->GetString(m_d.m_sztext);
   }
   else if (id == FID(TYPE))
   {
      pbr->GetInt(&m_d.m_decaltype);
   }
   else if (id == FID(COLR))
   {
      pbr->GetInt(&m_d.m_color);
   }
   else if (id == FID(MATR))
   {
      pbr->GetString(m_d.m_szMaterial);
   }
   else if (id == FID(SIZE))
   {
      pbr->GetInt(&m_d.m_sizingtype);
   }
   else if (id == FID(VERT))
   {
      pbr->GetBool(&m_d.m_fVerticalText);
   }
   else if (id == FID(BGLS))
   {
      pbr->GetBool(&m_fBackglass);
   }
   else if (id == FID(FONT))
   {
      if (!m_pIFont)
      {
         FONTDESC fd;
         fd.cbSizeofstruct = sizeof(FONTDESC);
         fd.lpstrName = L"Arial";
         fd.cySize.int64 = 142500;
         fd.sWeight = FW_NORMAL;
         fd.sCharset = 0;
         fd.fItalic = 0;
         fd.fUnderline = 0;
         fd.fStrikethrough = 0;
         OleCreateFontIndirect(&fd, IID_IFont, (void **)&m_pIFont);
      }

      IPersistStream * ips;
      m_pIFont->QueryInterface(IID_IPersistStream, (void **)&ips);

      ips->Load(pbr->m_pistream);
   }
   else
   {
      ISelect::LoadToken(id, pbr);
   }

   return fTrue;
}

HRESULT Decal::InitPostLoad()
{
   EnsureSize();

   return S_OK;
}

void Decal::EnsureSize()
{
   if (((m_d.m_sizingtype != AutoSize) ||
      (m_d.m_decaltype == DecalImage)) && (m_d.m_sizingtype != AutoWidth) ||
      (m_d.m_decaltype == DecalText && (m_d.m_sztext[0] == '\0')))
   {
      m_realwidth = m_d.m_width;
      m_realheight = m_d.m_height;
   }
   else if ((m_d.m_sizingtype == AutoSize) && (m_d.m_decaltype != DecalImage))
   {
      // ignore the auto aspect flag
      int sizex, sizey;
      GetTextSize(&sizex, &sizey);

      CY cy;
      m_pIFont->get_Size(&cy);

      m_realheight = (float)cy.Lo * (float)(1.0 / 2545.0);

      if (m_d.m_fVerticalText)
         m_realheight *= lstrlen(m_d.m_sztext);

      m_realwidth = m_realheight * (float)sizex / (float)sizey;
   }
   else // Auto aspect
   {
      m_realheight = m_d.m_height;

      if (m_d.m_decaltype == DecalImage)
      {
         Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
         m_realwidth = m_realheight;
         if (pin)
         {
            m_realwidth *= (float)pin->m_width / (float)pin->m_height;
         }
      }
      else
      {
         CY cy;
         m_pIFont->get_Size(&cy);

         int sizex, sizey;
         GetTextSize(&sizex, &sizey);

         m_realheight = (float)cy.Lo * (float)(1.0 / 2545.0);

         if (m_d.m_fVerticalText)
         {
            m_realheight *= lstrlen(m_d.m_sztext);
            m_realwidth = m_d.m_width;
         }
         else
         {
            m_realwidth = m_realheight * (float)sizex / (float)sizey;
            m_realheight = m_d.m_height;
         }
      }
   }
}

HFONT Decal::GetFont()
{
   LOGFONT lf;
   ZeroMemory(&lf, sizeof(lf));

   lf.lfHeight = -72;
   lf.lfCharSet = DEFAULT_CHARSET;
   lf.lfQuality = NONANTIALIASED_QUALITY;

   CComBSTR bstr;
   HRESULT hr = m_pIFont->get_Name(&bstr);

   WideCharToMultiByte(CP_ACP, 0, bstr, -1, lf.lfFaceName, LF_FACESIZE, NULL, NULL);

   BOOL bl;
   hr = m_pIFont->get_Bold(&bl);

   lf.lfWeight = bl ? FW_BOLD : FW_NORMAL;

   hr = m_pIFont->get_Italic(&bl);

   lf.lfItalic = (BYTE)bl;

   HFONT hFont = CreateFontIndirect(&lf);

   return hFont;
}

STDMETHODIMP Decal::get_Rotation(float *pVal)
{
   *pVal = m_d.m_rotation;

   return S_OK;
}

STDMETHODIMP Decal::put_Rotation(float newVal)
{
   STARTUNDO

   m_d.m_rotation = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Decal::get_Image(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Decal::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, szImage, 32, NULL, NULL);
   const Texture * const tex = m_ptable->GetImage(szImage);
   if(tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }

   STARTUNDO

   strcpy_s(m_d.m_szImage,szImage);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Decal::get_Width(float *pVal)
{
   *pVal = m_d.m_width;

   return S_OK;
}

STDMETHODIMP Decal::put_Width(float newVal)
{
   STARTUNDO

   m_d.m_width = newVal;

   EnsureSize();

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Decal::get_Height(float *pVal)
{
   *pVal = m_d.m_height;

   return S_OK;
}

STDMETHODIMP Decal::put_Height(float newVal)
{
   STARTUNDO

   m_d.m_height = newVal;

   EnsureSize();

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Decal::get_X(float *pVal)
{
   *pVal = m_d.m_vCenter.x;

   return S_OK;
}

STDMETHODIMP Decal::put_X(float newVal)
{
   STARTUNDO

   m_d.m_vCenter.x = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Decal::get_Y(float *pVal)
{
   *pVal = m_d.m_vCenter.y;

   return S_OK;
}

STDMETHODIMP Decal::put_Y(float newVal)
{
   STARTUNDO

   m_d.m_vCenter.y = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Decal::get_Surface(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Decal::put_Surface(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

void Decal::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROPDECAL_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPDECAL_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);
}

STDMETHODIMP Decal::get_Type(DecalType *pVal)
{
   *pVal = m_d.m_decaltype;

   return S_OK;
}

STDMETHODIMP Decal::put_Type(DecalType newVal)
{
   STARTUNDO

   m_d.m_decaltype = newVal;

   EnsureSize();

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Decal::get_Text(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, (char *)m_d.m_sztext, -1, wz, 512);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Decal::put_Text(BSTR newVal)
{
   if (lstrlenW(newVal) < 512)
   {
      STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_sztext, 512, NULL, NULL);

      EnsureSize();

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Decal::get_SizingType(SizingType *pVal)
{
   *pVal = m_d.m_sizingtype;

   return S_OK;
}

STDMETHODIMP Decal::put_SizingType(SizingType newVal)
{
   STARTUNDO

   m_d.m_sizingtype = newVal;

   EnsureSize();

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Decal::get_FontColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_color;

   return S_OK;
}

STDMETHODIMP Decal::put_FontColor(OLE_COLOR newVal)
{
   STARTUNDO

   m_d.m_color = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Decal::get_Material(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Decal::put_Material(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szMaterial, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Decal::get_Font(IFontDisp **pVal)
{
   m_pIFont->QueryInterface(IID_IFontDisp, (void **)pVal);

   return S_OK;
}

STDMETHODIMP Decal::putref_Font(IFontDisp *pFont)
{
   //We know that our own property browser gives us the same pointer

   SetDirtyDraw();

   EnsureSize();

   return S_OK;
}

STDMETHODIMP Decal::get_HasVerticalText(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fVerticalText);

   return S_OK;
}

STDMETHODIMP Decal::put_HasVerticalText(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fVerticalText = VBTOF(newVal);

   SetDirtyDraw();

   EnsureSize();

   STOPUNDO

   return S_OK;
}
