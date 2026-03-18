// license:GPLv3+

#include "core/stdafx.h"
#include "textbox.h"

#include "renderer/Shader.h"
#include "renderer/RenderCommand.h"

Textbox::~Textbox()
{
   assert(m_rd == nullptr);
}

Textbox *Textbox::CopyForPlay() const
{
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(Textbox)
   return dst;
}

HRESULT Textbox::Init(const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   SetDefaults(fromMouseClick);
   const float width  = g_app->m_settings.GetDefaultPropsTextbox_Width();
   const float height = g_app->m_settings.GetDefaultPropsTextbox_Height();
   m_d.m_v1.x = x;
   m_d.m_v1.y = y;
   m_d.m_v2.x = x + width;
   m_d.m_v2.y = y + height;
   return S_OK;
}

void Textbox::SetDefaults(const bool fromMouseClick)
{
#define LinkProp(field, prop) field = fromMouseClick ? g_app->m_settings.GetDefaultPropsTextbox_##prop() : Settings::GetDefaultPropsTextbox_##prop##_Default()
   m_d.m_visible = true;
   LinkProp(m_d.m_backcolor, BackColor);
   LinkProp(m_d.m_fontcolor, FontColor);
   LinkProp(m_d.m_transparent, Transparent);
   LinkProp(m_d.m_isDMD, DMD);
   LinkProp(m_d.m_backcolor, BackColor);
   LinkProp(m_d.m_intensity_scale, IntensityScale);
   LinkProp(m_d.m_text, Text);
   LinkProp(m_d.m_talign, TextAlignment);
   LinkProp(m_d.m_tdr.m_TimerEnabled, TimerEnabled);
   LinkProp(m_d.m_tdr.m_TimerInterval, TimerInterval);
   LinkProp(m_d.m_font.name, FontName);
   LinkProp(m_d.m_font.weight, FontWeight);
   LinkProp(m_d.m_font.charset, FontCharSet);

   float fontSize;
   LinkProp(fontSize, FontSize);
   m_d.m_font.size = (uint32_t)(fontSize * 10000.0f);

   bool fItalic, fUnderline, fStrikethrough;
   LinkProp(fItalic, FontItalic);
   LinkProp(fUnderline, FontUnderline);
   LinkProp(fStrikethrough, FontStrikeThrough);
   m_d.m_font.attributes = (fItalic ? 0x02 : 0x00) | (fUnderline ? 0x04 : 0x00) | (fStrikethrough ? 0x08 : 0x00);
#undef LinkProp
}

void Textbox::WriteRegDefaults()
{
#define LinkProp(field, prop) g_app->m_settings.SetDefaultPropsTextbox_##prop(field, false)
   LinkProp(m_d.m_backcolor, BackColor);
   LinkProp(m_d.m_fontcolor, FontColor);
   LinkProp(m_d.m_transparent, Transparent);
   LinkProp(m_d.m_isDMD, DMD);
   LinkProp(m_d.m_backcolor, BackColor);
   LinkProp(m_d.m_intensity_scale, IntensityScale);
   LinkProp(m_d.m_text, Text);
   LinkProp(m_d.m_talign, TextAlignment);
   LinkProp(m_d.m_tdr.m_TimerEnabled, TimerEnabled);
   LinkProp(m_d.m_tdr.m_TimerInterval, TimerInterval);

   const float fontSize = (float)(m_d.m_font.size / 10000.0);
   const bool fItalic = (m_d.m_font.attributes & 0x02) != 0;
   const bool fUnderline = (m_d.m_font.attributes & 0x04) != 0;
   const bool fStrikethrough = (m_d.m_font.attributes & 0x08) != 0;

   LinkProp(fontSize, FontSize);
   LinkProp(m_d.m_font.name, FontName);
   LinkProp(m_d.m_font.weight, FontWeight);
   LinkProp(m_d.m_font.charset, FontCharSet);
   LinkProp(fItalic, FontItalic);
   LinkProp(fUnderline, FontUnderline);
   LinkProp(fStrikethrough, FontStrikeThrough);
#undef LinkProp
}

