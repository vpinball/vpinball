// Decal.cpp: implementation of the Decal class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 
#include "Shader.h"

#define AUTOLEADING (tm.tmAscent - tm.tmInternalLeading/4)

Decal::Decal()
{
   m_pIFont = nullptr;
   m_vertexBuffer = nullptr;
   m_textImg = nullptr;
   m_ptable = nullptr;
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
   SAFE_BUFFER_RELEASE(m_vertexBuffer);
}

HRESULT Decal::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;

   SetDefaults(fromMouseClick);

   InitVBA(fTrue, 0, nullptr);

   EnsureSize();

   return S_OK;
}

void Decal::SetDefaults(bool fromMouseClick)
{
#define regKey regKey[RegName::DefaultPropsDecal]

   m_d.m_width = fromMouseClick ? LoadValueFloatWithDefault(regKey, "Width"s, 100.0f) : 100.0f;
   m_d.m_height = fromMouseClick ? LoadValueFloatWithDefault(regKey, "Height"s, 100.0f) : 100.0f;
   m_d.m_rotation = fromMouseClick ? LoadValueFloatWithDefault(regKey, "Rotation"s, 0.f) : 0.f;

   HRESULT hr = LoadValue(regKey, "Image"s, m_d.m_szImage);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage.clear();

   hr = LoadValue(regKey, "Surface"s, m_d.m_szSurface);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface.clear();

   m_d.m_decaltype = fromMouseClick ? (enum DecalType)LoadValueIntWithDefault(regKey, "DecalType"s, (int)DecalImage) : DecalImage;

   hr = LoadValue(regKey, "Text"s, m_d.m_sztext);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_sztext.clear();

   m_d.m_sizingtype = fromMouseClick ? (enum SizingType)LoadValueIntWithDefault(regKey, "Sizing"s, (int)ManualSize) : ManualSize;
   m_d.m_color = fromMouseClick ? LoadValueIntWithDefault(regKey, "Color"s, RGB(0,0,0)) : RGB(0,0,0);
   m_d.m_verticalText = fromMouseClick ? LoadValueBoolWithDefault(regKey, "VerticalText"s, false) : false;

   if (!m_pIFont)
   {
      FONTDESC fd;
      fd.cbSizeofstruct = sizeof(FONTDESC);

      float fTmp;
      hr = LoadValue(regKey, "FontSize"s, fTmp);
      fd.cySize.int64 = (hr == S_OK) && fromMouseClick ? (LONGLONG)(fTmp * 10000.0f) : 142500;

      char tmp[MAXSTRING];
      hr = LoadValue(regKey, "FontName"s, tmp, MAXSTRING);
      if ((hr != S_OK) || !fromMouseClick)
         fd.lpstrName = (LPOLESTR)(L"Arial Black");
      else
      {
         const int len = lstrlen(tmp) + 1;
         fd.lpstrName = (LPOLESTR)malloc(len*sizeof(WCHAR));
         memset(fd.lpstrName, 0, len*sizeof(WCHAR));
         MultiByteToWideCharNull(CP_ACP, 0, tmp, -1, fd.lpstrName, len);
      }

      fd.sWeight = fromMouseClick ? LoadValueIntWithDefault(regKey, "FontWeight"s, FW_NORMAL) : FW_NORMAL;
      fd.sCharset = fromMouseClick ? LoadValueIntWithDefault(regKey, "FontCharSet"s, 0) : 0;
      fd.fItalic = fromMouseClick ? LoadValueBoolWithDefault(regKey, "FontItalic"s, false) : false;
      fd.fUnderline = fromMouseClick ? LoadValueBoolWithDefault(regKey, "FontUnderline"s, false) : false;
      fd.fStrikethrough = fromMouseClick ? LoadValueBoolWithDefault(regKey, "FontStrikeThrough"s, false) : false;

      OleCreateFontIndirect(&fd, IID_IFont, (void **)&m_pIFont);
   }

#undef regKey
}

