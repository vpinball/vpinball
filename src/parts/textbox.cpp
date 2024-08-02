#include "stdafx.h"
#include "renderer/Shader.h"
#include "renderer/RenderCommand.h"
#include "captureExt.h"

Textbox::Textbox()
{
}

Textbox::~Textbox()
{
   assert(m_rd == nullptr);
   SAFE_RELEASE(m_pIFont);
}

Textbox *Textbox::CopyForPlay(PinTable *live_table)
{
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(Textbox, live_table)
   m_pIFont->Clone(&dst->m_pIFont);
   return dst;
}

HRESULT Textbox::Init(PinTable *const ptable, const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   m_ptable = ptable;
   SetDefaults(fromMouseClick);
   const float width  = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultPropsTextBox, "Width"s, 100.0f);
   const float height = g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultPropsTextBox, "Height"s, 50.0f);
   m_d.m_v1.x = x;
   m_d.m_v1.y = y;
   m_d.m_v2.x = x + width;
   m_d.m_v2.y = y + height;
   return forPlay ? S_OK : InitVBA(fTrue, 0, nullptr); //ApcProjectItem.Define(ptable->ApcProject, GetDispatch(), axTypeHostProjectItem/*axTypeHostClass*/, L"Textbox", nullptr);
}

void Textbox::SetDefaults(const bool fromMouseClick)
{
#define regKey Settings::DefaultPropsTextBox

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
      fd.lpstrName = (LPOLESTR)(L"Arial");
      fd.sWeight = FW_NORMAL;
      fd.sCharset = 0;
      fd.fItalic = 0;
      fd.fUnderline = 0;
      fd.fStrikethrough = 0;
   }
   else
   {
      m_d.m_backcolor = g_pvp->m_settings.LoadValueWithDefault(regKey, "BackColor"s, (int)RGB(0, 0, 0));
      m_d.m_fontcolor = g_pvp->m_settings.LoadValueWithDefault(regKey, "FontColor"s, (int)RGB(255, 255, 255));
      m_d.m_intensity_scale = g_pvp->m_settings.LoadValueWithDefault(regKey, "IntensityScale"s, 1.0f);
      m_d.m_tdr.m_TimerEnabled = g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerEnabled"s, false) ? true : false;
      m_d.m_tdr.m_TimerInterval = g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerInterval"s, 100);
      m_d.m_talign = (TextAlignment)g_pvp->m_settings.LoadValueWithDefault(regKey, "TextAlignment"s, (int)TextAlignRight);
      m_d.m_transparent = g_pvp->m_settings.LoadValueWithDefault(regKey, "Transparent"s, false);
      m_d.m_isDMD = g_pvp->m_settings.LoadValueWithDefault(regKey, "DMD"s, false);

      float fontsize = g_pvp->m_settings.LoadValueWithDefault(regKey, "FontSize"s, 14.25f);
      fd.cySize.int64 = (LONGLONG)(fontsize * 10000.0f);

      string tmp;
      bool hr;
      hr = g_pvp->m_settings.LoadValue(regKey, "FontName"s, tmp);
      if (!hr)
         fd.lpstrName = (LPOLESTR)(L"Arial");
      else
      {
         const int len = (int)tmp.length() + 1;
         fd.lpstrName = (LPOLESTR)malloc(len*sizeof(WCHAR));
         memset(fd.lpstrName, 0, len*sizeof(WCHAR));
         MultiByteToWideCharNull(CP_ACP, 0, tmp.c_str(), -1, fd.lpstrName, len);

         free_lpstrName = true;
      }

      fd.sWeight = g_pvp->m_settings.LoadValueWithDefault(regKey, "FontWeight"s, (int)FW_NORMAL);
      fd.sCharset = g_pvp->m_settings.LoadValueWithDefault(regKey, "FontCharSet"s, 0);
      fd.fItalic = g_pvp->m_settings.LoadValueWithDefault(regKey, "FontItalic"s, 0);
      fd.fUnderline = g_pvp->m_settings.LoadValueWithDefault(regKey, "FontUnderline"s, 0);
      fd.fStrikethrough = g_pvp->m_settings.LoadValueWithDefault(regKey, "FontStrikeThrough"s, 0);

      hr = g_pvp->m_settings.LoadValue(regKey, "Text"s, m_d.m_sztext);
      if (!hr)
         m_d.m_sztext.clear();
   }

   SAFE_RELEASE(m_pIFont);
   OleCreateFontIndirect(&fd, IID_IFont, (void **)&m_pIFont);
   if (free_lpstrName)
      free(fd.lpstrName);

#undef regKey
}

