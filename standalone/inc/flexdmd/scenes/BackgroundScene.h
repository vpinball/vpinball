#pragma once

#include "Scene.h"

class BackgroundScene : public Scene
{
public:
   BackgroundScene(FlexDMD* pFlexDMD, Actor* pBackground, AnimationType animateIn, float pauseS, AnimationType animateOut, string id);
   ~BackgroundScene();

   void SetBackground(Actor* background);
   Actor* GetBackground() { return m_pBackground; }

   virtual void Update(float delta);

private:
   Actor* m_pBackground;
};