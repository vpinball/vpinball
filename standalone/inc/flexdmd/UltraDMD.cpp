#include "core/stdafx.h"

#include "UltraDMD.h"
#include "scenes/SingleLineScene.h"
#include "scenes/TwoLineScene.h"
#include "scenes/BackgroundScene.h"
#include "scenes/ScrollingCreditsScene.h"
#include "actors/Label.h"
#include "actors/Frame.h"
#include "actors/GIFImage.h"
#include "actors/Video.h"
#include "actors/ImageSequence.h"

#include "ImageSequenceDef.h"
#include "VideoDef.h"

UltraDMD::UltraDMD(FlexDMD* pFlexDMD)
{
   m_visible = true;
   m_stretchMode = 0;
   m_nextId = 1;

   m_pFlexDMD = pFlexDMD;
   m_pFlexDMD->AddRef();

   m_pFlexDMD->SetRenderMode(RenderMode_DMD_RGB);

   m_pQueue = new Sequence(m_pFlexDMD, string());
   m_pQueue->SetFillParent(true);

   m_pScoreFontText = new FontDef("FlexDMD.Resources.udmd-f4by5.fnt"s, RGB(168, 168, 168), RGB(255, 255, 255));
   m_pScoreFontNormal = new FontDef("FlexDMD.Resources.udmd-f5by7.fnt"s, RGB(168, 168, 168), RGB(255, 255, 255));
   m_pScoreFontHighlight = new FontDef("FlexDMD.Resources.udmd-f6by12.fnt"s, RGB(255, 255, 255), RGB(255, 255, 255));

   m_singleLineFonts.push_back(new FontDef("FlexDMD.Resources.udmd-f14by26.fnt"s, RGB(255, 255, 255), RGB(255, 255, 255)));
   m_singleLineFonts.push_back(new FontDef("FlexDMD.Resources.udmd-f12by24.fnt"s, RGB(255, 255, 255), RGB(255, 255, 255)));
   m_singleLineFonts.push_back(new FontDef("FlexDMD.Resources.udmd-f7by13.fnt"s, RGB(255, 255, 255), RGB(255, 255, 255)));

   m_pTwoLinesFontTop = new FontDef("FlexDMD.Resources.udmd-f5by7.fnt"s, RGB(255, 255, 255), RGB(255, 255, 255));
   m_pTwoLinesFontBottom = new FontDef("FlexDMD.Resources.udmd-f6by12.fnt"s, RGB(255, 255, 255), RGB(255, 255, 255));

   m_pScoreBoard = new ScoreBoard(m_pFlexDMD,
      m_pFlexDMD->NewFont(m_pScoreFontText->GetPath(), m_pScoreFontText->GetTint(), m_pScoreFontText->GetBorderTint(), m_pScoreFontText->GetBorderSize()),
      m_pFlexDMD->NewFont(m_pScoreFontNormal->GetPath(), m_pScoreFontNormal->GetTint(), m_pScoreFontNormal->GetBorderTint(), m_pScoreFontNormal->GetBorderSize()),
      m_pFlexDMD->NewFont(m_pScoreFontHighlight->GetPath(), m_pScoreFontHighlight->GetTint(), m_pScoreFontHighlight->GetBorderTint(), m_pScoreFontHighlight->GetBorderSize()));

   m_pScoreBoard->SetVisible(false);

   m_pFlexDMD->GetStage()->AddActor((Actor*)m_pScoreBoard);
   m_pFlexDMD->GetStage()->AddActor((Actor*)m_pQueue);
}

UltraDMD::~UltraDMD()
{
   m_pFlexDMD->Release();

   delete m_pScoreFontText;
   delete m_pScoreFontNormal;
   delete m_pScoreFontHighlight;

   for (const auto& pFontDef : m_singleLineFonts)
      delete pFontDef;

   delete m_pTwoLinesFontTop;
   delete m_pTwoLinesFontBottom;
}

STDMETHODIMP UltraDMD::LoadSetup()
{
   return S_OK;
}

