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

   const float width = GetRegStringAsFloatWithDefault("DefaultProps\\TextBox", "Width", 100.0f);
   const float height = GetRegStringAsFloatWithDefault("DefaultProps\\TextBox", "Height", 50.0f);

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
   m_fBackglass = true;
   m_d.m_fVisible = true;

   FONTDESC fd;
   fd.cbSizeofstruct = sizeof(FONTDESC);
   bool free_lpstrName = false;

   if (!fromMouseClick)
   {
      m_d.m_backcolor = RGB(0, 0, 0);
      m_d.m_fontcolor = RGB(255, 255, 255);
      m_d.m_intensity_scale = 1.0f;
      m_d.m_tdr.m_fTimerEnabled = false;
      m_d.m_tdr.m_TimerInterval = 100;
      m_d.m_talign = TextAlignRight;
      m_d.m_fTransparent = false;
	  m_d.m_IsDMD = false;
	  lstrcpy(m_d.sztext, "0");

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
      m_d.m_backcolor = GetRegIntWithDefault("DefaultProps\\TextBox", "BackColor", RGB(0, 0, 0));
      m_d.m_fontcolor = GetRegIntWithDefault("DefaultProps\\TextBox", "FontColor", RGB(255, 255, 255));
      m_d.m_intensity_scale = GetRegStringAsFloatWithDefault("DefaultProps\\TextBox", "IntensityScale", 1.0f);
      m_d.m_tdr.m_fTimerEnabled = GetRegBoolWithDefault("DefaultProps\\TextBox", "TimerEnabled", false) ? true : false;
      m_d.m_tdr.m_TimerInterval = GetRegIntWithDefault("DefaultProps\\TextBox", "TimerInterval", 100);
      m_d.m_talign = (TextAlignment)GetRegIntWithDefault("DefaultProps\\TextBox", "TextAlignment", TextAlignRight);
      m_d.m_fTransparent = GetRegBoolWithDefault("DefaultProps\\TextBox", "Transparent", false);
	  m_d.m_IsDMD = GetRegBoolWithDefault("DefaultProps\\TextBox", "DMD", false);

      const float fontSize = GetRegStringAsFloatWithDefault("DefaultProps\\TextBox", "FontSize", 14.25f);
      fd.cySize.int64 = (LONGLONG)(fontSize * 10000.0f);

      char tmp[256];
      HRESULT hr;
      hr = GetRegString("DefaultProps\\TextBox", "FontName", tmp, 256);
      if (hr != S_OK)
         fd.lpstrName = L"Arial";
      else
      {
         int len = lstrlen(tmp) + 1;
         fd.lpstrName = (LPOLESTR)malloc(len*sizeof(WCHAR));
         memset(fd.lpstrName, 0, len*sizeof(WCHAR));
         UNICODE_FROM_ANSI(fd.lpstrName, tmp, len);
         free_lpstrName = true;
      }

      fd.sWeight = GetRegIntWithDefault("DefaultProps\\TextBox", "FontWeight", FW_NORMAL);
      fd.sCharset = GetRegIntWithDefault("DefaultProps\\TextBox", "FontCharSet", 0);
      fd.fItalic = GetRegIntWithDefault("DefaultProps\\TextBox", "FontItalic", 0);
      fd.fUnderline = GetRegIntWithDefault("DefaultProps\\TextBox", "FontUnderline", 0);
      fd.fStrikethrough = GetRegIntWithDefault("DefaultProps\\TextBox", "FontStrikeThrough", 0);

      hr = GetRegString("DefaultProps\\TextBox", "Text", m_d.sztext, MAXSTRING);
      if (hr != S_OK)
         lstrcpy(m_d.sztext, "0");
   }

   OleCreateFontIndirect(&fd, IID_IFont, (void **)&m_pIFont);
   if (free_lpstrName)
      free(fd.lpstrName);
}

