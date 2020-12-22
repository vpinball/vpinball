#include "StdAfx.h"

Textbox::Textbox()
{
   m_pIFont = NULL;
   m_texture = NULL;
}

Textbox::~Textbox()
{
   m_pIFont->Release();
}

HRESULT Textbox::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   const float width = LoadValueFloatWithDefault("DefaultProps\\TextBox", "Width", 100.0f);
   const float height = LoadValueFloatWithDefault("DefaultProps\\TextBox", "Height", 50.0f);

   m_d.m_v1.x = x;
   m_d.m_v1.y = y;
   m_d.m_v2.x = x + width;
   m_d.m_v2.y = y + height;

   SetDefaults(fromMouseClick);

   return InitVBA(fTrue, 0, NULL);//ApcProjectItem.Define(ptable->ApcProject, GetDispatch(), axTypeHostProjectItem/*axTypeHostClass*/, L"Textbox", NULL);
}

void Textbox::SetDefaults(bool fromMouseClick)
{
   //Textbox is always located on backdrop
   m_backglass = true;
   m_d.m_visible = true;

   FONTDESC fd;
   fd.cbSizeofstruct = sizeof(FONTDESC);
   bool free_lpstrName = false;

   if (!fromMouseClick)
   {
      m_d.m_backcolor = RGB(0, 0, 0);
      m_d.m_fontcolor = RGB(255, 255, 255);
      m_d.m_intensity_scale = 1.0f;
      m_d.m_tdr.m_TimerEnabled = false;
      m_d.m_tdr.m_TimerInterval = 100;
      m_d.m_talign = TextAlignRight;
      m_d.m_transparent = false;
      m_d.m_isDMD = false;
      m_d.m_sztext.clear();

      fd.cySize.int64 = (LONGLONG)(14.25f * 10000.0f);
      fd.lpstrName = L"Arial";
      fd.sWeight = FW_NORMAL;
      fd.sCharset = 0;
      fd.fItalic = 0;
      fd.fUnderline = 0;
      fd.fStrikethrough = 0;
   }
   else
   {
      m_d.m_backcolor = LoadValueIntWithDefault("DefaultProps\\TextBox", "BackColor", RGB(0, 0, 0));
      m_d.m_fontcolor = LoadValueIntWithDefault("DefaultProps\\TextBox", "FontColor", RGB(255, 255, 255));
      m_d.m_intensity_scale = LoadValueFloatWithDefault("DefaultProps\\TextBox", "IntensityScale", 1.0f);
      m_d.m_tdr.m_TimerEnabled = LoadValueBoolWithDefault("DefaultProps\\TextBox", "TimerEnabled", false) ? true : false;
      m_d.m_tdr.m_TimerInterval = LoadValueIntWithDefault("DefaultProps\\TextBox", "TimerInterval", 100);
      m_d.m_talign = (TextAlignment)LoadValueIntWithDefault("DefaultProps\\TextBox", "TextAlignment", TextAlignRight);
      m_d.m_transparent = LoadValueBoolWithDefault("DefaultProps\\TextBox", "Transparent", false);
      m_d.m_isDMD = LoadValueBoolWithDefault("DefaultProps\\TextBox", "DMD", false);

      const float fontSize = LoadValueFloatWithDefault("DefaultProps\\TextBox", "FontSize", 14.25f);
      fd.cySize.int64 = (LONGLONG)(fontSize * 10000.0f);

      char tmp[MAXSTRING];
      HRESULT hr;
      hr = LoadValueString("DefaultProps\\TextBox", "FontName", tmp, MAXSTRING);
      if (hr != S_OK)
         fd.lpstrName = L"Arial";
      else
      {
         const int len = lstrlen(tmp) + 1;
         fd.lpstrName = (LPOLESTR)malloc(len*sizeof(WCHAR));
         memset(fd.lpstrName, 0, len*sizeof(WCHAR));
         MultiByteToWideCharNull(CP_ACP, 0, tmp, -1, fd.lpstrName, len);

         free_lpstrName = true;
      }

      fd.sWeight = LoadValueIntWithDefault("DefaultProps\\TextBox", "FontWeight", FW_NORMAL);
      fd.sCharset = LoadValueIntWithDefault("DefaultProps\\TextBox", "FontCharSet", 0);
      fd.fItalic = LoadValueIntWithDefault("DefaultProps\\TextBox", "FontItalic", 0);
      fd.fUnderline = LoadValueIntWithDefault("DefaultProps\\TextBox", "FontUnderline", 0);
      fd.fStrikethrough = LoadValueIntWithDefault("DefaultProps\\TextBox", "FontStrikeThrough", 0);

      hr = LoadValueString("DefaultProps\\TextBox", "Text", tmp, MAXSTRING);
      if (hr != S_OK)
         m_d.m_sztext = "";
      else
         m_d.m_sztext = tmp;
   }

   OleCreateFontIndirect(&fd, IID_IFont, (void **)&m_pIFont);
   if (free_lpstrName)
      free(fd.lpstrName);
}

