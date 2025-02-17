#pragma once

#include "BackgroundScene.h"
#include "actors/Label.h"

class TwoLineScene final : public BackgroundScene
{
public:
   TwoLineScene(FlexDMD* pFlexDMD, Actor* pBackground, const string& topText, Font* pTopFont, const string& bottomText, Font* pBottomFont, AnimationType animateIn, float pauseS, AnimationType animateOut, const string& id);
   ~TwoLineScene();

   void SetText(const string& topText, const string& bottomText);
   void Update(float delta) override;

private:
   Label* m_pTopText;
   Label* m_pBottomText;
};