void Textbox::WriteRegDefaults()
{
   char strTmp[128];

   SetRegValue("DefaultProps\\TextBox", "BackColor", REG_DWORD, &m_d.m_backcolor, 4);
   SetRegValue("DefaultProps\\TextBox", "FontColor", REG_DWORD, &m_d.m_fontcolor, 4);
   SetRegValueBool("DefaultProps\\TextBox", "TimerEnabled", m_d.m_tdr.m_fTimerEnabled);
   SetRegValue("DefaultProps\\TextBox", "TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
   SetRegValueBool("DefaultProps\\TextBox", "Transparent", m_d.m_fTransparent);
   SetRegValueBool("DefaultProps\\TextBox", "DMD", m_d.m_IsDMD);

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
   SetRegValueFloat("DefaultProps\\TextBox", "FontSize", fTmp);
   size_t charCnt = wcslen(fd.lpstrName) + 1;
   WideCharToMultiByte(CP_ACP, 0, fd.lpstrName, (int)charCnt, strTmp, (int)(2 * charCnt), NULL, NULL);
   SetRegValue("DefaultProps\\TextBox", "FontName", REG_SZ, &strTmp, lstrlen(strTmp));
   int weight = fd.sWeight;
   int charset = fd.sCharset;
   SetRegValueInt("DefaultProps\\TextBox", "FontWeight", weight);
   SetRegValueInt("DefaultProps\\TextBox", "FontCharSet", charset);
   SetRegValue("DefaultProps\\TextBox", "FontItalic", REG_DWORD, &fd.fItalic, 4);
   SetRegValue("DefaultProps\\TextBox", "FontUnderline", REG_DWORD, &fd.fUnderline, 4);
   SetRegValue("DefaultProps\\TextBox", "FontStrikeThrough", REG_DWORD, &fd.fStrikethrough, 4);

   SetRegValue("DefaultProps\\TextBox", "Text", REG_SZ, &m_d.sztext, lstrlen(m_d.sztext));
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

void Textbox::PreRender(Sur * const psur)
{
   psur->SetBorderColor(-1, false, 0);
   psur->SetFillColor(m_d.m_backcolor);
   psur->SetObject(this);

   psur->Rectangle(m_d.m_v1.x, m_d.m_v1.y, m_d.m_v2.x, m_d.m_v2.y);
}

void Textbox::Render(Sur * const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(this);
   psur->SetObject(NULL);

   psur->Rectangle(m_d.m_v1.x, m_d.m_v1.y, m_d.m_v2.x, m_d.m_v2.y);
}

void Textbox::GetTimers(Vector<HitTimer> * const pvht)
{
   IEditable::BeginPlay();

   HitTimer * const pht = new HitTimer();
   pht->m_interval = m_d.m_tdr.m_TimerInterval >= 0 ? max(m_d.m_tdr.m_TimerInterval, MAX_TIMER_MSEC_INTERVAL) : -1;
   pht->m_nextfire = pht->m_interval;
   pht->m_pfe = (IFireEvents *)this;

   m_phittimer = pht;

   if (m_d.m_tdr.m_fTimerEnabled)
   {
      pvht->AddElement(pht);
   }
}

void Textbox::GetHitShapes(Vector<HitObject> * const pvho)
{
}

void Textbox::GetHitShapesDebug(Vector<HitObject> * const pvho)
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

void Textbox::PostRenderStatic(RenderDevice* pd3dDevice)
{
   TRACE_FUNCTION();

   const bool dmd = (m_d.m_IsDMD || strstr(m_d.sztext, "DMD") != NULL); //!! second part is VP10.0 legacy

   if (!m_d.m_fVisible || (dmd && !g_pplayer->m_texdmd))
      return;

   if (g_pplayer->m_ptable->m_tblMirrorEnabled^g_pplayer->m_ptable->m_fReflectionEnabled)
      pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
   else
      pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

   pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);

   const float mult = (float)(1.0 / EDITOR_BG_WIDTH);
   const float ymult = (float)(1.0 / EDITOR_BG_WIDTH * 4.0 / 3.0);

   const float x = (float)m_rect.left*mult;
   const float y = (float)m_rect.top*ymult;
   const float width = (float)(m_rect.right - m_rect.left)*mult;
   const float height = (float)(m_rect.bottom - m_rect.top)*ymult;

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

         g_pplayer->Spritedraw(x, y, width, height, 0xFFFFFFFF, pd3dDevice->m_texMan.LoadTexture(m_texture), m_d.m_intensity_scale);

         //g_pplayer->m_pin3d.DisableAlphaBlend(); //!! not necessary anymore
         pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, FALSE);
      }

   //if(g_pplayer->m_ptable->m_tblMirrorEnabled^g_pplayer->m_ptable->m_fReflectionEnabled)
   //	pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
}

