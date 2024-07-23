// Decal.cpp: implementation of the Decal class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 
#include "renderer/Shader.h"

#define AUTOLEADING (tm.tmAscent - tm.tmInternalLeading/4)

Decal::Decal()
{
}

Decal::~Decal()
{
   assert(m_rd == nullptr);
   SAFE_RELEASE(m_pIFont);
}

Decal *Decal::CopyForPlay(PinTable *live_table)
{
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(Decal, live_table)
   dst->m_backglass = m_backglass;
#ifndef __STANDALONE__
   m_pIFont->Clone(&dst->m_pIFont);
#endif
   dst->EnsureSize();
   return dst;
}

HRESULT Decal::Init(PinTable * const ptable, const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   m_ptable = ptable;
   SetDefaults(fromMouseClick);
   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;
   EnsureSize();
   return forPlay ? S_OK : InitVBA(fTrue, 0, nullptr);
}

void Decal::SetDefaults(const bool fromMouseClick)
{
#define regKey Settings::DefaultPropsDecal

   m_d.m_width = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Width"s, 100.0f) : 100.0f;
   m_d.m_height = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Height"s, 100.0f) : 100.0f;
   m_d.m_rotation = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Rotation"s, 0.f) : 0.f;

   bool hr = g_pvp->m_settings.LoadValue(regKey, "Image"s, m_d.m_szImage);
   if (!hr || !fromMouseClick)
      m_d.m_szImage.clear();

   hr = g_pvp->m_settings.LoadValue(regKey, "Surface"s, m_d.m_szSurface);
   if (!hr || !fromMouseClick)
      m_d.m_szSurface.clear();

   m_d.m_decaltype = fromMouseClick ? (enum DecalType)g_pvp->m_settings.LoadValueWithDefault(regKey, "DecalType"s, (int)DecalImage) : DecalImage;
   hr = g_pvp->m_settings.LoadValue(regKey, "Text"s, m_d.m_sztext);
   if (!hr || !fromMouseClick)
      m_d.m_sztext.clear();

   m_d.m_sizingtype = fromMouseClick ? (enum SizingType)g_pvp->m_settings.LoadValueWithDefault(regKey, "Sizing"s, (int)ManualSize) : ManualSize;
   m_d.m_color = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Color"s, (int)RGB(0,0,0)) : RGB(0,0,0);
   m_d.m_verticalText = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "VerticalText"s, false) : false;

   if (!m_pIFont)
   {
#ifndef __STANDALONE__
      FONTDESC fd;
      fd.cbSizeofstruct = sizeof(FONTDESC);

      float fTmp;
      hr = g_pvp->m_settings.LoadValue(regKey, "FontSize"s, fTmp);
      fd.cySize.int64 = hr && fromMouseClick ? (LONGLONG)(fTmp * 10000.0f) : 142500;

      char tmp[MAXSTRING];
      hr = g_pvp->m_settings.LoadValue(regKey, "FontName"s, tmp, MAXSTRING);
      if (!hr || !fromMouseClick)
         fd.lpstrName = (LPOLESTR)(L"Arial Black");
      else
      {
         const int len = lstrlen(tmp) + 1;
         fd.lpstrName = (LPOLESTR)malloc(len*sizeof(WCHAR));
         memset(fd.lpstrName, 0, len*sizeof(WCHAR));
         MultiByteToWideCharNull(CP_ACP, 0, tmp, -1, fd.lpstrName, len);
      }

      fd.sWeight = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "FontWeight"s, FW_NORMAL) : FW_NORMAL;
      fd.sCharset = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "FontCharSet"s, 0) : 0;
      fd.fItalic = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "FontItalic"s, false) : false;
      fd.fUnderline = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "FontUnderline"s, false) : false;
      fd.fStrikethrough = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "FontStrikeThrough"s, false) : false;

      OleCreateFontIndirect(&fd, IID_IFont, (void **)&m_pIFont);