void Textbox::Save(IObjectWriter& writer, const bool saveForUndo)
{
   writer.WriteVector2(FID(VER1), m_d.m_v1);
   writer.WriteVector2(FID(VER2), m_d.m_v2);
   writer.WriteInt(FID(CLRB), m_d.m_backcolor);
   writer.WriteInt(FID(CLRF), m_d.m_fontcolor);
   writer.WriteFloat(FID(INSC), m_d.m_intensity_scale);
   writer.WriteString(FID(TEXT), m_d.m_text);
   writer.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   writer.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   writer.WriteWideString(FID(NAME), m_wzName);
   writer.WriteInt(FID(ALGN), m_d.m_talign);
   writer.WriteBool(FID(TRNS), m_d.m_transparent);
   writer.WriteBool(FID(IDMD), m_d.m_isDMD);
   SaveSharedEditableFields(writer);
   writer.WriteFontDescriptor(FID(FONT), m_d.m_font);
   writer.EndObject();
}

void Textbox::Load(IObjectReader& reader)
{
   SetDefaults(false);
   reader.AsObject(
      [this](int tag, IObjectReader& reader)
      {
         switch (tag)
         {
         case FID(PIID): reader.AsInt(); break;
         case FID(VER1): m_d.m_v1 = reader.AsVector2(); break;
         case FID(VER2): m_d.m_v2 = reader.AsVector2(); break;
         case FID(CLRB): m_d.m_backcolor = reader.AsInt(); break;
         case FID(CLRF): m_d.m_fontcolor = reader.AsInt(); break;
         case FID(INSC): m_d.m_intensity_scale = reader.AsFloat(); break;
         case FID(TMON): m_d.m_tdr.m_TimerEnabled = reader.AsBool(); break;
         case FID(TMIN): m_d.m_tdr.m_TimerInterval = reader.AsInt(); break;
         case FID(TEXT): m_d.m_text = reader.AsString(); break;
         case FID(NAME): m_wzName = reader.AsWideString(); break;
         case FID(ALGN): m_d.m_talign = static_cast<TextAlignment>(reader.AsInt()); break;
         case FID(TRNS): m_d.m_transparent = reader.AsBool(); break;
         case FID(IDMD): m_d.m_isDMD = reader.AsBool(); break;
         case FID(FONT):
         {
            m_d.m_font = reader.AsFontDescriptor();
            break;
         }
         default: LoadSharedEditableField(tag, reader); break;
         }
         return true;
      });
   m_texture = nullptr;
}

string Textbox::GetFontName() const
{
   return m_d.m_font.name;
}

STDMETHODIMP Textbox::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_ITextbox,
   };

   for (size_t i = 0; i < std::size(arr); i++)
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;

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

void Textbox::PhysicSetup(PhysicsEngine* physics, const bool isUI)
{
   if (isUI)
   {
      // FIXME implement UI picking
   }
}

void Textbox::PhysicRelease(PhysicsEngine* physics, const bool isUI)
{
}

#pragma endregion


#pragma region Rendering

void Textbox::RenderSetup(RenderDevice *device)
{
   assert(m_rd == nullptr);
   m_rd = device;

#ifndef __STANDALONE__
   FONTDESC oleFD = m_d.m_font.ToOLEFontDesc();
   OleCreateFontIndirect(&oleFD, IID_IFont, (void **)&m_pIFontPlay);
   delete[] oleFD.lpstrName;

   CY size;
   m_pIFontPlay->get_Size(&size);
   size.int64 = (LONGLONG)(size.int64 / 1.5 * (g_pplayer->m_playfieldWnd->GetWidth() * g_pplayer->m_playfieldWnd->GetHeight()));
   m_pIFontPlay->put_Size(size);
#endif

   const int width = (int)max(m_d.m_v1.x, m_d.m_v2.x) - (int)min(m_d.m_v1.x, m_d.m_v2.x);
   const int height = (int)max(m_d.m_v1.y, m_d.m_v2.y) - (int)min(m_d.m_v1.y, m_d.m_v2.y);
   if (width > 0 && height > 0)
   {
      m_texture = BaseTexture::Create(width, height, BaseTexture::SRGBA);
      m_texture->SetName("Textbox." + GetName());
      m_textureDirty = true;
   }
}