char * Decal::GetFontName()
{
    if(m_pIFont)
    {
        CComBSTR bstr;
        /*HRESULT hr =*/ m_pIFont->get_Name(&bstr);

        static char fontName[LF_FACESIZE];
        WideCharToMultiByteNull(CP_ACP, 0, bstr, -1, fontName, LF_FACESIZE, nullptr, nullptr);
        return fontName;
    }
    return nullptr;
}

void Decal::WriteRegDefaults()
{
#define regKey regKey[RegName::DefaultPropsDecal]

   SaveValueFloat(regKey, "Width"s, m_d.m_width);
   SaveValueFloat(regKey, "Height"s, m_d.m_height);
   SaveValueFloat(regKey, "Rotation"s, m_d.m_rotation);
   SaveValue(regKey, "Image"s, m_d.m_szImage);
   SaveValueInt(regKey, "DecalType"s, m_d.m_decaltype);
   SaveValue(regKey, "Text"s, m_d.m_sztext);
   SaveValueInt(regKey, "Sizing"s, m_d.m_sizingtype);
   SaveValueInt(regKey, "Color"s, m_d.m_color);
   SaveValueBool(regKey, "VerticalText"s, m_d.m_verticalText);
   SaveValue(regKey, "Surface"s, m_d.m_szSurface);

   if (m_pIFont)
   {
      FONTDESC fd;
      fd.cbSizeofstruct = sizeof(FONTDESC);
      m_pIFont->get_Size(&fd.cySize);
      m_pIFont->get_Name(&fd.lpstrName); //!! BSTR
      m_pIFont->get_Weight(&fd.sWeight);
      m_pIFont->get_Charset(&fd.sCharset);
      m_pIFont->get_Italic(&fd.fItalic);
      m_pIFont->get_Underline(&fd.fUnderline);
      m_pIFont->get_Strikethrough(&fd.fStrikethrough);

      const float fTmp = (float)(fd.cySize.int64 / 10000.0);
      SaveValueFloat(regKey, "FontSize"s, fTmp);

      const size_t charCnt = wcslen(fd.lpstrName) + 1;
      char * const strTmp = new char[2 * charCnt];
      WideCharToMultiByteNull(CP_ACP, 0, fd.lpstrName, -1, strTmp, (int)(2 * charCnt), nullptr, nullptr);
      SaveValue(regKey, "FontName"s, strTmp);
      delete[] strTmp;
      const int weight = fd.sWeight;
      const int charset = fd.sCharset;
      SaveValueInt(regKey, "FontWeight"s, weight);
      SaveValueInt(regKey, "FontCharSet"s, charset);
      SaveValueInt(regKey, "FontItalic"s, fd.fItalic);
      SaveValueInt(regKey, "FontUnderline"s, fd.fUnderline);
      SaveValueInt(regKey, "FontStrikeThrough"s, fd.fStrikethrough);
   }

#undef regKey
}


