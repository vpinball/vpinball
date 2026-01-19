#include "Label.h"
#include "FlexDMD.h"

#include <sstream>
#include <cmath>

namespace Flex {

Label::Label(const FlexDMD* pFlexDMD, Font* pFont, const string& text, const string& name)
   : Actor(pFlexDMD, name)
{
   m_alignment = Alignment_Center;

   m_pFont = pFont;

   if (m_pFont)
      m_pFont->AddRef();
   else {
      LOGW("NULL font specified for label: %s", name.c_str());
   }

   m_autopack = pFlexDMD->GetRuntimeVersion() <= 1008;
   m_text = text + '_'; // To force bounds update
   SetText(text);
   Pack();
}

Label::~Label()
{
   if (m_pFont)
      m_pFont->Release();
}

void Label::SetText(const string& szText)
{
   string szTmp = szText;

   static const std::array<string,1> patterns{ "\r\n"s };
   static const string replaceWith = "\n"s;

   for (const auto& pattern : patterns) {
      size_t pos = 0;
      while((pos = szTmp.find(pattern, pos)) != string::npos) {
         szTmp.replace(pos, pattern.size(), replaceWith);
         pos += replaceWith.size();
      }
   }

   if (m_text != szTmp) {
      m_text = szTmp;
      m_lines.clear();
      std::stringstream ss(m_text);
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

   SDL_Rect size = m_pFont->MeasureFont(m_text);
   m_textWidth = size.w;
   m_textHeight = size.h;

   if (m_autopack)
      Pack();
}

void Label::Draw(Flex::SurfaceGraphics* pGraphics)
{
   Actor::Draw(pGraphics);
   if (GetVisible() && m_pFont) {
      if (m_lines.size() > 1 && m_alignment != Alignment_Left && m_alignment != Alignment_BottomLeft && m_alignment != Alignment_TopLeft) {
         float x = 0;
         float y = 0;
         Layout::Align(m_alignment, (float)GetPrefWidth(), (float)GetPrefHeight(), GetWidth(), GetHeight(), x, y);

         for (const string& line : m_lines) {
            float lx = 0;
            float ly = 0;
            Layout::Align(m_alignment, static_cast<float>(m_pFont->MeasureFont(line).w), static_cast<float>(GetPrefHeight()), GetWidth(), GetHeight(), lx, ly);
            if (GetFlexDMD()->GetRuntimeVersion() <= 1008) // FIXME check this against original code. Is it a cast or a rounding ?
               m_pFont->DrawText_(pGraphics, (int)roundf(GetX() + lx), (int)roundf(GetY() + y), line);
            else
               m_pFont->DrawText_(pGraphics, (int)floorf(GetX() + lx), (int)floorf(GetY() + y), line);
            y += (float)m_pFont->GetBitmapFont()->GetLineHeight();
         }
      }
      else {
         float x = 0;
         float y = 0;
         Layout::Align(m_alignment, (float)GetPrefWidth(), (float)GetPrefHeight(), GetWidth(), GetHeight(), x, y);
         if (GetFlexDMD()->GetRuntimeVersion() <= 1008) // FIXME check this against original code. Is it a cast or a rounding ?
            m_pFont->DrawText_(pGraphics, (int)roundf(GetX() + x), (int)roundf(GetY() + y), m_text);
         else
            m_pFont->DrawText_(pGraphics, (int)floorf(GetX() + x), (int)floorf(GetY() + y), m_text);
      }
   }
}

}
