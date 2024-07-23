#include "stdafx.h"

#include "Label.h"

#include <sstream>

Label::Label(FlexDMD* pFlexDMD, Font* pFont, const string& text, const string& name) : Actor(pFlexDMD, name)
{
   m_autopack = false;
   m_alignment = Alignment_Center;

   m_pFont = pFont;

   if (m_pFont)
      m_pFont->AddRef();
   else {
      PLOGW.printf("NULL font specified for label %s", name.c_str());
   }

   m_autopack = pFlexDMD->GetRuntimeVersion() <= 1008;
   SetText(text);
   Pack();
}

Label::~Label()
{
   if (m_pFont)
      m_pFont->Release();
}

STDMETHODIMP Label::get_AutoPack(VARIANT_BOOL *pRetVal)
{
   *pRetVal = m_autopack ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP Label::put_AutoPack(VARIANT_BOOL pRetVal)
{
   m_autopack = (pRetVal == VARIANT_TRUE);

   return S_OK;
}

void Label::Missing36() { }
void Label::Missing37() { }

STDMETHODIMP Label::get_Font(IUnknown **pRetVal)
{
   return m_pFont ? m_pFont->QueryInterface(IID_IUnknown, (void**)pRetVal) : E_FAIL;
}

STDMETHODIMP Label::putref_Font(IUnknown *pRetVal)
{
   if (m_pFont)
      m_pFont->Release();

   m_pFont = dynamic_cast<Font*>(pRetVal);

   if (m_pFont) {
      m_pFont->AddRef();
      UpdateBounds();
   }

   return S_OK;
}

STDMETHODIMP Label::get_Text(BSTR *pRetVal)
{
   CComBSTR Val(m_szText.c_str());
   *pRetVal = Val.Detach();

   return S_OK;
}

STDMETHODIMP Label::put_Text(BSTR pRetVal)
{
   SetText(MakeString(pRetVal));

   return S_OK;
}

void Label::SetText(const string& szText)
{
   string szTmp = szText;

   static string patterns[] = { "\r\n" };
   static string replaceWith = "\n";

   for (const auto& pattern : patterns) {
      size_t pos = 0;
      while((pos = szTmp.find(pattern, pos)) != string::npos) {
         szTmp.replace(pos, pattern.size(), replaceWith);
         pos += replaceWith.size();
      }
   }

   if (m_szText != szTmp) {
      m_szText = szTmp;
      m_lines.clear();
      std::stringstream ss(m_szText);
      string line;
      while (std::getline(ss, line, '\n'))
         m_lines.push_back(line);
      UpdateBounds();
   }
}

void Label::UpdateBounds()
{
   if (!m_pFont)
      return;

   SDL_Rect size = m_pFont->MeasureFont(m_szText);
   m_textWidth = size.w;
   m_textHeight = size.h;

   if (m_autopack)
      Pack();
}

void Label::Draw(VP::SurfaceGraphics* pGraphics)
{
   Actor::Draw(pGraphics);
   if (GetVisible() && m_pFont) {
      if (m_lines.size() > 1 && m_alignment != Alignment_Left && m_alignment != Alignment_BottomLeft && m_alignment != Alignment_TopLeft) {
         float x = 0;
         float y = 0;
         Layout::Align(m_alignment, GetPrefWidth(), GetPrefHeight(), GetWidth(), GetHeight(), x, y);

         for (const string& line : m_lines) {
            float lx = 0;
            float ly = 0;
            Layout::Align(m_alignment, m_pFont->MeasureFont(line).w, GetPrefHeight(), GetWidth(), GetHeight(), lx, ly);
            if (GetFlexDMD()->GetRuntimeVersion() <= 1008)
               m_pFont->DrawText_(pGraphics, (int)(GetX() + lx), (int)(GetY() + y), line);
            else
               m_pFont->DrawText_(pGraphics, (int)floor(GetX() + lx), (int)floor(GetY() + y), line);
            y += m_pFont->GetBitmapFont()->GetLineHeight();
         }
      }
      else {
         float x = 0;
         float y = 0;
         Layout::Align(m_alignment, GetPrefWidth(), GetPrefHeight(), GetWidth(), GetHeight(), x, y);
         if (GetFlexDMD()->GetRuntimeVersion() <= 1008)
            m_pFont->DrawText_(pGraphics, (int)(GetX() + x), (int)(GetY() + y), m_szText);
         else
            m_pFont->DrawText_(pGraphics, (int)floor(GetX() + x), (int)floor(GetY() + y), m_szText);
      }
   }
}