void Decal::UIRenderPass1(Sur * const psur)
{
   if (!m_backglass || GetPTable()->GetDecalsEnabled())
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

void Decal::UIRenderPass2(Sur * const psur)
{
   if (!m_backglass || GetPTable()->GetDecalsEnabled())
   {
      psur->SetBorderColor(RGB(0, 0, 0), false, 0);
      psur->SetFillColor(-1);
      psur->SetObject(this);
      psur->SetObject(nullptr);

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

void Decal::GetTimers(vector<HitTimer*> &pvht)
{
   IEditable::BeginPlay();
}

void Decal::GetTextSize(int * const px, int * const py)
{
   const int len = (int)m_d.m_sztext.length();
   const HFONT hFont = GetFont();
   constexpr int alignment = DT_LEFT;

   const CClientDC clientDC(nullptr);
   const HFONT hFontOld = clientDC.SelectObject(hFont);

   TEXTMETRIC tm;
   clientDC.GetTextMetrics(tm);
   if (m_d.m_verticalText)
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
         clientDC.DrawText(m_d.m_sztext.c_str()+i, 1, rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT);

         *px = max(*px, (int)rcOut.right);
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
      clientDC.DrawText(m_d.m_sztext.c_str(), len, rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT);

      *px = rcOut.right;
   }

   clientDC.SelectObject(hFontOld);

   DeleteObject(hFont);
}

void Decal::PreRenderText()
{
   if (m_d.m_decaltype != DecalText)
      return;

   RECT rcOut = { };
   const int len = (int)m_d.m_sztext.length();
   const HFONT hFont = GetFont();
   int alignment = DT_LEFT;

   const CClientDC clientDC(nullptr);

   HFONT hFontOld = clientDC.SelectObject(hFont);

   TEXTMETRIC tm;
   clientDC.GetTextMetrics(tm);

   float charheight;
   if (m_d.m_verticalText)
   {
      int maxwidth = 0;

      for (int i = 0; i < len; i++)
      {
         rcOut.left = 0;
         rcOut.top = 0;//-tm.tmInternalLeading + 2; // Leave a pixel for anti-aliasing;
         rcOut.right = 1;
         rcOut.bottom = 1;
         clientDC.DrawText(m_d.m_sztext.c_str()+i, 1, rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT);
         maxwidth = max(maxwidth, (int)rcOut.right);
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
      clientDC.DrawText(m_d.m_sztext.c_str(), len, rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT);

      charheight = m_realheight;
   }

   clientDC.SelectObject(hFontOld);

   // Calculate the percentage of the texture which is for oomlats and commas.
   const float invascent = charheight / (float)tm.tmAscent;
   m_leading = (float)tm.tmInternalLeading * invascent /*m_d.m_height*/;
   m_descent = (float)tm.tmDescent * invascent;

   m_textImg = new BaseTexture(rcOut.right, rcOut.bottom, BaseTexture::SRGBA);

   if (m_d.m_color == RGB(255, 255, 255))
      m_d.m_color = RGB(254, 255, 255); //m_pinimage.SetTransparentColor(RGB(0,0,0));
   else if (m_d.m_color == RGB(0, 0, 0))
      m_d.m_color = RGB(0, 0, 1);

   BITMAPINFO bmi = {};
   bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth = m_textImg->width();
   bmi.bmiHeader.biHeight = -(LONG)m_textImg->height();
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biBitCount = 32;
   bmi.bmiHeader.biCompression = BI_RGB;
   bmi.bmiHeader.biSizeImage = 0;

   void *bits;
   const HBITMAP hbm = CreateDIBSection(0, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);

   assert(hbm);

   CDC dc;
   /*const HDC hdc =*/ dc.CreateCompatibleDC(nullptr);
   const HBITMAP oldBmp = dc.SelectObject(hbm);

   dc.SelectObject(reinterpret_cast<HBRUSH>(dc.GetStockObject(WHITE_BRUSH)));
   dc.PatBlt(0, 0, rcOut.right, rcOut.bottom, PATCOPY);

   hFontOld = dc.SelectObject(hFont);

   dc.SetTextColor(m_d.m_color);
   dc.SetBkMode(TRANSPARENT);
   dc.SetTextAlign(TA_LEFT | TA_TOP | TA_NOUPDATECP);
   alignment = DT_CENTER;

   if (m_d.m_verticalText)
   {
      for (int i = 0; i < len; i++)
      {
         rcOut.top = AUTOLEADING * i;//-tm.tmInternalLeading + 2; // Leave a pixel for anti-aliasing;
         rcOut.bottom = rcOut.top + 100;
         dc.DrawText(m_d.m_sztext.c_str()+i, 1, rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK);
      }
   }
   else
      dc.DrawText(m_d.m_sztext.c_str(), len, rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK);

   // Copy and set to opaque
   const D3DCOLOR* __restrict bitsd = (D3DCOLOR*)bits;
         D3DCOLOR* __restrict dest = (D3DCOLOR*)m_textImg->data();
   for (unsigned int i = 0; i < m_textImg->height(); i++)
   {
      for (unsigned int l = 0; l < m_textImg->width(); l++, dest++, bitsd++)
         *dest = *bitsd | 0xFF000000u;
      dest += m_textImg->pitch()/4 - m_textImg->width();
   }

   dc.SelectObject(hFontOld);
   dc.SelectObject(oldBmp);
   DeleteObject(hFont);
   DeleteObject(hbm);
}

void Decal::GetHitShapes(vector<HitObject*> &pvho)
{
}

void Decal::GetHitShapesDebug(vector<HitObject*> &pvho)
{
}

void Decal::EndPlay()
{
   if (m_textImg)
   {
      delete m_textImg;
      m_textImg = nullptr;
   }

   SAFE_BUFFER_RELEASE(m_vertexBuffer);

   IEditable::EndPlay();
}

void Decal::RenderDynamic()
{
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   if (!m_backglass //!! should just check if material has opacity enabled, but this is crucial for HV setup performance like-is
      && mat->m_bOpacityActive)
      RenderObject();
}

static constexpr WORD rgi0123[4] = { 0, 1, 2, 3 };

void Decal::RenderSetup()
{
   PreRenderText();

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

   SAFE_BUFFER_RELEASE(m_vertexBuffer);
   const DWORD vertexType = m_backglass ? MY_D3DTRANSFORMED_NOTEX2_VERTEX : MY_D3DFVF_NOTEX2_VERTEX;
   m_vertexBuffer = new VertexBuffer(m_backglass ? g_pplayer->m_pin3d.m_pd3dSecondaryDevice : g_pplayer->m_pin3d.m_pd3dPrimaryDevice, 4, 0, vertexType);

   Vertex3D_NoTex2 *vertices;
   m_vertexBuffer->lock(0, 0, (void**)&vertices, VertexBuffer::WRITEONLY);

   vertices[0].x = m_d.m_vCenter.x + sn*(halfheight + leading) - cs*halfwidth;
   vertices[0].y = m_d.m_vCenter.y - cs*(halfheight + leading) - sn*halfwidth;

   vertices[1].x = m_d.m_vCenter.x + sn*(halfheight + leading) + cs*halfwidth;
   vertices[1].y = m_d.m_vCenter.y - cs*(halfheight + leading) + sn*halfwidth;

   vertices[2].x = m_d.m_vCenter.x - sn*(halfheight + descent) + cs*halfwidth;
   vertices[2].y = m_d.m_vCenter.y + cs*(halfheight + descent) + sn*halfwidth;

   vertices[3].x = m_d.m_vCenter.x - sn*(halfheight + descent) - cs*halfwidth;
   vertices[3].y = m_d.m_vCenter.y + cs*(halfheight + descent) - sn*halfwidth;

   if (!m_backglass)
   {
      for (int i = 0; i < 4; i++)
         vertices[i].z = height + 0.2f;

      SetNormal(vertices, rgi0123, 4);
   }
   else
   {
      const float  mult = getBGxmult();
      const float ymult = getBGymult();

      for (int i = 0; i < 4; ++i)
      {
         vertices[i].x = vertices[i].x* mult - 0.5f;
         vertices[i].y = vertices[i].y*ymult - 0.5f;
         vertices[i].z = 0.0f;

         vertices[i].nx = 1.0f; //!! as this is the w component due to MY_D3DTRANSFORMED_NOTEX2_VERTEX usage
         vertices[i].ny = 0.0f;
         vertices[i].nz = 0.0f;
      }
   }

   vertices[0].tu = 0;
   vertices[0].tv = 0;
   vertices[1].tu = 1.0f;
   vertices[1].tv = 0;
   vertices[2].tu = 1.0f;
   vertices[2].tv = 1.0f;
   vertices[3].tu = 0;
   vertices[3].tv = 1.0f;

   m_vertexBuffer->unlock();
}

float Decal::GetDepth(const Vertex3Ds& viewDir) const
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y) * m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
   return !m_backglass ? (viewDir.x * m_d.m_vCenter.x + viewDir.y * m_d.m_vCenter.y + viewDir.z*height) : 0.f;
}

void Decal::RenderObject()
{
   if (m_backglass && !GetPTable()->GetDecalsEnabled())
      return;

   RenderDevice * const pd3dDevice = m_backglass ? g_pplayer->m_pin3d.m_pd3dSecondaryDevice : g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   if (m_backglass && (m_ptable->m_tblMirrorEnabled^m_ptable->m_reflectionEnabled))
      pd3dDevice->SetRenderStateCulling(RenderDevice::CULL_NONE);
   else
      pd3dDevice->SetRenderStateCulling(RenderDevice::CULL_CCW);

   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   pd3dDevice->basicShader->SetMaterial(mat);

   //pd3dDevice->basicShader->SetFloat("fmaterialAlpha",1.0f);

   if (m_d.m_decaltype != DecalImage)
   {
      if (!m_backglass)
         pd3dDevice->basicShader->SetTechniqueMetal(SHADER_TECHNIQUE_basic_with_texture, mat->m_bIsMetal);
      else
         pd3dDevice->basicShader->SetTechnique(SHADER_TECHNIQUE_bg_decal_with_texture);
      pd3dDevice->basicShader->SetTexture(SHADER_tex_base_color, m_textImg);
      pd3dDevice->basicShader->SetAlphaTestValue(-1.0f);
   }
   else
   {
      Texture *const pin = m_ptable->GetImage(m_d.m_szImage);
      if (pin)
      {
         if (!m_backglass)
            pd3dDevice->basicShader->SetTechniqueMetal(SHADER_TECHNIQUE_basic_with_texture, mat->m_bIsMetal);
         else
            pd3dDevice->basicShader->SetTechnique(SHADER_TECHNIQUE_bg_decal_with_texture);
         // Set texture to mirror, so the alpha state of the texture blends correctly to the outside
         pd3dDevice->basicShader->SetTexture(SHADER_tex_base_color, pin, SF_TRILINEAR, SA_MIRROR, SA_MIRROR);
         pd3dDevice->basicShader->SetAlphaTestValue(pin->m_alphaTestValue * (float)(1.0 / 255.0));
      }
      else
      {
         if (!m_backglass)
            pd3dDevice->basicShader->SetTechniqueMetal(SHADER_TECHNIQUE_basic_without_texture, mat->m_bIsMetal);
         else
            pd3dDevice->basicShader->SetTechnique(SHADER_TECHNIQUE_bg_decal_without_texture);
      }
   }

   g_pplayer->m_pin3d.EnableAlphaBlend(false);

   if (!m_backglass)
   {
      constexpr float depthbias = -5.f;
      pd3dDevice->SetRenderStateDepthBias(depthbias);
   }
   else
   {
      pd3dDevice->SetRenderStateDepthBias(0.0f);
      const vec4 staticColor(1.0f, 1.0f, 1.0f, 1.0f);
      pd3dDevice->basicShader->SetVector(SHADER_cBase_Alpha, &staticColor);
   }

   pd3dDevice->basicShader->Begin();
   pd3dDevice->DrawPrimitiveVB(RenderDevice::TRIANGLEFAN, m_backglass ? MY_D3DTRANSFORMED_NOTEX2_VERTEX : MY_D3DFVF_NOTEX2_VERTEX, m_vertexBuffer, 0, 4, true);
   pd3dDevice->basicShader->End();

   // Set the render state.
   //pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, RenderDevice::RS_FALSE); //!! not necessary anymore

   //if(m_backglass && (m_ptable->m_tblMirrorEnabled^m_ptable->m_reflectionEnabled))
   //   pd3dDevice->SetRenderStateCulling(RenderDevice::CULL_CCW);
}

void Decal::RenderStatic()
{
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);

   if (m_backglass //!! should just check if material has no opacity enabled, but this is crucial for HV setup performance like-is
      || !mat->m_bOpacityActive)
      RenderObject();
}