void Textbox::WriteRegDefaults()
{
#define regKey Settings::DefaultPropsTextBox

   g_pvp->m_settings.SaveValue(regKey, "BackColor"s, (int)m_d.m_backcolor);
   g_pvp->m_settings.SaveValue(regKey, "FontColor"s, (int)m_d.m_fontcolor);
   g_pvp->m_settings.SaveValue(regKey, "TimerEnabled"s, m_d.m_tdr.m_TimerEnabled);
   g_pvp->m_settings.SaveValue(regKey, "TimerInterval"s, m_d.m_tdr.m_TimerInterval);
   g_pvp->m_settings.SaveValue(regKey, "Transparent"s, m_d.m_transparent);
   g_pvp->m_settings.SaveValue(regKey, "DMD"s, m_d.m_isDMD);

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

   g_pvp->m_settings.SaveValue(regKey, "Text"s, m_d.m_sztext);

#undef regKey
}


HRESULT Textbox::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
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
   IPersistStream * ips;
   m_pIFont->QueryInterface(IID_IPersistStream, (void **)&ips);
   const HRESULT hr = ips->Save(pstm, TRUE);
   SAFE_RELEASE_NO_RCC(ips);

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
      SAFE_RELEASE_NO_RCC(ips);
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
    FONTDESC fd;
    fd.cbSizeofstruct = sizeof(FONTDESC);
    m_pIFont->get_Size(&fd.cySize);
    const float fontSize = (float)(fd.cySize.int64 / 10000.0);

    LOGFONT lf = {};
    lf.lfHeight = -MulDiv((int)fontSize, GetDeviceCaps(g_pvp->GetDC(), LOGPIXELSY), 72);
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
   m_phittimer = new HitTimer(GetName(), m_d.m_tdr.m_TimerInterval, this);
   if (m_d.m_tdr.m_TimerEnabled)
      pvht.push_back(m_phittimer);
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


#pragma region Physics

void Textbox::GetHitShapes(vector<HitObject*> &pvho)
{
}

void Textbox::GetHitShapesDebug(vector<HitObject*> &pvho)
{
}

void Textbox::EndPlay()
{
   IEditable::EndPlay();
}

#pragma endregion


#pragma region Rendering

void Textbox::RenderSetup(RenderDevice *device)
{
   assert(m_rd == nullptr);
   m_rd = device;

   m_pIFont->Clone(&m_pIFontPlay);

   CY size;
   m_pIFontPlay->get_Size(&size);
   size.int64 = (LONGLONG)(size.int64 / 1.5 * g_pplayer->m_wnd_height * g_pplayer->m_wnd_width);
   m_pIFontPlay->put_Size(size);

   const int width = (int)max(m_d.m_v1.x, m_d.m_v2.x) - (int)min(m_d.m_v1.x, m_d.m_v2.x);
   const int height = (int)max(m_d.m_v1.y, m_d.m_v2.y) - (int)min(m_d.m_v1.y, m_d.m_v2.y);
   if (width > 0 && height > 0)
   {
      m_texture = new BaseTexture(width, height, BaseTexture::SRGBA);
      m_textureDirty = true;
   }
}

void Textbox::RenderRelease()
{
   assert(m_rd != nullptr);
   delete m_texture;
   m_texture = nullptr;
   SAFE_RELEASE(m_pIFontPlay);
   m_rd = nullptr;
}

void Textbox::UpdateAnimation(const float diff_time_msec)
{
   assert(m_rd != nullptr);
}

