#include "stdafx.h"
#include "Shader.h"

Textbox::Textbox()
{
   m_ptable = nullptr;
   m_texture = nullptr;
   m_pIFont = nullptr;
   m_pIFontPlay = nullptr;
}

Textbox::~Textbox()
{
   m_pIFont->Release();
}

HRESULT Textbox::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   const float width  = LoadValueFloatWithDefault(regKey[RegName::DefaultPropsTextBox], "Width"s, 100.0f);
   const float height = LoadValueFloatWithDefault(regKey[RegName::DefaultPropsTextBox], "Height"s, 50.0f);

   m_d.m_v1.x = x;
   m_d.m_v1.y = y;
   m_d.m_v2.x = x + width;
   m_d.m_v2.y = y + height;

   SetDefaults(fromMouseClick);

   return InitVBA(fTrue, 0, nullptr);//ApcProjectItem.Define(ptable->ApcProject, GetDispatch(), axTypeHostProjectItem/*axTypeHostClass*/, L"Textbox", nullptr);
}

void Textbox::SetDefaults(bool fromMouseClick)
{
#define regKey regKey[RegName::DefaultPropsTextBox]

   //Textbox is always located on backdrop
   m_backglass = true;
   m_d.m_visible = true;

#ifndef __STANDALONE__
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
      fd.lpstrName = (LPOLESTR)(L"Arial");
      fd.sWeight = FW_NORMAL;
      fd.sCharset = 0;
      fd.fItalic = 0;
      fd.fUnderline = 0;
      fd.fStrikethrough = 0;
   }
   else
   {
      m_d.m_backcolor = LoadValueIntWithDefault(regKey, "BackColor"s, RGB(0, 0, 0));
      m_d.m_fontcolor = LoadValueIntWithDefault(regKey, "FontColor"s, RGB(255, 255, 255));
      m_d.m_intensity_scale = LoadValueFloatWithDefault(regKey, "IntensityScale"s, 1.0f);
      m_d.m_tdr.m_TimerEnabled = LoadValueBoolWithDefault(regKey, "TimerEnabled"s, false) ? true : false;
      m_d.m_tdr.m_TimerInterval = LoadValueIntWithDefault(regKey, "TimerInterval"s, 100);
      m_d.m_talign = (TextAlignment)LoadValueIntWithDefault(regKey, "TextAlignment"s, TextAlignRight);
      m_d.m_transparent = LoadValueBoolWithDefault(regKey, "Transparent"s, false);
      m_d.m_isDMD = LoadValueBoolWithDefault(regKey, "DMD"s, false);

      m_d.m_fontsize = LoadValueFloatWithDefault(regKey, "FontSize"s, 14.25f);
      fd.cySize.int64 = (LONGLONG)(m_d.m_fontsize * 10000.0f);

      string tmp;
      HRESULT hr;
      hr = LoadValue(regKey, "FontName"s, tmp);
      if (hr != S_OK)
         fd.lpstrName = (LPOLESTR)(L"Arial");
      else
      {
         const int len = (int)tmp.length() + 1;
         fd.lpstrName = (LPOLESTR)malloc(len*sizeof(WCHAR));
         memset(fd.lpstrName, 0, len*sizeof(WCHAR));
         MultiByteToWideCharNull(CP_ACP, 0, tmp.c_str(), -1, fd.lpstrName, len);

         free_lpstrName = true;
      }

      fd.sWeight = LoadValueIntWithDefault(regKey, "FontWeight"s, FW_NORMAL);
      fd.sCharset = LoadValueIntWithDefault(regKey, "FontCharSet"s, 0);
      fd.fItalic = LoadValueIntWithDefault(regKey, "FontItalic"s, 0);
      fd.fUnderline = LoadValueIntWithDefault(regKey, "FontUnderline"s, 0);
      fd.fStrikethrough = LoadValueIntWithDefault(regKey, "FontStrikeThrough"s, 0);

      hr = LoadValue(regKey, "Text"s, m_d.m_sztext);
      if (hr != S_OK)
         m_d.m_sztext.clear();
   }

   if (m_pIFont)
      m_pIFont->Release();
   OleCreateFontIndirect(&fd, IID_IFont, (void **)&m_pIFont);
   if (free_lpstrName)
      free(fd.lpstrName);

#endif
#undef regKey
}

