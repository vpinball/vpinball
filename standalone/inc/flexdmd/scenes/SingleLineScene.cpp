#include "stdafx.h"

#include "SingleLineScene.h"

SingleLineScene::SingleLineScene(FlexDMD* pFlexDMD, Actor* pBackground, const string& text, Font* pFont, AnimationType animateIn, float pauseS, AnimationType animateOut, bool scroll, const string& id) : BackgroundScene(pFlexDMD, pBackground, animateIn, pauseS, animateOut, id)
{
   m_pText = new Label(pFlexDMD, pFont, text, "");
   m_pText->AddRef();
   AddActor((Actor*)m_pText);

   m_scroll = scroll;
}

SingleLineScene::~SingleLineScene()
{
   m_pText->Release();
}

void SingleLineScene::SetText(string text)
{
   m_pText->SetText(text);
}

void SingleLineScene::Begin()
{
   BackgroundScene::Begin();
   if (m_scroll) {
      // UltraDMD moves text by 1.2 pixel per frame (no delta time) and seems to render based on a frame rate at 60FPS.
      float offset = -(GetWidth() - m_pText->GetWidth()) / 2.0f;
      m_scrollX = offset + GetWidth();
      AddTween(m_scrollX, offset - m_pText->GetWidth(), (GetWidth() + m_pText->GetWidth()) / (60.0f * 1.2f), ([this](float newValue) {
         this->m_scrollX = newValue;
         return false;
      }));
   }
}

void SingleLineScene::Update(float delta)
{
   BackgroundScene::Update(delta);
   m_pText->SetPosition((GetWidth() - m_pText->GetWidth()) / 2.0f + m_scrollX, (GetHeight() - m_pText->GetHeight()) / 2.0f);
}