void Textbox::Render(const unsigned int renderMask)
{
   assert(m_rd != nullptr);
   const bool isStaticOnly = renderMask & Player::STATIC_ONLY;
   const bool isDynamicOnly = renderMask & Player::DYNAMIC_ONLY;
   const bool isReflectionPass = renderMask & Player::REFLECTION_PASS;
   TRACE_FUNCTION();

   const bool dmd = m_d.m_isDMD || StrStrI(m_d.m_sztext.c_str(), "DMD") != nullptr; //!! second part is VP10.0 legacy
   if (isStaticOnly
      || !m_d.m_visible
      || (m_backglass && isReflectionPass)
      || (!dmd && m_texture == nullptr)
      || (dmd && g_pplayer->m_texdmd == nullptr))
      return;

   constexpr float mult  = (float)(1.0 / EDITOR_BG_WIDTH);
   constexpr float ymult = (float)(1.0 / EDITOR_BG_HEIGHT);

   const float rect_left = min(m_d.m_v1.x, m_d.m_v2.x);
   const float rect_top = min(m_d.m_v1.y, m_d.m_v2.y);
   const float rect_right = max(m_d.m_v1.x, m_d.m_v2.x);
   const float rect_bottom = max(m_d.m_v1.y, m_d.m_v2.y);

   float x = rect_left*mult;
   float y = rect_top*ymult;
   float w = (rect_right - rect_left)*mult;
   float h = (rect_bottom - rect_top)*ymult;

   if (dmd)
   {
      const bool isExternalDMD = HasDMDCapture();

      m_rd->ResetRenderState();
      m_rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
      #ifdef ENABLE_SDL
      // If DMD capture is enabled check if external DMD exists and update m_texdmd with captured data (for capturing UltraDMD+P-ROC DMD)
      m_rd->DMDShader->SetTechnique(isExternalDMD ? SHADER_TECHNIQUE_basic_DMD_ext : SHADER_TECHNIQUE_basic_DMD); //!! DMD_UPSCALE ?? -> should just work
      if (g_pplayer->m_pin3d.m_backGlass)
      {
         g_pplayer->m_pin3d.m_backGlass->GetDMDPos(x, y, w, h);
         m_rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
         m_rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
         m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
      }
      #else
      //const float width = m_pin3d.m_useAA ? 2.0f*(float)m_width : (float)m_width; //!! AA ?? -> should just work
      m_rd->DMDShader->SetTechnique(SHADER_TECHNIQUE_basic_DMD); //!! DMD_UPSCALE ?? -> should just work
      #endif

      Vertex3D_NoTex2 vertices[4] = { 
         { 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f }, 
         { 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f }, 
         { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f },
         { 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f } };

      for (unsigned int i = 0; i < 4; ++i)
      {
         vertices[i].x = (vertices[i].x * w + x) * 2.0f - 1.0f;
         vertices[i].y = 1.0f - (vertices[i].y * h + y) * 2.0f;
      }

      const vec4 c = convertColor(m_d.m_fontcolor, m_d.m_intensity_scale);
      m_rd->DMDShader->SetVector(SHADER_vColor_Intensity, &c);
      #ifdef DMD_UPSCALE
      const vec4 r((float)(m_dmd.x * 3), (float)(m_dmd.y * 3), 1.f, (float)(m_overall_frames % 2048));
      #else
      const vec4 r((float)g_pplayer->m_dmd.x, (float)g_pplayer->m_dmd.y, 1.f, (float)(g_pplayer->m_overall_frames % 2048));
      #endif
      m_rd->DMDShader->SetVector(SHADER_vRes_Alpha_time, &r);
      m_rd->DMDShader->SetTexture(SHADER_tex_dmd, g_pplayer->m_texdmd, isExternalDMD ? SF_TRILINEAR : SF_NONE, SA_CLAMP, SA_CLAMP);
      m_rd->DrawTexturedQuad(m_rd->DMDShader, vertices);
      m_rd->GetCurrentPass()->m_commands.back()->SetTransparent(true);
      m_rd->GetCurrentPass()->m_commands.back()->SetDepth(-10000.f);
   }
   else if (m_texture)
   {
      if (m_textureDirty)
      {
         m_textureDirty = false;

         RECT rect;
         rect.left = (int)min(m_d.m_v1.x, m_d.m_v2.x);
         rect.top = (int)min(m_d.m_v1.y, m_d.m_v2.y);
         rect.right = (int)max(m_d.m_v1.x, m_d.m_v2.x);
         rect.bottom = (int)max(m_d.m_v1.y, m_d.m_v2.y);

         const int width = rect.right - rect.left;
         const int height = rect.bottom - rect.top;

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
         case TextAlignLeft: alignment = DT_LEFT; break;

         default:
         case TextAlignCenter: alignment = DT_CENTER; break;

         case TextAlignRight: alignment = DT_RIGHT; break;
         }

         const int border = (4 * g_pplayer->m_wnd_width) / EDITOR_BG_WIDTH;
         RECT rcOut;
         rcOut.left = border;
         rcOut.top = border;
         rcOut.right = width - border * 2;
         rcOut.bottom = height - border * 2;

         DrawText(hdc, m_d.m_sztext.c_str(), (int)m_d.m_sztext.length(), &rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK);

         GdiFlush(); // make sure everything is drawn

         // Set alpha for pixels that match transparent color (if transparent enabled), otherwise set to opaque
         const D3DCOLOR *__restrict bitsd = (D3DCOLOR *)bits;
         D3DCOLOR *__restrict dest = (D3DCOLOR *)m_texture->data();
         for (unsigned int i = 0; i < m_texture->height(); i++)
         {
            for (unsigned int l = 0; l < m_texture->width(); l++, dest++, bitsd++)
            {
               const D3DCOLOR src = *bitsd;
               if (m_d.m_transparent && ((src & 0xFFFFFFu) == m_d.m_backcolor))
                  *dest = 0x00000000; // set to black & alpha full transparent
               else
                  *dest = ((src & 0x000000FFu) << 16) | (src & 0x0000FF00u) | ((src & 0x0000FF0000u) >> 16) | 0xFF000000u;
            }
            dest += m_texture->pitch() / 4 - m_texture->width();
         }

         m_rd->m_texMan.SetDirty(m_texture);

         SelectObject(hdc, oldFont);
         SelectObject(hdc, oldBmp);
         DeleteDC(hdc);
         DeleteObject(hbm);
      }

      m_rd->ResetRenderState();
      m_rd->DMDShader->SetFloat(SHADER_alphaTestValue, (float)(128.0 / 255.0));
      g_pplayer->Spritedraw(x, y, w, h, 0xFFFFFFFF, m_rd->m_texMan.LoadTexture(m_texture, SF_TRILINEAR, SA_REPEAT, SA_REPEAT, false), m_d.m_intensity_scale);
      m_rd->DMDShader->SetFloat(SHADER_alphaTestValue, 1.0f);
   }
}

#pragma endregion


#pragma region ScriptProxy

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
   m_textureDirty = true;

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

#pragma endregion