void Textbox::RenderRelease()
{
   assert(m_rd != nullptr);
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
   assert(m_desktopBackdrop);
   const bool isStaticOnly = renderMask & Renderer::STATIC_ONLY;
   const bool isDynamicOnly = renderMask & Renderer::DYNAMIC_ONLY;
   const bool isReflectionPass = renderMask & Renderer::REFLECTION_PASS;
   TRACE_FUNCTION();

   const bool is_dmd = m_d.m_isDMD || StrFindNoCase(m_d.m_text, "DMD"s) != string::npos; //!! second part is VP10.0 legacy
   if (isStaticOnly
      || !m_d.m_visible
      || (!is_dmd && m_texture == nullptr))
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

   #ifdef ENABLE_DX9
      x -= 0.5f / m_rd->GetOutputBackBuffer()->GetWidth();
      y -= 0.5f / m_rd->GetOutputBackBuffer()->GetHeight();
   #endif

   if (is_dmd)
   {
      m_rd->ResetRenderState();
      m_rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
      m_rd->m_DMDShader->SetTechnique(SHADER_TECHNIQUE_basic_DMD);

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

      ResURIResolver::DisplayState dmd = g_pplayer->m_resURIResolver.GetDisplayState("ctrl://default/display"s);
      if (dmd.state.frame == nullptr)
         return;
      BaseTexture::Update(m_texture, dmd.source->width, dmd.source->height, 
              dmd.source->frameFormat == CTLPI_DISPLAY_FORMAT_LUM32F  ? BaseTexture::BW_FP32
            : dmd.source->frameFormat == CTLPI_DISPLAY_FORMAT_SRGB565 ? BaseTexture::SRGB565
                                                                      : BaseTexture::SRGB,
         dmd.state.frame);
      // DMD support for textbox is for backward compatibility only, so only use compatibility style #0
      const vec3 color = m_texture->m_format == BaseTexture::BW_FP32 ? convertColor(m_d.m_fontcolor) : vec3(1.f, 1.f, 1.f);
      g_pplayer->m_renderer->SetupDMDRender(0, true, color, m_d.m_intensity_scale, m_texture, 1.f, Renderer::Reinhard, nullptr,
         vec4(0.f, 0.f, 0.f, 0.f), vec3(1.f, 1.f, 1.f), 0.f,
         nullptr, vec4(), vec3(0.f, 0.f, 0.f));
      m_rd->DrawTexturedQuad(m_rd->m_DMDShader, vertices);
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

         const int border = (4 * g_pplayer->m_playfieldWnd->GetWidth()) / EDITOR_BG_WIDTH;
         RECT rcOut;
         rcOut.left = border;
         rcOut.top = border;
         rcOut.right = width - border * 2;
         rcOut.bottom = height - border * 2;

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
         case TextAlignLeft: alignment = DT_LEFT; break;

         default:
         case TextAlignCenter: alignment = DT_CENTER; break;

         case TextAlignRight: alignment = DT_RIGHT; break;
         }

         DrawText(hdc, m_d.m_text.c_str(), (int)m_d.m_text.length(), &rcOut, alignment | DT_NOCLIP | DT_NOPREFIX | DT_WORDBREAK);

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

         SelectObject(hdc, oldFont);
         SelectObject(hdc, oldBmp);
         DeleteDC(hdc);
         DeleteObject(hbm);
#else
         SDL_Surface* pSurface = SDL_CreateSurface(m_texture->width(), m_texture->height(), SDL_PIXELFORMAT_ABGR8888);
         if (pSurface) {
            SDL_FillSurfaceRect(pSurface, NULL,
               SDL_MapRGBA(SDL_GetPixelFormatDetails(pSurface->format), NULL,
               GetRValue(m_d.m_backcolor),
               GetGValue(m_d.m_backcolor),
               GetBValue(m_d.m_backcolor),
               m_d.m_transparent ? 0 : 255));

            TTF_Font* pFont = LoadFont();
            if (pFont) {
               SDL_Color textColor = {
                  GetRValue(m_d.m_fontcolor),
                  GetGValue(m_d.m_fontcolor),
                  GetBValue(m_d.m_fontcolor),
                  255
               };

               int maxWidth = rcOut.right - rcOut.left;
               SDL_Surface* pTextSurface = TTF_RenderText_Blended_Wrapped(pFont, m_d.m_text.c_str(), m_d.m_text.length(), textColor, maxWidth);
               if (pTextSurface) {
                  SDL_Rect textRect;
                  textRect.y = rcOut.top;

                  switch (m_d.m_talign) {
                     case TextAlignLeft:
                        textRect.x = rcOut.left;
                        break;
                     case TextAlignCenter:
                        textRect.x = rcOut.left + (maxWidth - pTextSurface->w) / 2;
                        break;
                     case TextAlignRight:
                        textRect.x = rcOut.right - pTextSurface->w;
                        break;
                     default:
                        textRect.x = rcOut.left;
                        break;
                  }

                  textRect.w = pTextSurface->w;
                  textRect.h = pTextSurface->h;

                  if (textRect.y + textRect.h > rcOut.bottom)
                     textRect.h = rcOut.bottom - textRect.y;

                  SDL_BlitSurface(pTextSurface, NULL, pSurface, &textRect);
                  SDL_DestroySurface(pTextSurface);
               }
               TTF_CloseFont(pFont);
            }
            memcpy(m_texture->data(), pSurface->pixels, pSurface->pitch * pSurface->h);
            SDL_DestroySurface(pSurface);
         }
#endif
         m_rd->m_texMan.SetDirty(m_texture.get());
      }

      m_rd->ResetRenderState();
      m_rd->m_DMDShader->SetFloat(SHADER_alphaTestValue, (float)(128.0 / 255.0));
      g_pplayer->m_renderer->DrawSprite(x, y, w, h, 0xFFFFFFFF, m_rd->m_texMan.LoadTexture(m_texture.get(), false), m_d.m_intensity_scale);
      m_rd->m_DMDShader->SetFloat(SHADER_alphaTestValue, 1.0f);
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
   *pVal = MakeWideBSTR(m_d.m_text);
   return S_OK;
}