void Decal::SetObjectPos()
{
   m_vpinball->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Decal::Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter)
{
   ISelect::Rotate(ang, pvCenter, useElementCenter);

   m_d.m_rotation += ang;
}

HRESULT Decal::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool backupForPlay)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteVector2(FID(VCEN), m_d.m_vCenter);
   bw.WriteFloat(FID(WDTH), m_d.m_width);
   bw.WriteFloat(FID(HIGH), m_d.m_height);
   bw.WriteFloat(FID(ROTA), m_d.m_rotation);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteString(FID(TEXT), m_d.m_sztext);
   bw.WriteInt(FID(TYPE), m_d.m_decaltype);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteInt(FID(COLR), m_d.m_color);
   bw.WriteInt(FID(SIZE), m_d.m_sizingtype);

   bw.WriteBool(FID(VERT), m_d.m_verticalText);

   bw.WriteBool(FID(BGLS), m_backglass);

   ISelect::SaveData(pstm, hcrypthash);

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

bool Decal::LoadToken(const int id, BiffReader * const pbr)
{
   switch (id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(VCEN): pbr->GetVector2(m_d.m_vCenter); break;
   case FID(WDTH): pbr->GetFloat(m_d.m_width); break;
   case FID(HIGH): pbr->GetFloat(m_d.m_height); break;
   case FID(ROTA): pbr->GetFloat(m_d.m_rotation); break;
   case FID(IMAG): pbr->GetString(m_d.m_szImage); break;
   case FID(SURF): pbr->GetString(m_d.m_szSurface); break;
   case FID(NAME): pbr->GetWideString(m_wzName,sizeof(m_wzName)/sizeof(m_wzName[0])); break;
   case FID(TEXT): pbr->GetString(m_d.m_sztext); break;
   case FID(TYPE): pbr->GetInt(&m_d.m_decaltype); break;
   case FID(COLR): pbr->GetInt(m_d.m_color); break;
   case FID(MATR): pbr->GetString(m_d.m_szMaterial); break;
   case FID(SIZE): pbr->GetInt(&m_d.m_sizingtype); break;
   case FID(VERT): pbr->GetBool(m_d.m_verticalText); break;
   case FID(BGLS): pbr->GetBool(m_backglass); break;
   case FID(FONT):
   {
      if (!m_pIFont)
      {
         FONTDESC fd;
         fd.cbSizeofstruct = sizeof(FONTDESC);
         fd.lpstrName = (LPOLESTR)(L"Arial");
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

      break;
   }
   default: ISelect::LoadToken(id, pbr); break;
   }
   return true;
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
      ((m_d.m_decaltype == DecalText) && m_d.m_sztext.empty()))
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

      double rh = (double)cy.Lo * (1.0 / 2545.0);

      if (m_d.m_verticalText)
         rh *= m_d.m_sztext.length();

      m_realheight = (float)rh;
      m_realwidth = (float)(rh * sizex / sizey);
   }
   else // Auto aspect
   {
      m_realheight = m_d.m_height;

      if (m_d.m_decaltype == DecalImage)
      {
         Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
         m_realwidth = m_realheight;
         if (pin)
            m_realwidth *= (float)((double)pin->m_width / (double)pin->m_height);
      }
      else
      {
         CY cy;
         m_pIFont->get_Size(&cy);

         int sizex, sizey;
         GetTextSize(&sizex, &sizey);

         double rh = (double)cy.Lo * (1.0 / 2545.0);

         if (m_d.m_verticalText)
         {
            rh *= m_d.m_sztext.length();
            m_realheight = (float)rh;
            m_realwidth = m_d.m_width;
         }
         else
         {
            m_realwidth = (float)(rh * sizex / sizey);
            m_realheight = m_d.m_height;
         }
      }
   }
}