#endif
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
#define regKey Settings::DefaultPropsDecal

   g_pvp->m_settings.SaveValue(regKey, "Width"s, m_d.m_width);
   g_pvp->m_settings.SaveValue(regKey, "Height"s, m_d.m_height);
   g_pvp->m_settings.SaveValue(regKey, "Rotation"s, m_d.m_rotation);
   g_pvp->m_settings.SaveValue(regKey, "Image"s, m_d.m_szImage);
   g_pvp->m_settings.SaveValue(regKey, "DecalType"s, m_d.m_decaltype);
   g_pvp->m_settings.SaveValue(regKey, "Text"s, m_d.m_sztext);
   g_pvp->m_settings.SaveValue(regKey, "Sizing"s, m_d.m_sizingtype);
   g_pvp->m_settings.SaveValue(regKey, "Color"s, (int)m_d.m_color);
   g_pvp->m_settings.SaveValue(regKey, "VerticalText"s, m_d.m_verticalText);
   g_pvp->m_settings.SaveValue(regKey, "Surface"s, m_d.m_szSurface);

   if (m_pIFont)
   {
#ifndef __STANDALONE__
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
      g_pvp->m_settings.SaveValue(regKey, "FontSize"s, fTmp);

      const size_t charCnt = wcslen(fd.lpstrName) + 1;
      char * const strTmp = new char[2 * charCnt];
      WideCharToMultiByteNull(CP_ACP, 0, fd.lpstrName, -1, strTmp, (int)(2 * charCnt), nullptr, nullptr);
      g_pvp->m_settings.SaveValue(regKey, "FontName"s, strTmp);
      delete[] strTmp;
      const int weight = fd.sWeight;
      const int charset = fd.sCharset;
      g_pvp->m_settings.SaveValue(regKey, "FontWeight"s, weight);
      g_pvp->m_settings.SaveValue(regKey, "FontCharSet"s, charset);
      g_pvp->m_settings.SaveValue(regKey, "FontItalic"s, fd.fItalic);
      g_pvp->m_settings.SaveValue(regKey, "FontUnderline"s, fd.fUnderline);
      g_pvp->m_settings.SaveValue(regKey, "FontStrikeThrough"s, fd.fStrikethrough);
#endif
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
#ifndef __STANDALONE__
   const int len = (int)m_d.m_sztext.length();
   const HFONT hFont = GetFont();
   constexpr int alignment = DT_LEFT;

   const CClientDC clientDC(nullptr);
   const CFont hFontOld = clientDC.SelectObject(hFont);

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
#endif
}

float Decal::GetDepth(const Vertex3Ds& viewDir) const
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   return !m_backglass ? (viewDir.x * m_d.m_vCenter.x + viewDir.y * m_d.m_vCenter.y + viewDir.z*height) : 0.f;
}