void Textbox::WriteRegDefaults()
{
#define regKey regKey[RegName::DefaultPropsTextBox]

   SaveValueInt(regKey, "BackColor"s, m_d.m_backcolor);
   SaveValueInt(regKey, "FontColor"s, m_d.m_fontcolor);
   SaveValueBool(regKey, "TimerEnabled"s, m_d.m_tdr.m_TimerEnabled);
   SaveValueInt(regKey, "TimerInterval"s, m_d.m_tdr.m_TimerInterval);
   SaveValueBool(regKey, "Transparent"s, m_d.m_transparent);
   SaveValueBool(regKey, "DMD"s, m_d.m_isDMD);

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

   SaveValue(regKey, "Text"s, m_d.m_sztext);

#undef regKey
#endif
}

char * Textbox::GetFontName()
{
    if (m_pIFont)
    {
        CComBSTR bstr;
        /*HRESULT hr =*/ m_pIFont->get_Name(&bstr);

        static char fontName[LF_FACESIZE];
        WideCharToMultiByteNull(CP_ACP, 0, bstr, -1, fontName, LF_FACESIZE, nullptr, nullptr);
        return fontName;
    }
    return nullptr;
}

HFONT Textbox::GetFont()
{
#ifndef __STANDALONE__
    LOGFONT lf = {};
    lf.lfHeight = -MulDiv((int)m_d.m_fontsize, GetDeviceCaps(g_pvp->GetDC(), LOGPIXELSY), 72);
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfQuality = NONANTIALIASED_QUALITY;

    CComBSTR bstr;
    HRESULT hr = m_pIFont->get_Name(&bstr);

    WideCharToMultiByteNull(CP_ACP, 0, bstr, -1, lf.lfFaceName, LF_FACESIZE, nullptr, nullptr);

    BOOL bl;
    hr = m_pIFont->get_Bold(&bl);

    lf.lfWeight = bl ? FW_BOLD : FW_NORMAL;

    hr = m_pIFont->get_Italic(&bl);

    lf.lfItalic = (BYTE)bl;

    const HFONT hFont = CreateFontIndirect(&lf);
    return hFont;
#else
   return 0L;
#endif
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
   psur->SetObject(nullptr);

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
      m_texture = nullptr;

      m_pIFontPlay->Release();
   }

   IEditable::EndPlay();
}

void Textbox::RenderDynamic()
{
   TRACE_FUNCTION();

   const bool dmd = m_d.m_isDMD || StrStrI(m_d.m_sztext.c_str(), "DMD") != nullptr; //!! second part is VP10.0 legacy

   if (!m_d.m_visible || (dmd && !g_pplayer->m_texdmd) || (m_backglass && m_ptable->m_reflectionEnabled))
      return;

   RenderDevice * const pd3dDevice = m_backglass ? g_pplayer->m_pin3d.m_pd3dSecondaryDevice : g_pplayer->m_pin3d.m_pd3dPrimaryDevice;
   RenderDevice::RenderStateCache initial_state;
   pd3dDevice->CopyRenderStates(true, initial_state);
   if (m_ptable->m_tblMirrorEnabled^m_ptable->m_reflectionEnabled)
      pd3dDevice->SetRenderStateCulling(RenderDevice::CULL_NONE);
   else
      pd3dDevice->SetRenderStateCulling(RenderDevice::CULL_CCW);
   pd3dDevice->SetRenderStateDepthBias(0.0f);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);

   constexpr float mult  = (float)(1.0 / EDITOR_BG_WIDTH);
   constexpr float ymult = (float)(1.0 / EDITOR_BG_HEIGHT);

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
      pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, RenderDevice::RS_FALSE);
      g_pplayer->DMDdraw(x, y, width, height, m_d.m_fontcolor, m_d.m_intensity_scale); //!! replace??!
   }
   else if (m_texture)
   {
      pd3dDevice->DMDShader->SetFloat(SHADER_alphaTestValue, (float)(128.0 / 255.0));
      g_pplayer->Spritedraw(x, y, width, height, 0xFFFFFFFF, pd3dDevice->m_texMan.LoadTexture(m_texture, SF_TRILINEAR, SA_REPEAT, SA_REPEAT, false), m_d.m_intensity_scale);
      pd3dDevice->DMDShader->SetFloat(SHADER_alphaTestValue, 1.0f);
   }

   pd3dDevice->CopyRenderStates(false, initial_state);
}

