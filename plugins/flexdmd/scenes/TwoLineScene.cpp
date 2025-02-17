#include "TwoLineScene.h"

TwoLineScene::TwoLineScene(FlexDMD* pFlexDMD, Actor* pBackground, const string& topText, Font* pTopFont, const string& bottomText, Font* pBottomFont, AnimationType animateIn, float pauseS, AnimationType animateOut, const string& id) : BackgroundScene(pFlexDMD, pBackground, animateIn, pauseS, animateOut, id)
{
   m_pTopText = new Label(pFlexDMD, pTopFont, topText, string());
   AddActor(m_pTopText);

   m_pBottomText = new Label(pFlexDMD, pBottomFont, bottomText, string());
   AddActor(m_pBottomText);
}

TwoLineScene::~TwoLineScene()
{
   m_pTopText->Remove();
   m_pBottomText->Remove();
}

void TwoLineScene::SetText(const string& topText, const string& bottomText)
{
   m_pTopText->SetText(topText);
   m_pBottomText->SetText(bottomText);
}

void TwoLineScene::Update(float delta)
{
   BackgroundScene::Update(delta);
   m_pTopText->SetPosition((GetWidth() - m_pTopText->GetWidth()) / 2.0f, 4.0f);
   m_pBottomText->SetPosition((GetWidth() - m_pBottomText->GetWidth()) / 2.0f, 15.0f);
}