void Textbox::RenderSetup(RenderDevice* pd3dDevice)
{
   const float left = min(m_d.m_v1.x, m_d.m_v2.x);
   const float right = max(m_d.m_v1.x, m_d.m_v2.x);
   const float top = min(m_d.m_v1.y, m_d.m_v2.y);
   const float bottom = max(m_d.m_v1.y, m_d.m_v2.y);

   m_rect.left = (int)left;
   m_rect.top = (int)top;
   m_rect.right = (int)right;
   m_rect.bottom = (int)bottom;

   m_pIFont->Clone(&m_pIFontPlay);

   CY size;
   m_pIFontPlay->get_Size(&size);
   size.int64 = (LONGLONG)(size.int64 / 1.5 * g_pplayer->m_height * g_pplayer->m_width);
   m_pIFontPlay->put_Size(size);

   RenderText();
}

void Textbox::RenderStatic(RenderDevice* pd3dDevice)
{
}

void Textbox::RenderText()
{
   const int width = m_rect.right - m_rect.left;
   const int height = m_rect.bottom - m_rect.top;

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
   HBITMAP hbm = CreateDIBSection(0, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
   assert(hbm);

   HDC hdc = CreateCompatibleDC(NULL);
   HBITMAP oldBmp = (HBITMAP)SelectObject(hdc, hbm);

   HBRUSH hbrush = CreateSolidBrush(m_d.m_backcolor);
   HBRUSH hbrushold = (HBRUSH)SelectObject(hdc, hbrush);
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

   DrawText(hdc, m_d.sztext, lstrlen(m_d.sztext), &rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK);

   GdiFlush();     // make sure everything is drawn

   if (!m_texture)
      m_texture = new BaseTexture(m_rect.right - m_rect.left, m_rect.bottom - m_rect.top);
   m_texture->CopyFrom_Raw(bits);

   // Set alpha for pixels that match transparent color (if transparent enabled), otherwise set to opaque
   BYTE *pch = m_texture->data();
   for (int i = 0; i < m_texture->height(); i++)
   {
      for (int l = 0; l < m_texture->width(); l++)
      {
         if (m_d.m_fTransparent && (((*(D3DCOLOR *)pch) & 0xFFFFFF) == m_d.m_backcolor))
            *(D3DCOLOR *)pch = 0x00000000; // set to black & alpha full transparent
         else
            *(D3DCOLOR *)pch |= 0xFF000000;
         pch += 4;
      }
      pch += m_texture->pitch() - m_texture->width() * 4;
   }

   g_pplayer->m_pin3d.m_pd3dDevice->m_texMan.SetDirty(m_texture);

   SelectObject(hdc, oldFont);
   SelectObject(hdc, oldBmp);
   DeleteDC(hdc);
   DeleteObject(hbm);
}

void Textbox::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_v1.x, m_d.m_v1.y);
}

void Textbox::MoveOffset(const float dx, const float dy)
{
   m_d.m_v1.x += dx;
   m_d.m_v1.y += dy;

   m_d.m_v2.x += dx;
   m_d.m_v2.y += dy;

   m_ptable->SetDirtyDraw();
}

void Textbox::GetCenter(Vertex2D * const pv) const
{
   *pv = m_d.m_v1;
}

void Textbox::PutCenter(const Vertex2D * const pv)
{
   m_d.m_v2.x = pv->x + m_d.m_v2.x - m_d.m_v1.x;
   m_d.m_v2.y = pv->y + m_d.m_v2.y - m_d.m_v1.y;

   m_d.m_v1 = *pv;

   m_ptable->SetDirtyDraw();
}

STDMETHODIMP Textbox::get_BackColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_backcolor;

   return S_OK;
}

STDMETHODIMP Textbox::put_BackColor(OLE_COLOR newVal)
{
   STARTUNDO

      m_d.m_backcolor = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Textbox::get_FontColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_fontcolor;

   return S_OK;
}

STDMETHODIMP Textbox::put_FontColor(OLE_COLOR newVal)
{
   STARTUNDO

      m_d.m_fontcolor = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Textbox::get_Text(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, (char *)m_d.sztext, -1, wz, 512);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Textbox::put_Text(BSTR newVal)
{
   if (lstrlenW(newVal) < 512)
   {
      STARTUNDO

         WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.sztext, 512, NULL, NULL);

      if (g_pplayer)
         RenderText();

      STOPUNDO
   }

   return S_OK;
}

