#include "stdafx.h"

#include "UltraDMDDMDObject.h"

UltraDMDDMDObject::UltraDMDDMDObject()
{
   m_pFlexDMD = new FlexDMD();
   m_pFlexDMD->AddRef();

   m_pUltraDMD = new UltraDMD(m_pFlexDMD);
   m_pUltraDMD->AddRef();

   LoadSetup();
}

UltraDMDDMDObject::~UltraDMDDMDObject()
{
   m_pUltraDMD->Release();
   m_pFlexDMD->Release();
}

STDMETHODIMP UltraDMDDMDObject::get_FloatProperty(single *pRetVal)
{
   return m_pUltraDMD->get_FloatProperty(pRetVal);
}

STDMETHODIMP UltraDMDDMDObject::put_FloatProperty(single pRetVal)
{
   return m_pUltraDMD->put_FloatProperty(pRetVal);
}

STDMETHODIMP UltraDMDDMDObject::Init()
{
   return m_pUltraDMD->Init();
}

STDMETHODIMP UltraDMDDMDObject::Uninit()
{
   return m_pUltraDMD->Uninit();
}

STDMETHODIMP UltraDMDDMDObject::GetMajorVersion(LONG *pRetVal)
{
   return m_pUltraDMD->GetMajorVersion(pRetVal);
}

STDMETHODIMP UltraDMDDMDObject::GetMinorVersion(LONG *pRetVal)
{
   return m_pUltraDMD->GetMinorVersion(pRetVal);
}

STDMETHODIMP UltraDMDDMDObject::GetBuildNumber(LONG *pRetVal)
{
   return m_pUltraDMD->GetBuildNumber(pRetVal);
}

STDMETHODIMP UltraDMDDMDObject::SetVisibleVirtualDMD(VARIANT_BOOL bHide, VARIANT_BOOL *pRetVal)
{
   return m_pUltraDMD->SetVisibleVirtualDMD(bHide, pRetVal);
}

STDMETHODIMP UltraDMDDMDObject::SetFlipY(VARIANT_BOOL flipY, VARIANT_BOOL *pRetVal)
{
   return m_pUltraDMD->SetFlipY(flipY, pRetVal);
}

STDMETHODIMP UltraDMDDMDObject::IsRendering(VARIANT_BOOL *pRetVal)
{
   return m_pUltraDMD->IsRendering(pRetVal);
}

STDMETHODIMP UltraDMDDMDObject::CancelRendering()
{
   return m_pUltraDMD->CancelRendering();
}

STDMETHODIMP UltraDMDDMDObject::CancelRenderingWithId(BSTR sceneId)
{
   return m_pUltraDMD->CancelRenderingWithId(sceneId);
}

STDMETHODIMP UltraDMDDMDObject::Clear()
{
   return m_pUltraDMD->Clear();
}

STDMETHODIMP UltraDMDDMDObject::SetProjectFolder(BSTR basePath)
{
   return m_pUltraDMD->SetProjectFolder(basePath);
}

STDMETHODIMP UltraDMDDMDObject::SetVideoStretchMode(LONG mode)
{
   return m_pUltraDMD->SetVideoStretchMode(mode);
}

STDMETHODIMP UltraDMDDMDObject::SetScoreboardBackgroundImage(BSTR filename, LONG selectedBrightness, LONG unselectedBrightness)
{
   return m_pUltraDMD->SetScoreboardBackgroundImage(filename, selectedBrightness, unselectedBrightness);
}

STDMETHODIMP UltraDMDDMDObject::CreateAnimationFromImages(LONG fps, VARIANT_BOOL loop, BSTR imagelist, LONG *pRetVal)
{
   return m_pUltraDMD->CreateAnimationFromImages(fps, loop, imagelist, pRetVal);
}

STDMETHODIMP UltraDMDDMDObject::RegisterVideo(LONG videoStretchMode, VARIANT_BOOL loop, BSTR videoFilename, LONG *pRetVal)
{
   return m_pUltraDMD->RegisterVideo(videoStretchMode, loop, videoFilename, pRetVal);
}

STDMETHODIMP UltraDMDDMDObject::RegisterFont(BSTR file, LONG *pRetVal)
{
   return m_pUltraDMD->RegisterFont(file, pRetVal);
}

STDMETHODIMP UltraDMDDMDObject::UnregisterFont(LONG registeredFont)
{
   return m_pUltraDMD->UnregisterFont(registeredFont);
}

STDMETHODIMP UltraDMDDMDObject::DisplayVersionInfo()
{
   return m_pUltraDMD->DisplayVersionInfo();
}

