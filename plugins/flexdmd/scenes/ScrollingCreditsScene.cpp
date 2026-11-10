#include "ScrollingCreditsScene.h"
#include "actors/Label.h"

// Credits scrolling:
// 1 (default) = original C# FlexDMD: the text container scrolls up from the bottom, the scene and its background stay in place
// 0 = earlier versions of this plugin: the whole scene scrolls up from its own position
#ifndef FLEXDMD_CSHARP_CREDITS_SCROLL
 #define FLEXDMD_CSHARP_CREDITS_SCROLL 1
#endif

namespace Flex {

ScrollingCreditsScene::ScrollingCreditsScene(FlexDMD* pFlexDMD, Actor* pBackground, const vector<string>& text, Font* pFont, AnimationType animateIn, float pauseS, AnimationType animateOut, const string& id) : BackgroundScene(pFlexDMD, pBackground, animateIn, pauseS, animateOut, id)
{
   m_pContainer = new Group(pFlexDMD, string());
   m_pContainer->AddRef();
   AddActor(m_pContainer);

   m_length = 3.0f + (float)text.size() * 0.4f;

   int y = 0;

   for (const auto& line : text) {
      string txt = trim_string(line);
      if (txt.empty())
         txt = " "sv;
      Label* const pLabel = new Label(pFlexDMD, pFont, txt, string());
      pLabel->SetY((float)y);
      y += pLabel->GetHeight();
      m_pContainer->AddActor(pLabel);
   }
   m_pContainer->SetHeight(y);
}

ScrollingCreditsScene::~ScrollingCreditsScene()
{
   m_pContainer->Release();
}

void ScrollingCreditsScene::Begin()
{
   BackgroundScene::Begin();
   m_pContainer->SetY((float)GetHeight());
#if FLEXDMD_CSHARP_CREDITS_SCROLL
   AddTween(m_pContainer, TweenProperty::Y, (float)-m_pContainer->GetHeight(), m_length);
#else
   AddTween(this, TweenProperty::Y, (float)-m_pContainer->GetHeight(), m_length);
#endif
}

void ScrollingCreditsScene::Update(float delta)
{
   BackgroundScene::Update(delta);
   if (m_pContainer->GetWidth() != GetWidth()) {
      m_pContainer->SetWidth(GetWidth());
      for (const auto& line : m_pContainer->GetChildren()) {
         Label* label = dynamic_cast<Label*>(line);
         label->SetX((float)(GetWidth() - line->GetWidth()) / 2.0f);
         line->Release();
      }
   }
}

}