STDMETHODIMP UltraDMD::get_FloatProperty(single *pRetVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP UltraDMD::put_FloatProperty(single pRetVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP UltraDMD::Init()
{
   return m_pFlexDMD->put_Run(VARIANT_TRUE);
}

STDMETHODIMP UltraDMD::Uninit()
{
   return m_pFlexDMD->put_Run(VARIANT_FALSE);
}

STDMETHODIMP UltraDMD::GetMajorVersion(LONG *pRetVal)
{
   *pRetVal = 1;

   return S_OK;
}

STDMETHODIMP UltraDMD::GetMinorVersion(LONG *pRetVal)
{
   *pRetVal = 9999;

   return S_OK;
}

STDMETHODIMP UltraDMD::GetBuildNumber(LONG *pRetVal)
{
   *pRetVal = 9999;

   return S_OK;
}

STDMETHODIMP UltraDMD::SetVisibleVirtualDMD(VARIANT_BOOL bHide, VARIANT_BOOL *pRetVal)
{
   bool wasVisible = m_visible;

   m_visible = (bHide == VARIANT_TRUE);

   *pRetVal = wasVisible ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP UltraDMD::SetFlipY(VARIANT_BOOL flipY, VARIANT_BOOL *pRetVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP UltraDMD::IsRendering(VARIANT_BOOL *pRetVal)
{
   *pRetVal = !m_pQueue->IsFinished() ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP UltraDMD::CancelRendering()
{
   m_pQueue->RemoveAllScenes();

   return S_OK;
}

STDMETHODIMP UltraDMD::CancelRenderingWithId(BSTR sceneId)
{
   m_pQueue->RemoveScene(MakeString(sceneId));

   return S_OK;
}

STDMETHODIMP UltraDMD::Clear()
{
   VP::SurfaceGraphics* pGraphics = m_pFlexDMD->GetGraphics();
   pGraphics->SetColor(RGB(0, 0, 0));
   pGraphics->Clear();
   m_pScoreBoard->SetVisible(false);
   if (m_pQueue->IsFinished())
      m_pQueue->SetVisible(false);

   return S_OK;
}

STDMETHODIMP UltraDMD::SetProjectFolder(BSTR basePath)
{
   return m_pFlexDMD->put_ProjectFolder(basePath);
}

STDMETHODIMP UltraDMD::SetVideoStretchMode(LONG mode)
{
   m_stretchMode = mode;

   return S_OK;
}

Label* UltraDMD::GetFittedLabel(const string& text, float fillBrightness, float outlineBrightness)
{
   for (const auto& pFontDef : m_singleLineFonts) {
      Label* pLabel = new Label(m_pFlexDMD, GetFont(pFontDef->GetPath(), fillBrightness, outlineBrightness), text, string());
      pLabel->SetPosition((m_pFlexDMD->GetWidth() - pLabel->GetWidth()) / 2.f, (m_pFlexDMD->GetHeight() - pLabel->GetHeight()) / 2.f);
      if ((pLabel->GetX() >= 0.f && pLabel->GetY() >= 0.f) || pFontDef == m_singleLineFonts[m_singleLineFonts.size() - 1])
         return pLabel;
      delete pLabel;
    }
    return nullptr;
}

Font* UltraDMD::GetFont(const string& path, float brightness, float outlineBrightness) const
{
   brightness = brightness > 1.f ? 1.f : brightness;
   outlineBrightness = outlineBrightness > 1.f ? 1.f : outlineBrightness;
   OLE_COLOR baseColor = m_pFlexDMD->GetRenderMode() == RenderMode_DMD_RGB ? m_pFlexDMD->GetDMDColor() : RGB(255, 255, 255);

   OLE_COLOR tint = brightness >= 0.0f ? RGB(
      min(GetRValue(baseColor) * brightness, 255.f),
      min(GetGValue(baseColor) * brightness, 255.f),
      min(GetBValue(baseColor) * brightness, 255.f)) : RGB(0, 0, 0);

   if (outlineBrightness >= 0.0f) {
      OLE_COLOR borderTint = RGB(
         min(GetRValue(baseColor) * outlineBrightness, 255.f),
         min(GetGValue(baseColor) * outlineBrightness, 255.f),
         min(GetBValue(baseColor) * outlineBrightness, 255.f));

      return m_pFlexDMD->NewFont(path, tint, borderTint, 1);
   }

   return m_pFlexDMD->NewFont(path, tint, RGB(255, 255, 255), 0);
}

STDMETHODIMP UltraDMD::SetScoreboardBackgroundImage(BSTR filename, LONG selectedBrightness, LONG unselectedBrightness)
{
   m_pScoreBoard->SetBackground(ResolveImage(MakeString(filename), false));

   m_pScoreBoard->SetFonts(
      GetFont(m_pScoreFontNormal->GetPath(), unselectedBrightness / 15.0f, -1),
      GetFont(m_pScoreFontHighlight->GetPath(), selectedBrightness / 15.0f, -1),
      GetFont(m_pScoreFontText->GetPath(), unselectedBrightness / 15.0f, -1));

   return S_OK;
}

Actor* UltraDMD::ResolveImage(const string& filename, bool useFrame)
{
   int key;
   if (try_parse_int(filename, key) && m_preloads.find(key) != m_preloads.end()) {
      const VideoDef* const pVideoDef = dynamic_cast<const VideoDef*>(m_preloads[key]);
      if (pVideoDef) {
         Video* const pActor = Video::Create(m_pFlexDMD, pVideoDef->GetVideoFilename(), string(), pVideoDef->GetLoop());
         if (pActor) {
            pActor->SetLoop(pVideoDef->GetLoop());
            pActor->SetScaling(pVideoDef->GetScaling());
            pActor->SetAlignment(pVideoDef->GetAlignment());
         }
         return pActor;
      }
      else {
         const ImageSequenceDef* const pImageSequenceDef = dynamic_cast<const ImageSequenceDef*>(m_preloads[key]);
         if (pImageSequenceDef) {
            ImageSequence* const pVideo = ImageSequence::Create(m_pFlexDMD, m_pFlexDMD->GetAssetManager(), pImageSequenceDef->GetImages(), string(), pImageSequenceDef->GetFPS(), pImageSequenceDef->GetLoop());
            if (pVideo) {
               pVideo->SetFPS(pImageSequenceDef->GetFPS());
               pVideo->SetLoop(pImageSequenceDef->GetLoop());
               pVideo->SetScaling(pImageSequenceDef->GetScaling());
               pVideo->SetAlignment(pImageSequenceDef->GetAlignment());
            }
            return pVideo;
         }
      }
   }
   else {
       string path = filename;
       std::ranges::replace(path.begin(), path.end(), ',', '|');
       if (path.find('|') != string::npos)
          return ImageSequence::Create(m_pFlexDMD, m_pFlexDMD->GetAssetManager(), path, string(), 30, true);
       else {
          AssetSrc* pAssetSrc = m_pFlexDMD->GetAssetManager()->ResolveSrc(path, nullptr);
          AssetType assetType = pAssetSrc->GetAssetType();
          delete pAssetSrc;

          if (assetType == AssetType_Image)
             return Image::Create(m_pFlexDMD, m_pFlexDMD->GetAssetManager(), path, string());
          else if (assetType == AssetType_Video || assetType == AssetType_GIF) {
             AnimatedActor* pActor = m_pFlexDMD->NewVideo(path, string());
             if (pActor != nullptr) {
                switch (m_stretchMode) {
                   case 0:
                      pActor->SetScaling(Scaling_Stretch);
                      pActor->SetAlignment(Alignment_Center);
                      break;
                   case 1:
                      pActor->SetScaling(Scaling_FillX);
                      pActor->SetAlignment(Alignment_Top);
                      break;
                   case 2:
                      pActor->SetScaling(Scaling_FillX);
                      pActor->SetAlignment(Alignment_Center);
                      break;
                   case 3:
                      pActor->SetScaling(Scaling_FillX);
                      pActor->SetAlignment(Alignment_Bottom);
                      break;
                }
                return pActor;
             }
          }
       }
   }

   return useFrame ? new Frame(m_pFlexDMD, string()) : new Actor(m_pFlexDMD, string());
}

STDMETHODIMP UltraDMD::CreateAnimationFromImages(LONG fps, VARIANT_BOOL loop, BSTR imagelist, LONG *pRetVal)
{
   int id = m_nextId;

   string szImagelist = MakeString(imagelist);
   std::ranges::replace(szImagelist.begin(), szImagelist.end(), ',', '|');

   m_preloads[id] = new ImageSequenceDef(szImagelist, fps, loop == VARIANT_TRUE);

   m_nextId++;

   *pRetVal = id;

   return S_OK;
}

STDMETHODIMP UltraDMD::RegisterVideo(LONG videoStretchMode, VARIANT_BOOL loop, BSTR videoFilename, LONG *pRetVal)
{
   VideoDef* const pVideoDef = new VideoDef(MakeString(videoFilename), loop == VARIANT_TRUE);

   switch (videoStretchMode) {
      case 0:
         pVideoDef->SetScaling(Scaling_Stretch);
         pVideoDef->SetAlignment(Alignment_Center);
         break;
      case 1:
         pVideoDef->SetScaling(Scaling_FillX);
         pVideoDef->SetAlignment(Alignment_Top);
         break;
      case 2:
         pVideoDef->SetScaling(Scaling_FillX);
         pVideoDef->SetAlignment(Alignment_Center);
         break;
      case 3:
         pVideoDef->SetScaling(Scaling_FillX);
         pVideoDef->SetAlignment(Alignment_Bottom);
         break;
   }

   for (const auto& kv : m_preloads) {
      VideoDef* const pObject = dynamic_cast<VideoDef*>(kv.second);
      if (pObject) {
         if (*pObject == *pVideoDef) {
            *pRetVal = kv.first;
            return S_OK;
         }
      }
   }

   int id = m_nextId;

   m_preloads[id] = pVideoDef;

   m_nextId++;

   *pRetVal = id;

   return S_OK;
}

STDMETHODIMP UltraDMD::RegisterFont(BSTR file, LONG *pRetVal)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP UltraDMD::UnregisterFont(LONG registeredFont)
{
   PLOGW << "Not implemented";

   return S_OK;
}

STDMETHODIMP UltraDMD::DisplayVersionInfo()
{
   m_pScoreBoard->SetVisible(false);
   m_pQueue->SetVisible(false);

   return S_OK;
}

STDMETHODIMP UltraDMD::DisplayScoreboard(LONG cPlayers, LONG highlightedPlayer, LONG score1, LONG score2, LONG score3, LONG score4, BSTR lowerLeft, BSTR lowerRight)
{
   m_pScoreBoard->SetNPlayers(cPlayers);
   m_pScoreBoard->SetHighlightedPlayer(highlightedPlayer);
   m_pScoreBoard->SetScore(score1, score2, score3, score4);
   m_pScoreBoard->GetLowerLeft()->SetText(MakeString(lowerLeft));
   m_pScoreBoard->GetLowerRight()->SetText(MakeString(lowerRight));
   if (m_pQueue->IsFinished()) {
      m_pQueue->SetVisible(false);
      m_pScoreBoard->SetVisible(true);
   }

   return S_OK;
}

STDMETHODIMP UltraDMD::DisplayScoreboard00(LONG cPlayers, LONG highlightedPlayer, LONG score1, LONG score2, LONG score3, LONG score4, BSTR lowerLeft, BSTR lowerRight)
{
   return DisplayScoreboard(cPlayers, highlightedPlayer, score1, score2, score3, score4, lowerLeft, lowerRight);
}

STDMETHODIMP UltraDMD::DisplayScene00(BSTR background, BSTR toptext, LONG topBrightness, BSTR bottomtext, LONG bottomBrightness, LONG animateIn, LONG pauseTime, LONG animateOut)
{
   BSTR sceneId = SysAllocString(L"");
   HRESULT hres = DisplayScene00ExWithId(sceneId, false, background, toptext, topBrightness, -15, bottomtext, bottomBrightness, -15, animateIn, pauseTime, animateOut);
   SysFreeString(sceneId);

   return hres;
}

STDMETHODIMP UltraDMD::DisplayScene00Ex(BSTR background, BSTR toptext, LONG topBrightness, LONG topOutlineBrightness, BSTR bottomtext, LONG bottomBrightness, LONG bottomOutlineBrightness, LONG animateIn, LONG pauseTime, LONG animateOut)
{
   BSTR sceneId = SysAllocString(L"");
   HRESULT hres = DisplayScene00ExWithId(sceneId, false, background, toptext, topBrightness, topOutlineBrightness, bottomtext, bottomBrightness, bottomOutlineBrightness, animateIn, pauseTime, animateOut);
   SysFreeString(sceneId);

   return hres;
}

STDMETHODIMP UltraDMD::DisplayScene00ExWithId(BSTR sceneId, VARIANT_BOOL cancelPrevious, BSTR background, BSTR toptext, LONG topBrightness, LONG topOutlineBrightness, BSTR bottomtext, LONG bottomBrightness, LONG bottomOutlineBrightness, LONG animateIn, LONG pauseTime, LONG animateOut)
{
   string szSceneId = (sceneId != nullptr) ? MakeString(sceneId) : string();

   if (cancelPrevious == VARIANT_TRUE && !szSceneId.empty()) {
      Scene* pScene = m_pQueue->GetActiveScene();
      if (pScene != nullptr && pScene->GetName() == szSceneId)
         m_pQueue->RemoveScene(szSceneId);
   }
   m_pScoreBoard->SetVisible(false);
   m_pQueue->SetVisible(true);

   string szTopText = (toptext != nullptr) ? MakeString(toptext) : string();
   string szBottomText = (bottomtext != nullptr) ? MakeString(bottomtext) : string();

   if (!szTopText.empty() && !szBottomText.empty()) {
      Font* const pFontTop = GetFont(m_pTwoLinesFontTop->GetPath(), topBrightness / 15.0f, topOutlineBrightness / 15.0f);
      Font* const pFontBottom = GetFont(m_pTwoLinesFontBottom->GetPath(), bottomBrightness / 15.0f, bottomOutlineBrightness / 15.0f);
      TwoLineScene* pScene = new TwoLineScene(m_pFlexDMD, ResolveImage(MakeString(background), true), szTopText, pFontTop, szBottomText, pFontBottom, (AnimationType)animateIn, pauseTime / 1000.0f, (AnimationType)animateOut, szSceneId);
      m_pQueue->Enqueue(pScene);
   }
   else if (!szTopText.empty()) {
      Label* const pLabel = GetFittedLabel(szTopText, topBrightness / 15.0f, topOutlineBrightness / 15.0f);
      pLabel->AddRef();
      SingleLineScene* pScene = new SingleLineScene(m_pFlexDMD, ResolveImage(MakeString(background), true), szTopText, pLabel->GetFont(), (AnimationType)animateIn, pauseTime / 1000.0f, (AnimationType)animateOut, false, szSceneId);
      pLabel->Release();
      m_pQueue->Enqueue(pScene);
   }
   else if (!szBottomText.empty()) {
      Label* const pLabel = GetFittedLabel(szBottomText, bottomBrightness / 15.0f, bottomOutlineBrightness / 15.0f);
      pLabel->AddRef();
      SingleLineScene* pScene = new SingleLineScene(m_pFlexDMD, ResolveImage(MakeString(background), true), szBottomText, pLabel->GetFont(), (AnimationType)animateIn, pauseTime / 1000.0f, (AnimationType)animateOut, false, szSceneId);
      pLabel->Release();
      m_pQueue->Enqueue(pScene);
   }
   else {
      BackgroundScene* const pScene = new BackgroundScene(m_pFlexDMD, ResolveImage(MakeString(background), true), (AnimationType)animateIn, pauseTime / 1000.0f, (AnimationType)animateOut, szSceneId);
      m_pQueue->Enqueue(pScene);
   }

   return S_OK;
}

STDMETHODIMP UltraDMD::ModifyScene00(BSTR id, BSTR toptext, BSTR bottomtext)
{
   string szId = (id != NULL) ? MakeString(id) : string();

   Scene* const pScene = m_pQueue->GetActiveScene();
   if (pScene != nullptr && !szId.empty() && pScene->GetName() == szId) {
      TwoLineScene* const pScene2 = dynamic_cast<TwoLineScene*>(pScene);
      if (pScene2 != nullptr)
         pScene2->SetText(MakeString(toptext), MakeString(bottomtext));
      SingleLineScene* const pScene1 = dynamic_cast<SingleLineScene*>(pScene);
      if (pScene1 != nullptr)
         pScene1->SetText(MakeString(toptext));
   }

   return S_OK;
}

STDMETHODIMP UltraDMD::ModifyScene00Ex(BSTR id, BSTR toptext, BSTR bottomtext, LONG pauseTime)
{
   string szId = (id != NULL) ? MakeString(id) : string();

   Scene* const pScene = m_pQueue->GetActiveScene();
   if (pScene != nullptr && !szId.empty() && pScene->GetName() == szId) {
      TwoLineScene* const pScene2 = dynamic_cast<TwoLineScene*>(pScene);
      if (pScene2 != nullptr)
         pScene2->SetText(MakeString(toptext), MakeString(bottomtext));
      SingleLineScene* const pScene1 = dynamic_cast<SingleLineScene*>(pScene);
      if (pScene1 != nullptr)
         pScene1->SetText(MakeString(toptext));
      pScene->SetPause((float)(pScene->GetTime() + pauseTime / 1000.0));
   }

   return S_OK;
}

STDMETHODIMP UltraDMD::DisplayScene01(BSTR sceneId, BSTR background, BSTR text, LONG textBrightness, LONG textOutlineBrightness, LONG animateIn, LONG pauseTime, LONG animateOut)
{
   string szSceneId = (sceneId != NULL) ? MakeString(sceneId) : string();

   Font* const pFont = GetFont(m_singleLineFonts[0]->GetPath(), textBrightness / 15.0f, textOutlineBrightness / 15.0f);
   SingleLineScene* const pScene = new SingleLineScene(m_pFlexDMD, ResolveImage(MakeString(background), false), MakeString(text), pFont, (AnimationType)animateIn, pauseTime / 1000.0f, (AnimationType)animateOut, true, szSceneId);
   m_pScoreBoard->SetVisible(false);
   m_pQueue->SetVisible(true);
   m_pQueue->Enqueue(pScene);

   return S_OK;
}

STDMETHODIMP UltraDMD::DisplayText(BSTR text, LONG textBrightness, LONG textOutlineBrightness)
{
   m_pScoreBoard->SetVisible(false);
   if (m_pQueue->IsFinished()) {
      m_pQueue->SetVisible(false);
      Label* const pLabel = GetFittedLabel(MakeString(text), textBrightness / 15.0f, textOutlineBrightness / 15.0f);
      pLabel->Draw(m_pFlexDMD->GetGraphics());
      delete pLabel;
   }

   return S_OK;
}

STDMETHODIMP UltraDMD::ScrollingCredits(BSTR background, BSTR text, LONG textBrightness, LONG animateIn, LONG pauseTime, LONG animateOut)
{
   m_pScoreBoard->SetVisible(false);

   vector<string> lines;

   string szText = MakeString(text);
   std::ranges::replace(szText.begin(), szText.end(), '|', '\n');

   std::stringstream ss(szText);
   string line;

   while (std::getline(ss, line, '\n'))
      lines.push_back(line);

   Font* pFont = GetFont(m_pScoreFontText->GetPath(), textBrightness / 15.0f, -1);
   ScrollingCreditsScene* pScene = new ScrollingCreditsScene(m_pFlexDMD, ResolveImage(MakeString(background), false), lines, pFont, (AnimationType)animateIn, pauseTime / 1000.0f, (AnimationType)animateOut, "");
   m_pQueue->SetVisible(true);
   m_pQueue->Enqueue(pScene);

   return S_OK;
}