HRESULT Textbox::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteStruct(FID(VER1), &m_d.m_v1, sizeof(Vertex2D));
   bw.WriteStruct(FID(VER2), &m_d.m_v2, sizeof(Vertex2D));
   bw.WriteInt(FID(CLRB), m_d.m_backcolor);
   bw.WriteInt(FID(CLRF), m_d.m_fontcolor);
   bw.WriteFloat(FID(INSC), m_d.m_intensity_scale);
   bw.WriteString(FID(TEXT), m_d.sztext);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteInt(FID(ALGN), m_d.m_talign);
   bw.WriteBool(FID(TRNS), m_d.m_fTransparent);
   bw.WriteBool(FID(IDMD), m_d.m_IsDMD);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

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

BOOL Textbox::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(VER1))
   {
      pbr->GetStruct(&m_d.m_v1, sizeof(Vertex2D));
   }
   else if (id == FID(VER2))
   {
      pbr->GetStruct(&m_d.m_v2, sizeof(Vertex2D));
   }
   else if (id == FID(CLRB))
   {
      pbr->GetInt(&m_d.m_backcolor);
   }
   else if (id == FID(CLRF))
   {
      pbr->GetInt(&m_d.m_fontcolor);
   }
   else if (id == FID(INSC))
   {
      pbr->GetFloat(&m_d.m_intensity_scale);
   }
   else if (id == FID(TMON))
   {
      pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
   }
   else if (id == FID(TMIN))
   {
      pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
   }
   else if (id == FID(TEXT))
   {
      pbr->GetString(m_d.sztext);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(ALGN))
   {
      pbr->GetInt(&m_d.m_talign);
   }
   else if (id == FID(TRNS))
   {
      pbr->GetBool(&m_d.m_fTransparent);
   }
   else if (id == FID(IDMD))
   {
	   pbr->GetBool(&m_d.m_IsDMD);
   }
   else if (id == FID(FONT))
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
   }
   else
   {
      ISelect::LoadToken(id, pbr);
   }

   return fTrue;
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
   STARTUNDO

      m_d.m_v2.x = m_d.m_v1.x + newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Textbox::get_Height(float *pVal)
{
   *pVal = m_d.m_v2.y - m_d.m_v1.y;

   return S_OK;
}

STDMETHODIMP Textbox::put_Height(float newVal)
{
   STARTUNDO

      m_d.m_v2.y = m_d.m_v1.y + newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Textbox::get_X(float *pVal)
{
   *pVal = m_d.m_v1.x;

   return S_OK;
}

STDMETHODIMP Textbox::put_X(float newVal)
{
   STARTUNDO

      float delta = newVal - m_d.m_v1.x;

   m_d.m_v1.x += delta;
   m_d.m_v2.x += delta;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Textbox::get_Y(float *pVal)
{
   *pVal = m_d.m_v1.y;

   return S_OK;
}

STDMETHODIMP Textbox::put_Y(float newVal)
{
   STARTUNDO

      float delta = newVal - m_d.m_v1.y;

   m_d.m_v1.y += delta;
   m_d.m_v2.y += delta;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Textbox::get_IntensityScale(float *pVal)
{
   *pVal = m_d.m_intensity_scale;

   return S_OK;
}

STDMETHODIMP Textbox::put_IntensityScale(float newVal)
{
   STARTUNDO

      m_d.m_intensity_scale = newVal;

   STOPUNDO

      return S_OK;
}

void Textbox::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPTEXTBOX_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPTEXTBOX_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPTEXTBOX_STATE, IDS_STATE);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}

STDMETHODIMP Textbox::get_Alignment(TextAlignment *pVal)
{
   *pVal = m_d.m_talign;

   return S_OK;
}

STDMETHODIMP Textbox::put_Alignment(TextAlignment newVal)
{
   STARTUNDO

      m_d.m_talign = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Textbox::get_IsTransparent(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fTransparent);

   return S_OK;
}

STDMETHODIMP Textbox::put_IsTransparent(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fTransparent = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Textbox::get_DMD(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_IsDMD);

	return S_OK;
}

STDMETHODIMP Textbox::put_DMD(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_IsDMD = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Textbox::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fVisible);

   return S_OK;
}

STDMETHODIMP Textbox::put_Visible(VARIANT_BOOL newVal)
{
   STARTUNDO
      m_d.m_fVisible = VBTOF(newVal);
   STOPUNDO

      return S_OK;
}