void Textbox::WriteRegDefaults()
{
   SaveValueInt("DefaultProps\\TextBox", "BackColor", m_d.m_backcolor);
   SaveValueInt("DefaultProps\\TextBox", "FontColor", m_d.m_fontcolor);
   SaveValueBool("DefaultProps\\TextBox", "TimerEnabled", m_d.m_tdr.m_TimerEnabled);
   SaveValueInt("DefaultProps\\TextBox", "TimerInterval", m_d.m_tdr.m_TimerInterval);
   SaveValueBool("DefaultProps\\TextBox", "Transparent", m_d.m_transparent);
   SaveValueBool("DefaultProps\\TextBox", "DMD", m_d.m_isDMD);

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
   SaveValueFloat("DefaultProps\\TextBox", "FontSize", fTmp);
   size_t charCnt = wcslen(fd.lpstrName) + 1;
   char * const strTmp = new char[2 * charCnt];
   WideCharToMultiByteNull(CP_ACP, 0, fd.lpstrName, -1, strTmp, (int)(2 * charCnt), NULL, NULL);
   SaveValueString("DefaultProps\\TextBox", "FontName", strTmp);
   delete[] strTmp;
   int weight = fd.sWeight;
   int charset = fd.sCharset;
   SaveValueInt("DefaultProps\\TextBox", "FontWeight", weight);
   SaveValueInt("DefaultProps\\TextBox", "FontCharSet", charset);
   SaveValueInt("DefaultProps\\TextBox", "FontItalic", fd.fItalic);
   SaveValueInt("DefaultProps\\TextBox", "FontUnderline", fd.fUnderline);
   SaveValueInt("DefaultProps\\TextBox", "FontStrikeThrough", fd.fStrikethrough);

   SaveValueString("DefaultProps\\TextBox", "Text", m_d.m_sztext);
}

char * Textbox::GetFontName()
{
    if (m_pIFont)
    {
        CComBSTR bstr;
        /*HRESULT hr =*/ m_pIFont->get_Name(&bstr);

        static char fontName[LF_FACESIZE];
        WideCharToMultiByteNull(CP_ACP, 0, bstr, -1, fontName, LF_FACESIZE, NULL, NULL);
        return fontName;
    }
    return NULL;
}

HFONT Textbox::GetFont()
{
    LOGFONT lf;
    ZeroMemory(&lf, sizeof(lf));

    lf.lfHeight = -72;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfQuality = NONANTIALIASED_QUALITY;

    CComBSTR bstr;
    HRESULT hr = m_pIFont->get_Name(&bstr);

    WideCharToMultiByteNull(CP_ACP, 0, bstr, -1, lf.lfFaceName, LF_FACESIZE, NULL, NULL);

    BOOL bl;
    hr = m_pIFont->get_Bold(&bl);

    lf.lfWeight = bl ? FW_BOLD : FW_NORMAL;

    hr = m_pIFont->get_Italic(&bl);

    lf.lfItalic = (BYTE)bl;

    HFONT hFont = CreateFontIndirect(&lf);

    return hFont;
}