HFONT Decal::GetFont()
{
   LOGFONT lf = {};
   lf.lfHeight = -72;
   lf.lfCharSet = DEFAULT_CHARSET;
   lf.lfQuality = NONANTIALIASED_QUALITY;

   CComBSTR bstr;
   (void)m_pIFont->get_Name(&bstr);

   WideCharToMultiByteNull(CP_ACP, 0, bstr, -1, lf.lfFaceName, LF_FACESIZE, nullptr, nullptr);

   BOOL bl;
   (void)m_pIFont->get_Bold(&bl);

   lf.lfWeight = bl ? FW_BOLD : FW_NORMAL;

   (void)m_pIFont->get_Italic(&bl);

   lf.lfItalic = (BYTE)bl;

   const HFONT hFont = CreateFontIndirect(&lf);

   return hFont;
}

STDMETHODIMP Decal::get_Rotation(float *pVal)
{
   *pVal = m_d.m_rotation;

   return S_OK;
}

STDMETHODIMP Decal::put_Rotation(float newVal)
{
   m_d.m_rotation = newVal;

   return S_OK;
}

STDMETHODIMP Decal::get_Image(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szImage.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Decal::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, nullptr, nullptr);
   const Texture * const tex = m_ptable->GetImage(szImage);
   if (tex && tex->IsHDR())
   {
      ShowError("Cannot use a HDR image (.exr/.hdr) here");
      return E_FAIL;
   }
   m_d.m_szImage = szImage;

   return S_OK;
}