STDMETHODIMP UltraDMDDMDObject::DisplayScoreboard(LONG cPlayers, LONG highlightedPlayer, LONG score1, LONG score2, LONG score3, LONG score4, BSTR lowerLeft, BSTR lowerRight)
{
   return m_pUltraDMD->DisplayScoreboard(cPlayers, highlightedPlayer, score1, score2, score3, score4, lowerLeft, lowerRight);
}

STDMETHODIMP UltraDMDDMDObject::DisplayScoreboard00(LONG cPlayers, LONG highlightedPlayer, LONG score1, LONG score2, LONG score3, LONG score4, BSTR lowerLeft, BSTR lowerRight)
{
   return m_pUltraDMD->DisplayScoreboard00(cPlayers, highlightedPlayer, score1, score2, score3, score4, lowerLeft, lowerRight);
}

STDMETHODIMP UltraDMDDMDObject::DisplayScene00(BSTR background, BSTR toptext, LONG topBrightness, BSTR bottomtext, LONG bottomBrightness, LONG animateIn, LONG pauseTime, LONG animateOut)
{
   return m_pUltraDMD->DisplayScene00(background, toptext, topBrightness, bottomtext, bottomBrightness, animateIn, pauseTime, animateOut);
}

STDMETHODIMP UltraDMDDMDObject::DisplayScene00Ex(BSTR background, BSTR toptext, LONG topBrightness, LONG topOutlineBrightness, BSTR bottomtext, LONG bottomBrightness, LONG bottomOutlineBrightness, LONG animateIn, LONG pauseTime, LONG animateOut)
{
   return m_pUltraDMD->DisplayScene00Ex(background, toptext, topBrightness, topOutlineBrightness, bottomtext, bottomBrightness, bottomOutlineBrightness, animateIn, pauseTime, animateOut);
}

STDMETHODIMP UltraDMDDMDObject::DisplayScene00ExWithId(BSTR sceneId, VARIANT_BOOL cancelPrevious, BSTR background, BSTR toptext, LONG topBrightness, LONG topOutlineBrightness, BSTR bottomtext, LONG bottomBrightness, LONG bottomOutlineBrightness, LONG animateIn, LONG pauseTime, LONG animateOut)
{
   return m_pUltraDMD->DisplayScene00ExWithId(sceneId, cancelPrevious, background, toptext, topBrightness, topOutlineBrightness, bottomtext, bottomBrightness, bottomOutlineBrightness, animateIn, pauseTime, animateOut);
}

STDMETHODIMP UltraDMDDMDObject::ModifyScene00(BSTR id, BSTR toptext, BSTR bottomtext)
{
   return m_pUltraDMD->ModifyScene00(id, toptext, bottomtext);
}

STDMETHODIMP UltraDMDDMDObject::ModifyScene00Ex(BSTR id, BSTR toptext, BSTR bottomtext, LONG pauseTime)
{
   return m_pUltraDMD->ModifyScene00Ex(id, toptext, bottomtext, pauseTime);
}

STDMETHODIMP UltraDMDDMDObject::DisplayScene01(BSTR sceneId, BSTR background, BSTR text, LONG textBrightness, LONG textOutlineBrightness, LONG animateIn, LONG pauseTime, LONG animateOut)
{
   return m_pUltraDMD->DisplayScene01(sceneId, background, text, textBrightness, textOutlineBrightness, animateIn, pauseTime, animateOut);
}

STDMETHODIMP UltraDMDDMDObject::DisplayText(BSTR text, LONG textBrightness, LONG textOutlineBrightness)
{
   return m_pUltraDMD->DisplayText(text, textBrightness, textOutlineBrightness);
}

STDMETHODIMP UltraDMDDMDObject::ScrollingCredits(BSTR background, BSTR text, LONG textBrightness, LONG animateIn, LONG pauseTime, LONG animateOut)
{
   return m_pUltraDMD->ScrollingCredits(background, text, textBrightness, animateIn, pauseTime, animateOut);
}

STDMETHODIMP UltraDMDDMDObject::LoadSetup()
{
   return m_pUltraDMD->LoadSetup();
}

STDMETHODIMP UltraDMDDMDObject::get_DmdColoredPixels(VARIANT *pRetVal)
{
   return m_pFlexDMD->get_DmdColoredPixels(pRetVal);
}

STDMETHODIMP UltraDMDDMDObject::get_DmdPixels(VARIANT *pRetVal)
{
   return m_pFlexDMD->get_DmdPixels(pRetVal);
}

STDMETHODIMP UltraDMDDMDObject::get_Width(unsigned short *pRetVal)
{
   return m_pFlexDMD->get_Width(pRetVal);
}

STDMETHODIMP UltraDMDDMDObject::get_Height(unsigned short *pRetVal)
{
   return m_pFlexDMD->get_Height(pRetVal);
}
