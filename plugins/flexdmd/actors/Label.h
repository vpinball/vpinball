#pragma once

#include "Actor.h"
#include "ressources/Font.h"

class Label : public Actor
{
public:
   Label(FlexDMD* pFlexDMD, Font* pFont, const string& text, const string& name);
   ~Label();

   virtual ActorType GetType() const override { return AT_Label; }

   Alignment GetAlignment() const { return m_alignment; }
   void SetAlignment(Alignment alignment) { m_alignment = alignment; }
   Font* GetFont() { return m_pFont; }
   void SetFont(Font* pFont) { m_pFont->Release(); m_pFont = pFont; m_pFont->AddRef(); UpdateBounds(); }
   const string& GetText() const { return m_szText; }
   void SetText(const string& szText);
   const bool GetAutoPack() const { return m_autopack; }
   void SetAutoPack(bool autopack) { m_autopack = autopack; }
   float GetPrefWidth() const override { return m_textWidth; }
   float GetPrefHeight() const override { return m_textHeight; }
   void Draw(VP::SurfaceGraphics* pGraphics) override;

private:
   void ProcessText();
   void UpdateBounds();

   Font* m_pFont;
   string m_szText;
   bool m_autopack;
   float m_textWidth;
   float m_textHeight;
   Alignment m_alignment;
   vector<string> m_lines;
};