STDMETHODIMP Textbox::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_ITextbox,
   };

   for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
   {
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;
   }
   return S_FALSE;
}

void Textbox::UIRenderPass1(Sur * const psur)
{
   psur->SetBorderColor(-1, false, 0);
   psur->SetFillColor(m_d.m_backcolor);
   psur->SetObject(this);

   psur->Rectangle(m_d.m_v1.x, m_d.m_v1.y, m_d.m_v2.x, m_d.m_v2.y);
}

void Textbox::UIRenderPass2(Sur * const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(this);
   psur->SetObject(NULL);

   psur->Rectangle(m_d.m_v1.x, m_d.m_v1.y, m_d.m_v2.x, m_d.m_v2.y);
}

void Textbox::GetTimers(vector<HitTimer*> &pvht)
{
   IEditable::BeginPlay();

   HitTimer * const pht = new HitTimer();
   pht->m_interval = m_d.m_tdr.m_TimerInterval >= 0 ? max(m_d.m_tdr.m_TimerInterval, MAX_TIMER_MSEC_INTERVAL) : -1;
   pht->m_nextfire = pht->m_interval;
   pht->m_pfe = (IFireEvents *)this;

   m_phittimer = pht;

   if (m_d.m_tdr.m_TimerEnabled)
      pvht.push_back(pht);
}

void Textbox::GetHitShapes(vector<HitObject*> &pvho)
{
}

void Textbox::GetHitShapesDebug(vector<HitObject*> &pvho)
{
}

void Textbox::EndPlay()
{
   if (m_texture)
   {
      delete m_texture;
      m_texture = NULL;

      m_pIFontPlay->Release();
   }

   IEditable::EndPlay();
}

void Textbox::RenderDynamic()
{
   TRACE_FUNCTION();

   const bool dmd = (m_d.m_isDMD || StrStrI(m_d.m_sztext.c_str(), "DMD") != NULL); //!! second part is VP10.0 legacy

   if (!m_d.m_visible || (dmd && !g_pplayer->m_texdmd))
      return;

   RenderDevice * const pd3dDevice = m_backglass ? g_pplayer->m_pin3d.m_pd3dSecondaryDevice : g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   if (m_ptable->m_tblMirrorEnabled^m_ptable->m_reflectionEnabled)
      pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE);
   else
      pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

   pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);

   const float mult  = (float)(1.0 / EDITOR_BG_WIDTH);
   const float ymult = (float)(1.0 / EDITOR_BG_HEIGHT);

   const float rect_left = min(m_d.m_v1.x, m_d.m_v2.x);
   const float rect_top = min(m_d.m_v1.y, m_d.m_v2.y);
   const float rect_right = max(m_d.m_v1.x, m_d.m_v2.x);
   const float rect_bottom = max(m_d.m_v1.y, m_d.m_v2.y);

   const float x = rect_left*mult;
   const float y = rect_top*ymult;
   const float width = (rect_right - rect_left)*mult;
   const float height = (rect_bottom - rect_top)*ymult;

   if (dmd)
   {
      g_pplayer->m_pin3d.DisableAlphaBlend();
      g_pplayer->DMDdraw(x, y, width, height,
                         m_d.m_fontcolor, m_d.m_intensity_scale); //!! replace??!
   }
   else
      if (m_texture)
      {
         g_pplayer->m_pin3d.EnableAlphaTestReference(0x80);
         g_pplayer->m_pin3d.EnableAlphaBlend(false);

         g_pplayer->Spritedraw(x, y, width, height, 0xFFFFFFFF, pd3dDevice->m_texMan.LoadTexture(m_texture, false), m_d.m_intensity_scale);

         //g_pplayer->m_pin3d.DisableAlphaBlend(); //!! not necessary anymore
         pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, RenderDevice::RS_FALSE);
      }

   //if (m_ptable->m_tblMirrorEnabled^m_ptable->m_reflectionEnabled)
   //	pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);
}