STDMETHODIMP Decal::get_Width(float *pVal)
{
   *pVal = m_d.m_width;

   return S_OK;
}

STDMETHODIMP Decal::put_Width(float newVal)
{
   m_d.m_width = newVal;
   EnsureSize();

   return S_OK;
}

STDMETHODIMP Decal::get_Height(float *pVal)
{
   *pVal = m_d.m_height;

   return S_OK;
}

STDMETHODIMP Decal::put_Height(float newVal)
{
   m_d.m_height = newVal;
   EnsureSize();

   return S_OK;
}

STDMETHODIMP Decal::get_X(float *pVal)
{
   *pVal = m_d.m_vCenter.x;
   m_vpinball->SetStatusBarUnitInfo(string(), true);

   return S_OK;
}

STDMETHODIMP Decal::put_X(float newVal)
{
   m_d.m_vCenter.x = newVal;

   return S_OK;
}

STDMETHODIMP Decal::get_Y(float *pVal)
{
   *pVal = m_d.m_vCenter.y;

   return S_OK;
}

STDMETHODIMP Decal::put_Y(float newVal)
{
   m_d.m_vCenter.y = newVal;

   return S_OK;
}

STDMETHODIMP Decal::get_Surface(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szSurface.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Decal::put_Surface(BSTR newVal)
{
   char buf[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXTOKEN, nullptr, nullptr);
   m_d.m_szSurface = buf;

   return S_OK;
}

STDMETHODIMP Decal::get_Type(DecalType *pVal)
{
   *pVal = m_d.m_decaltype;

   return S_OK;
}

STDMETHODIMP Decal::put_Type(DecalType newVal)
{
   m_d.m_decaltype = newVal;
   EnsureSize();

   return S_OK;
}

STDMETHODIMP Decal::get_Text(BSTR *pVal)
{
   WCHAR wz[MAXSTRING];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_sztext.c_str(), -1, wz, MAXSTRING);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Decal::put_Text(BSTR newVal)
{
   char buf[MAXSTRING];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXSTRING, nullptr, nullptr);
   m_d.m_sztext = buf;
   EnsureSize();

   return S_OK;
}

STDMETHODIMP Decal::get_SizingType(SizingType *pVal)
{
   *pVal = m_d.m_sizingtype;

   return S_OK;
}

STDMETHODIMP Decal::put_SizingType(SizingType newVal)
{
   m_d.m_sizingtype = newVal;
   EnsureSize();

   return S_OK;
}

STDMETHODIMP Decal::get_FontColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_color;

   return S_OK;
}

STDMETHODIMP Decal::put_FontColor(OLE_COLOR newVal)
{
   m_d.m_color = newVal;

   return S_OK;
}

STDMETHODIMP Decal::get_Material(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Decal::put_Material(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, nullptr, nullptr);
   m_d.m_szMaterial = buf;

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
   *pVal = FTOVB(m_d.m_verticalText);

   return S_OK;
}

STDMETHODIMP Decal::put_HasVerticalText(VARIANT_BOOL newVal)
{
   m_d.m_verticalText = VBTOb(newVal);
   EnsureSize();

   return S_OK;
}
