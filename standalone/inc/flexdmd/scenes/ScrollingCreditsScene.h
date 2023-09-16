#pragma once

#include "Scene.h"
#include "BackgroundScene.h"

class ScrollingCreditsScene : public BackgroundScene
{
public:
   ScrollingCreditsScene(FlexDMD* pFlexDMD, Actor* pBackground, vector<string> text, Font* pFont, AnimationType animateIn, float pauseS, AnimationType animateOut, const string& id);
   ~ScrollingCreditsScene();

   virtual void Begin();
   virtual void Update(float delta);

private:
   Group* m_pContainer;
   float m_length;
};