void Textbox::RenderSetup()
{
   m_pIFont->Clone(&m_pIFontPlay);

   CY size;
   m_pIFontPlay->get_Size(&size);
   size.int64 = (LONGLONG)(size.int64 / 1.5 * g_pplayer->m_height * g_pplayer->m_width);
   m_pIFontPlay->put_Size(size);

   PreRenderText();
}

void Textbox::RenderStatic()
{
}

void Textbox::PreRenderText()
{
   RECT rect;
   rect.left = (int)min(m_d.m_v1.x, m_d.m_v2.x);
   rect.top = (int)min(m_d.m_v1.y, m_d.m_v2.y);
   rect.right = (int)max(m_d.m_v1.x, m_d.m_v2.x);
   rect.bottom = (int)max(m_d.m_v1.y, m_d.m_v2.y);

   const int width = rect.right - rect.left;
   const int height = rect.bottom - rect.top;

   BITMAPINFO bmi;
   ZeroMemory(&bmi, sizeof(bmi));
   bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth = width;
   bmi.bmiHeader.biHeight = -height;
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biBitCount = 32;
   bmi.bmiHeader.biCompression = BI_RGB;
   bmi.bmiHeader.biSizeImage = 0;

   void *bits;
   const HBITMAP hbm = CreateDIBSection(0, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
   assert(hbm);

   const HDC hdc = CreateCompatibleDC(NULL);
   const HBITMAP oldBmp = (HBITMAP)SelectObject(hdc, hbm);

   const HBRUSH hbrush = CreateSolidBrush(m_d.m_backcolor);
   const HBRUSH hbrushold = (HBRUSH)SelectObject(hdc, hbrush);
   PatBlt(hdc, 0, 0, width, height, PATCOPY);
   SelectObject(hdc, hbrushold);
   DeleteObject(hbrush);

   HFONT hFont;
   m_pIFontPlay->get_hFont(&hFont);
   HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
   SetTextColor(hdc, m_d.m_fontcolor);
   SetBkMode(hdc, TRANSPARENT);
   SetTextAlign(hdc, TA_LEFT | TA_TOP | TA_NOUPDATECP);

   int alignment;
   switch (m_d.m_talign)
   {
   case TextAlignLeft:
      alignment = DT_LEFT;
      break;

   default:
   case TextAlignCenter:
      alignment = DT_CENTER;
      break;

   case TextAlignRight:
      alignment = DT_RIGHT;
      break;
   }

   const int border = (4 * g_pplayer->m_width) / EDITOR_BG_WIDTH;
   RECT rcOut;
   rcOut.left = border;
   rcOut.top = border;
   rcOut.right = width - border * 2;
   rcOut.bottom = height - border * 2;

   DrawText(hdc, m_d.m_sztext.c_str(), (int)m_d.m_sztext.length(), &rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK);

   GdiFlush();     // make sure everything is drawn

   if (!m_texture)
      m_texture = new BaseTexture(width, height, BaseTexture::RGBA, m_d.m_transparent);

   // Set alpha for pixels that match transparent color (if transparent enabled), otherwise set to opaque
   D3DCOLOR* __restrict bitsd = (D3DCOLOR*)bits;
   D3DCOLOR* __restrict dest = (D3DCOLOR*)m_texture->data();
   for (int i = 0; i < m_texture->height(); i++)
   {
      for (int l = 0; l < m_texture->width(); l++, dest++, bitsd++)
      {
         const D3DCOLOR src = *bitsd;
         if (m_d.m_transparent && ((src & 0xFFFFFFu) == m_d.m_backcolor))
            *dest = 0x00000000; // set to black & alpha full transparent
         else
            *dest = src | 0xFF000000u;
      }
      dest += m_texture->pitch()/4 - m_texture->width();
   }

   g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_texMan.SetDirty(m_texture);

   SelectObject(hdc, oldFont);
   SelectObject(hdc, oldBmp);
   DeleteDC(hdc);
   DeleteObject(hbm);
}

void Textbox::SetObjectPos()
{
    m_vpinball->SetObjectPosCur(m_d.m_v1.x, m_d.m_v1.y);
}

void Textbox::MoveOffset(const float dx, const float dy)
{
   m_d.m_v1.x += dx;
   m_d.m_v1.y += dy;

   m_d.m_v2.x += dx;
   m_d.m_v2.y += dy;
}

void Textbox::PutCenter(const Vertex2D& pv)
{
   m_d.m_v2.x = pv.x + m_d.m_v2.x - m_d.m_v1.x;
   m_d.m_v2.y = pv.y + m_d.m_v2.y - m_d.m_v1.y;

   m_d.m_v1 = pv;
}

STDMETHODIMP Textbox::get_BackColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_backcolor;

   return S_OK;
}

