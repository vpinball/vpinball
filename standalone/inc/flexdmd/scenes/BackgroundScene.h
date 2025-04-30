#pragma once

#include "Scene.h"

class BackgroundScene : public Scene
{
public:
   BackgroundScene(FlexDMD* pFlexDMD, Actor* pBackground, AnimationType animateIn, float pauseS, AnimationType animateOut, const string& id);
   ~BackgroundScene();

   void SetBackground(Actor* background);
   Actor* GetBackground() const { return m_pBackground; }
   void Update(float delta) override;

private:
   Actor* m_pBackground;
};