STDMETHODIMP Textbox::put_Text(BSTR newVal)
{
   m_d.m_text = MakeString(newVal);
   m_textureDirty = true;

   return S_OK;
}

STDMETHODIMP Textbox::get_Font(IFontDisp **pVal)
{
#ifndef __STANDALONE__
   IFont* pIFont;
   FONTDESC oleFD = m_d.m_font.ToOLEFontDesc();
   OleCreateFontIndirect(&oleFD, IID_IFont, (void **)&pIFont);
   delete[] oleFD.lpstrName;
   const HRESULT hr = pIFont->QueryInterface(IID_IFontDisp, (void **)pVal);
   pIFont->Release();
   return hr;
#else
   return S_OK;
#endif
}

STDMETHODIMP Textbox::put_Font(IFontDisp *newVal)
{
   // Does anybody use this way of setting the font?  Need to add to idl file.
   return S_OK;
}

STDMETHODIMP Textbox::putref_Font(IFontDisp* pFont)
{
   //We know that our own property browser gives us the same pointer
#ifndef __STANDALONE__
   IFont* pIFont;
   if (SUCCEEDED(pFont->QueryInterface(IID_IFont, (void**)&pIFont)))
   {
       m_d.m_font.FromOLEFont(pIFont);
       pIFont->Release();
   }
#endif

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
   if (m_vpinball)
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

#ifdef __STANDALONE__
TTF_Font* Textbox::LoadFont()
{
   TTF_Font* pFont = nullptr;

   string fontName = m_d.m_font.name;
   std::erase(fontName, ' ');

   vector<string> styles;
   if (m_d.m_font.IsBold() > 550 && m_d.m_font.IsItalic())
      styles.push_back("-BoldItalic"s);
   if (m_d.m_font.IsBold())
      styles.push_back("-Bold"s);
   if (m_d.m_font.IsItalic())
      styles.push_back("-Italic"s);
   styles.push_back("-Regular"s);

   const std::filesystem::path tablePath = PathFromFilename(GetPTable()->m_filename);

   std::filesystem::path path;
   for (const auto& szStyle : styles) {
      path = find_case_insensitive_file_path(tablePath / (fontName + szStyle + ".ttf"));
      if (!path.empty()) {
         pFont = TTF_OpenFont(path.string().c_str(), (float)m_d.m_font.size / 10000.f);
         if (pFont) {
            PLOGI << "Font loaded: path=" << path.string();
            break;
         }
      }
   }

   if (!pFont) {
      path = tablePath / (fontName + styles[0] + ".ttf");
      PLOGW << "Unable to locate font: path=" << path.string();

      path = g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets) / "LiberationSans-Regular.ttf";
      pFont = TTF_OpenFont(path.string().c_str(), (float)m_d.m_font.size / 10000.f);
      if (pFont) {
         PLOGW << "Default font loaded: path=" << path.string();
      }
      else {
         PLOGW << "Unable to load font: path=" << path.string();
         return nullptr;
      }
   }

   TTF_FontStyleFlags style = TTF_STYLE_NORMAL;
   if (m_d.m_font.IsBold())
      style |= TTF_STYLE_BOLD;
   if (m_d.m_font.IsItalic())
      style |= TTF_STYLE_ITALIC;
   if (m_d.m_font.IsUnderline())
      style |= TTF_STYLE_UNDERLINE;
   if (m_d.m_font.IsStrikeThrough())
      style |= TTF_STYLE_STRIKETHROUGH;
   TTF_SetFontStyle(pFont, style);

   return pFont;
}
#endif

#pragma endregion