STDMETHODIMP Textbox::put_BackColor(OLE_COLOR newVal)
{
   m_d.m_backcolor = newVal;

   return S_OK;
}

STDMETHODIMP Textbox::get_FontColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_fontcolor;

   return S_OK;
}

STDMETHODIMP Textbox::put_FontColor(OLE_COLOR newVal)
{
   m_d.m_fontcolor = newVal;

   return S_OK;
}

STDMETHODIMP Textbox::get_Text(BSTR *pVal)
{
   WCHAR wz[MAXSTRING];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_sztext.c_str(), -1, wz, MAXSTRING);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Textbox::put_Text(BSTR newVal)
{
   char buf[MAXSTRING];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXSTRING, NULL, NULL);
   m_d.m_sztext = buf;
   if (g_pplayer)
      PreRenderText();

   return S_OK;
}

HRESULT Textbox::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool backupForPlay)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteStruct(FID(VER1), &m_d.m_v1, sizeof(Vertex2D));
   bw.WriteStruct(FID(VER2), &m_d.m_v2, sizeof(Vertex2D));
   bw.WriteInt(FID(CLRB), m_d.m_backcolor);
   bw.WriteInt(FID(CLRF), m_d.m_fontcolor);
   bw.WriteFloat(FID(INSC), m_d.m_intensity_scale);
   bw.WriteString(FID(TEXT), m_d.m_sztext);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteInt(FID(ALGN), m_d.m_talign);
   bw.WriteBool(FID(TRNS), m_d.m_transparent);
   bw.WriteBool(FID(IDMD), m_d.m_isDMD);

   ISelect::SaveData(pstm, hcrypthash);

   bw.WriteTag(FID(FONT));
   IPersistStream * ips;
   m_pIFont->QueryInterface(IID_IPersistStream, (void **)&ips);
   HRESULT hr;
   hr = ips->Save(pstm, TRUE);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Textbox::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

