#pragma once

#include "BackgroundScene.h"
#include "../actors/Label.h"

class SingleLineScene : public BackgroundScene
{
public:
   SingleLineScene(FlexDMD* pFlexDMD, Actor* pBackground, const string& text, Font* pFont, AnimationType animateIn, float pauseS, AnimationType animateOut, bool scroll, const string& id);
   ~SingleLineScene();

   void SetText(string text);
   void Begin() override;
   void Update(float delta) override;

private:
   Label* m_pText;
   bool m_scroll;
   float m_scrollX;
};