#include "BackgroundScene.h"

namespace Flex {

BackgroundScene::BackgroundScene(FlexDMD* pFlexDMD, Actor* pBackground, AnimationType animateIn, float pauseS, AnimationType animateOut, const string& id) : Scene(pFlexDMD, animateIn, pauseS, animateOut, id)
{
   m_pBackground = pBackground;
   AddActor(m_pBackground);
}

BackgroundScene::~BackgroundScene()
{
}

void BackgroundScene::SetBackground(Actor* pBackground)
{
   if (m_pBackground == pBackground) return;
   if (m_pBackground != nullptr)
      RemoveActor(m_pBackground);
   m_pBackground = pBackground;
   if (m_pBackground != nullptr)
      AddActorAt(m_pBackground, 0);
}

void BackgroundScene::Update(float delta)
{
   Scene::Update(delta);
   if (m_pBackground)
      m_pBackground->SetSize(GetWidth(), GetHeight());
}

}