bool Textbox::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(VER1): pbr->GetStruct(&m_d.m_v1, sizeof(Vertex2D)); break;
   case FID(VER2): pbr->GetStruct(&m_d.m_v2, sizeof(Vertex2D)); break;
   case FID(CLRB): pbr->GetInt(&m_d.m_backcolor); break;
   case FID(CLRF): pbr->GetInt(&m_d.m_fontcolor); break;
   case FID(INSC): pbr->GetFloat(&m_d.m_intensity_scale); break;
   case FID(TMON): pbr->GetBool(&m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(&m_d.m_tdr.m_TimerInterval); break;
   case FID(TEXT): pbr->GetString(m_d.m_sztext); break;
   case FID(NAME): pbr->GetWideString(m_wzName); break;
   case FID(ALGN): pbr->GetInt(&m_d.m_talign); break;
   case FID(TRNS): pbr->GetBool(&m_d.m_transparent); break;
   case FID(IDMD): pbr->GetBool(&m_d.m_isDMD); break;
   case FID(FONT):
   {
      if (!m_pIFont)
      {
         FONTDESC fd;
         fd.cbSizeofstruct = sizeof(FONTDESC);
         fd.lpstrName = L"Arial";
         fd.cySize.int64 = 142500;
         //fd.cySize.Lo = 0;
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

HRESULT Textbox::InitPostLoad()
{
   m_texture = NULL;

   return S_OK;
}

STDMETHODIMP Textbox::get_Font(IFontDisp **pVal)
{
   m_pIFont->QueryInterface(IID_IFontDisp, (void **)pVal);

   return S_OK;
}

STDMETHODIMP Textbox::put_Font(IFontDisp *newVal)
{
   // Does anybody use this way of setting the font?  Need to add to idl file.
   return S_OK;
}

STDMETHODIMP Textbox::putref_Font(IFontDisp* pFont)
{
   //We know that our own property browser gives us the same pointer

   //m_pIFont->Release();
   //pFont->QueryInterface(IID_IFont, (void **)&m_pIFont);

   SetDirtyDraw();

   return S_OK;
}

STDMETHODIMP Textbox::get_Width(float *pVal)
{
   *pVal = m_d.m_v2.x - m_d.m_v1.x;

   return S_OK;
}

STDMETHODIMP Textbox::put_Width(float newVal)
{
   m_d.m_v2.x = m_d.m_v1.x + newVal;

   return S_OK;
}

STDMETHODIMP Textbox::get_Height(float *pVal)
{
   *pVal = m_d.m_v2.y - m_d.m_v1.y;

   return S_OK;
}

STDMETHODIMP Textbox::put_Height(float newVal)
{
   m_d.m_v2.y = m_d.m_v1.y + newVal;

   return S_OK;
}

STDMETHODIMP Textbox::get_X(float *pVal)
{
   *pVal = m_d.m_v1.x;
   m_vpinball->SetStatusBarUnitInfo("", true);

   return S_OK;
}

STDMETHODIMP Textbox::put_X(float newVal)
{
   const float delta = newVal - m_d.m_v1.x;
   m_d.m_v1.x += delta;
   m_d.m_v2.x += delta;

   return S_OK;
}

STDMETHODIMP Textbox::get_Y(float *pVal)
{
   *pVal = m_d.m_v1.y;

   return S_OK;
}

STDMETHODIMP Textbox::put_Y(float newVal)
{
   const float delta = newVal - m_d.m_v1.y;
   m_d.m_v1.y += delta;
   m_d.m_v2.y += delta;

   return S_OK;
}

STDMETHODIMP Textbox::get_IntensityScale(float *pVal)
{
   *pVal = m_d.m_intensity_scale;

   return S_OK;
}

STDMETHODIMP Textbox::put_IntensityScale(float newVal)
{
   m_d.m_intensity_scale = newVal;

   return S_OK;
}

STDMETHODIMP Textbox::get_Alignment(TextAlignment *pVal)
{
   *pVal = m_d.m_talign;

   return S_OK;
}

STDMETHODIMP Textbox::put_Alignment(TextAlignment newVal)
{
   m_d.m_talign = newVal;

   return S_OK;
}

STDMETHODIMP Textbox::get_IsTransparent(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_transparent);

   return S_OK;
}

STDMETHODIMP Textbox::put_IsTransparent(VARIANT_BOOL newVal)
{
   m_d.m_transparent = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Textbox::get_DMD(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_isDMD);

   return S_OK;
}

STDMETHODIMP Textbox::put_DMD(VARIANT_BOOL newVal)
{
   m_d.m_isDMD = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Textbox::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_visible);

   return S_OK;
}

STDMETHODIMP Textbox::put_Visible(VARIANT_BOOL newVal)
{
   m_d.m_visible = VBTOb(newVal);

   return S_OK;
}
