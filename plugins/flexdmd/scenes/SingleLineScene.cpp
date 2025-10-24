#include "SingleLineScene.h"

namespace Flex {

SingleLineScene::SingleLineScene(FlexDMD* pFlexDMD, Actor* pBackground, const string& text, Font* pFont, AnimationType animateIn, float pauseS, AnimationType animateOut, bool scroll, const string& id) : BackgroundScene(pFlexDMD, pBackground, animateIn, pauseS, animateOut, id)
{
   m_pText = new Label(pFlexDMD, pFont, text, string());
   AddActor(m_pText);

   m_scroll = scroll;
}

SingleLineScene::~SingleLineScene()
{
   m_pText->Remove();
}

void SingleLineScene::SetText(const string& text)
{
   m_pText->SetText(text);
}

void SingleLineScene::Begin()
{
   BackgroundScene::Begin();
   if (m_scroll) {
      // UltraDMD moves text by 1.2 pixel per frame (no delta time) and seems to render based on a frame rate at 60FPS.
      float offset = (float)-(GetWidth() - m_pText->GetWidth()) / 2.0f;
      m_scrollX = offset + (float)GetWidth();
      AddTween(m_scrollX, offset - (float)m_pText->GetWidth(), (float)(GetWidth() + m_pText->GetWidth()) / (60.0f * 1.2f), ([this](float newValue) {
         this->m_scrollX = newValue;
         return false;
      }));
   }
}

void SingleLineScene::Update(float delta)
{
   BackgroundScene::Update(delta);
   m_pText->SetPosition((float)(GetWidth() - m_pText->GetWidth()) / 2.0f + m_scrollX, (float)(GetHeight() - m_pText->GetHeight()) / 2.0f);
}

}