void Textbox::RenderSetup()
{
#ifndef __STANDALONE__
   m_pIFont->Clone(&m_pIFontPlay);

   CY size;
   m_pIFontPlay->get_Size(&size);
   size.int64 = (LONGLONG)(size.int64 / 1.5 * g_pplayer->m_wnd_height * g_pplayer->m_wnd_width);
   m_pIFontPlay->put_Size(size);
#endif

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

#ifndef __STANDALONE__
   BITMAPINFO bmi = {};
   bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth = width;
   bmi.bmiHeader.biHeight = -height;
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biBitCount = 32;
   bmi.bmiHeader.biCompression = BI_RGB;
   bmi.bmiHeader.biSizeImage = 0;

   void *bits;
   const HBITMAP hbm = CreateDIBSection(0, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
   assert(hbm);

   const HDC hdc = CreateCompatibleDC(nullptr);
   const HBITMAP oldBmp = (HBITMAP)SelectObject(hdc, hbm);

   const HBRUSH hbrush = CreateSolidBrush(m_d.m_backcolor);
   const HBRUSH hbrushold = (HBRUSH)SelectObject(hdc, hbrush);
   PatBlt(hdc, 0, 0, width, height, PATCOPY);
   SelectObject(hdc, hbrushold);
   DeleteObject(hbrush);

   HFONT hFont;
   m_pIFontPlay->get_hFont(&hFont);
   const HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
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

   const int border = (4 * g_pplayer->m_wnd_width) / EDITOR_BG_WIDTH;
   RECT rcOut;
   rcOut.left = border;
   rcOut.top = border;
   rcOut.right = width - border * 2;
   rcOut.bottom = height - border * 2;

   DrawText(hdc, m_d.m_sztext.c_str(), (int)m_d.m_sztext.length(), &rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK);

   GdiFlush();     // make sure everything is drawn

   if (!m_texture)
      m_texture = new BaseTexture(width, height, BaseTexture::SRGBA); // This could be optimized to an RGB texture if transparent is not set

   // Set alpha for pixels that match transparent color (if transparent enabled), otherwise set to opaque
   const D3DCOLOR* __restrict bitsd = (D3DCOLOR*)bits;
         D3DCOLOR* __restrict dest = (D3DCOLOR*)m_texture->data();
   for (unsigned int i = 0; i < m_texture->height(); i++)
   {
      for (unsigned int l = 0; l < m_texture->width(); l++, dest++, bitsd++)
      {
		  const D3DCOLOR src = *bitsd;
		  if (m_d.m_transparent && ((src & 0xFFFFFFu) == m_d.m_backcolor))
			  *dest = 0x00000000; // set to black & alpha full transparent
		  else
			  *dest = ((src & 0x000000FFu) << 16)
			  | (src & 0x0000FF00u)
			  | ((src & 0x0000FF0000u) >> 16)
			  | 0xFF000000u;
      }
      dest += m_texture->pitch()/4 - m_texture->width();
   }

   g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_texMan.SetDirty(m_texture);

   SelectObject(hdc, oldFont);
   SelectObject(hdc, oldBmp);
   DeleteDC(hdc);
   DeleteObject(hbm);
#endif
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
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXSTRING, nullptr, nullptr);
   m_d.m_sztext = buf;
   if (g_pplayer)
      PreRenderText();

   return S_OK;
}

HRESULT Textbox::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool backupForPlay)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteVector2(FID(VER1), m_d.m_v1);
   bw.WriteVector2(FID(VER2), m_d.m_v2);
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
#ifndef __STANDALONE__
   IPersistStream * ips;
   m_pIFont->QueryInterface(IID_IPersistStream, (void **)&ips);
   HRESULT hr;
   hr = ips->Save(pstm, TRUE);
#endif

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
   case FID(VER1): pbr->GetVector2(m_d.m_v1); break;
   case FID(VER2): pbr->GetVector2(m_d.m_v2); break;
   case FID(CLRB): pbr->GetInt(m_d.m_backcolor); break;
   case FID(CLRF): pbr->GetInt(m_d.m_fontcolor); break;
   case FID(INSC): pbr->GetFloat(m_d.m_intensity_scale); break;
   case FID(TMON): pbr->GetBool(m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(m_d.m_tdr.m_TimerInterval); break;
   case FID(TEXT): pbr->GetString(m_d.m_sztext); break;
   case FID(NAME): pbr->GetWideString(m_wzName,sizeof(m_wzName)/sizeof(m_wzName[0])); break;
   case FID(ALGN): pbr->GetInt(&m_d.m_talign); break;
   case FID(TRNS): pbr->GetBool(m_d.m_transparent); break;
   case FID(IDMD): pbr->GetBool(m_d.m_isDMD); break;
   case FID(FONT):
   {
#ifndef __STANDALONE__
      if (!m_pIFont)
      {
         FONTDESC fd;
         fd.cbSizeofstruct = sizeof(FONTDESC);
         fd.lpstrName = (LPOLESTR)(L"Arial");
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

HRESULT Textbox::InitPostLoad()
{
   m_texture = nullptr;

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
   m_vpinball->SetStatusBarUnitInfo(string(), true);

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
