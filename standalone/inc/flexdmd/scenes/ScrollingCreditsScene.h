#pragma once

#include "Scene.h"
#include "BackgroundScene.h"

class ScrollingCreditsScene final : public BackgroundScene
{
public:
   ScrollingCreditsScene(FlexDMD* pFlexDMD, Actor* pBackground, const vector<string>& text, Font* pFont, AnimationType animateIn, float pauseS, AnimationType animateOut, const string& id);
   ~ScrollingCreditsScene() override;

   void Begin() override;
   void Update(float delta) override;

private:
   Group* m_pContainer;
   float m_length;
};