void Decal::UpdateBounds()
{
   if (m_backglass)
      m_boundingSphereCenter.Set(0.f, 0.f, 0.f);
   else
   {
      const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
      m_boundingSphereCenter.Set(m_d.m_vCenter.x, m_d.m_vCenter.y, height);
   }
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

HRESULT Decal::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
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
#ifndef __STANDALONE__
   IPersistStream * ips;
   m_pIFont->QueryInterface(IID_IPersistStream, (void **)&ips);
   ips->Save(pstm, TRUE);
   SAFE_RELEASE_NO_RCC(ips);
#endif

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
#ifndef __STANDALONE__
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
      SAFE_RELEASE_NO_RCC(ips);

#else
      // https://github.com/freezy/VisualPinball.Engine/blob/master/VisualPinball.Engine/VPT/Font.cs#L25

      char data[255];

      ULONG read;
      pbr->ReadBytes(data, 3, &read);
      pbr->ReadBytes(data, 1, &read); // Italic
      pbr->ReadBytes(data, 2, &read); // Weight
      pbr->ReadBytes(data, 4, &read); // Size
      pbr->ReadBytes(data, 1, &read); // nameLen
      pbr->ReadBytes(data, (int)data[0], &read); // name
#endif
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
   if ((((m_d.m_sizingtype != AutoSize) || (m_d.m_decaltype == DecalImage)) && (m_d.m_sizingtype != AutoWidth)) ||
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
 #ifndef __STANDALONE__
      m_pIFont->get_Size(&cy);
 #endif

      double rh = (double)cy.Lo * (1.0 / 2545.0);

      if (m_d.m_verticalText)
         rh *= (double)m_d.m_sztext.length();

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
#ifndef __STANDALONE__
         m_pIFont->get_Size(&cy);
#endif

         int sizex, sizey;
         GetTextSize(&sizex, &sizey);

         double rh = (double)cy.Lo * (1.0 / 2545.0);

         if (m_d.m_verticalText)
         {
            rh *= (double)m_d.m_sztext.length();
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
#ifndef __STANDALONE__
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
#else
   return 0L;
#endif
}


#pragma region Physics

void Decal::GetHitShapes(vector<HitObject*> &pvho)
{
}

void Decal::GetHitShapesDebug(vector<HitObject*> &pvho)
{
}

void Decal::EndPlay()
{
   IEditable::EndPlay();
}

#pragma endregion


#pragma region Rendering

void Decal::RenderSetup(RenderDevice *device)
{
   assert(m_rd == nullptr);
   m_rd = device;

   UpdateBounds();
   
   if (m_d.m_decaltype == DecalText)
   {
#ifndef __STANDALONE__
      RECT rcOut = { };
      const int len = (int)m_d.m_sztext.length();
      const HFONT hFont = GetFont();
      int alignment = DT_LEFT;

      const CClientDC clientDC(nullptr);

      CFont hFontOld = clientDC.SelectObject(hFont);

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
      dc.CreateCompatibleDC(nullptr);
      const CBitmap oldBmp = dc.SelectObject(hbm);

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
#endif
   }

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

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

   VertexBuffer *vertexBuffer = new VertexBuffer(m_rd, 4);
   Vertex3D_NoTex2 *vertices;
   vertexBuffer->lock(0, 0, (void**)&vertices, VertexBuffer::WRITEONLY);
   float z = m_backglass ? 0.f : (height + 0.2f);

   vertices[0].x = m_d.m_vCenter.x + sn*(halfheight + leading) - cs*halfwidth;
   vertices[0].y = m_d.m_vCenter.y - cs*(halfheight + leading) - sn*halfwidth;
   vertices[0].z = z;
   vertices[0].nx = 0.f;
   vertices[0].ny = 0.f;
   vertices[0].nz = 1.f;
   vertices[0].tu = 0;
   vertices[0].tv = 0;

   vertices[1].x = m_d.m_vCenter.x + sn*(halfheight + leading) + cs*halfwidth;
   vertices[1].y = m_d.m_vCenter.y - cs*(halfheight + leading) + sn*halfwidth;
   vertices[1].z = z;
   vertices[1].nx = 0.f;
   vertices[1].ny = 0.f;
   vertices[1].nz = 1.f;
   vertices[1].tu = 1.0f;
   vertices[1].tv = 0;

   vertices[2].x = m_d.m_vCenter.x - sn*(halfheight + descent) - cs*halfwidth;
   vertices[2].y = m_d.m_vCenter.y + cs*(halfheight + descent) - sn*halfwidth;
   vertices[2].z = z;
   vertices[2].nx = 0.f;
   vertices[2].ny = 0.f;
   vertices[2].nz = 1.f;
   vertices[2].tu = 0;
   vertices[2].tv = 1.0f;

   vertices[3].x = m_d.m_vCenter.x - sn*(halfheight + descent) + cs*halfwidth;
   vertices[3].y = m_d.m_vCenter.y + cs*(halfheight + descent) + sn*halfwidth;
   vertices[3].z = z;
   vertices[3].nx = 0.f;
   vertices[3].ny = 0.f;
   vertices[3].nz = 1.f;
   vertices[3].tu = 1.0f;
   vertices[3].tv = 1.0f;

   if (m_backglass)
   {
      const float xmult = getBGxmult();
      const float ymult = getBGymult();
      for (int i = 0; i < 4; ++i)
      {
         vertices[i].x = vertices[i].x * xmult  - 0.5f;
         vertices[i].y = vertices[i].y * ymult - 0.5f;
      }
   }
   vertexBuffer->unlock();

   delete m_meshBuffer;
   m_meshBuffer = new MeshBuffer(m_wzName, vertexBuffer);
}

void Decal::RenderRelease()
{
   assert(m_rd != nullptr);
   delete m_textImg;
   delete m_meshBuffer;
   m_textImg = nullptr;
   m_meshBuffer = nullptr;
   m_rd = nullptr;
}

void Decal::UpdateAnimation(const float diff_time_msec)
{
   assert(m_rd != nullptr);
}

void Decal::Render(const unsigned int renderMask)
{
   assert(m_rd != nullptr);
   const bool isStaticOnly = renderMask & Player::STATIC_ONLY;
   const bool isDynamicOnly = renderMask & Player::DYNAMIC_ONLY;
   const bool isReflectionPass = renderMask & Player::REFLECTION_PASS;
   TRACE_FUNCTION();

   if (m_backglass && (!GetPTable()->GetDecalsEnabled() || g_pplayer->m_stereo3D == STEREO_VR))
      return;

   //!! should just check if material has no opacity enabled, but this is crucial for HV setup performance like-is
   const Material *const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   if (!(   (!isDynamicOnly && (m_backglass || !mat->m_bOpacityActive)) // Static prerendering
         || (!isStaticOnly && (!m_backglass && mat->m_bOpacityActive)))) // Not prerendered part pass
      return;

   m_rd->ResetRenderState();

   m_rd->basicShader->SetMaterial(mat);

   if (m_d.m_decaltype != DecalImage)
   {
      if (!m_backglass)
         m_rd->basicShader->SetTechniqueMaterial(SHADER_TECHNIQUE_basic_with_texture, mat, false);
      else
         m_rd->basicShader->SetTechnique(SHADER_TECHNIQUE_bg_decal_with_texture);
      m_rd->basicShader->SetTexture(SHADER_tex_base_color, m_textImg);
   }
   else
   {
      Texture *const pin = m_ptable->GetImage(m_d.m_szImage);
      if (pin)
      {
         if (!m_backglass)
            m_rd->basicShader->SetTechniqueMaterial(SHADER_TECHNIQUE_basic_with_texture, mat, pin->m_alphaTestValue >= 0.f && !pin->m_pdsBuffer->IsOpaque());
         else
            m_rd->basicShader->SetTechnique(SHADER_TECHNIQUE_bg_decal_with_texture);
         // Set texture to mirror, so the alpha state of the texture blends correctly to the outside
         m_rd->basicShader->SetTexture(SHADER_tex_base_color, pin, SF_TRILINEAR, SA_MIRROR, SA_MIRROR);
         m_rd->basicShader->SetAlphaTestValue(pin->m_alphaTestValue);
      }
      else
      {
         if (!m_backglass)
            m_rd->basicShader->SetTechniqueMaterial(SHADER_TECHNIQUE_basic_without_texture, mat);
         else
            m_rd->basicShader->SetTechnique(SHADER_TECHNIQUE_bg_decal_without_texture);
      }
   }

   m_rd->EnableAlphaBlend(false);

   if (!m_backglass)
   {
      constexpr float depthbias = -5.f;
      m_rd->SetRenderStateDepthBias(depthbias);
   }
   else
   {
      m_rd->SetRenderStateDepthBias(0.0f);
      const vec4 staticColor(1.0f, 1.0f, 1.0f, 1.0f);
      m_rd->basicShader->SetVector(SHADER_cBase_Alpha, &staticColor);
   }

   if (m_backglass)
   {
      Matrix3D matWorldViewProj; // MVP to move from back buffer space (0..w, 0..h) to clip space (-1..1, -1..1)
      matWorldViewProj.SetIdentity();
      matWorldViewProj._11 = 2.0f / (float)m_rd->GetMSAABackBufferTexture()->GetWidth();
      matWorldViewProj._41 = -1.0f;
      matWorldViewProj._22 = -2.0f / (float)m_rd->GetMSAABackBufferTexture()->GetHeight();
      matWorldViewProj._42 = 1.0f;
      #ifdef ENABLE_SDL
      struct
      {
         Matrix3D matWorld;
         Matrix3D matView;
         Matrix3D matWorldView;
         Matrix3D matWorldViewInverseTranspose;
         Matrix3D matWorldViewProj[2];
      } matrices;
      memcpy(&matrices.matWorldViewProj[0].m[0][0], &matWorldViewProj.m[0][0], 4 * 4 * sizeof(float));
      memcpy(&matrices.matWorldViewProj[1].m[0][0], &matWorldViewProj.m[0][0], 4 * 4 * sizeof(float));
      m_rd->basicShader->SetUniformBlock(SHADER_basicMatrixBlock, &matrices.matWorld.m[0][0]);
      #else
      m_rd->basicShader->SetMatrix(SHADER_matWorldViewProj, &matWorldViewProj);
      #endif
   }

   m_rd->DrawMesh(m_rd->basicShader, !m_backglass, m_boundingSphereCenter, 0.f, m_meshBuffer, RenderDevice::TRIANGLESTRIP, 0, 4);

   if (m_backglass)
      g_pplayer->UpdateBasicShaderMatrix();
}

#pragma endregion


#pragma region ScriptProxy

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

#pragma endregion
