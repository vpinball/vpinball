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

#include "resources/ResourceDef.h"

#include <sstream>

UltraDMD::UltraDMD(FlexDMD* pFlexDMD)
   : m_pFlexDMD(pFlexDMD)
   , m_pScoreFontText(new FontDef("FlexDMD.Resources.udmd-f4by5.fnt"s, RGB(168, 168, 168), RGB(255, 255, 255)))
   , m_pScoreFontNormal(new FontDef("FlexDMD.Resources.udmd-f5by7.fnt"s, RGB(168, 168, 168), RGB(255, 255, 255)))
   , m_pScoreFontHighlight(new FontDef("FlexDMD.Resources.udmd-f6by12.fnt"s, RGB(255, 255, 255), RGB(255, 255, 255)))
   , m_pTwoLinesFontTop(new FontDef("FlexDMD.Resources.udmd-f5by7.fnt"s, RGB(255, 255, 255), RGB(255, 255, 255)))
   , m_pTwoLinesFontBottom( new FontDef("FlexDMD.Resources.udmd-f6by12.fnt"s, RGB(255, 255, 255), RGB(255, 255, 255)))
{
   m_pFlexDMD->AddRef();
   m_pFlexDMD->SetRenderMode(RenderMode_DMD_RGB);

   m_pQueue = new Sequence(m_pFlexDMD, string());
   m_pQueue->SetFillParent(true);

   m_singleLineFonts.push_back(new FontDef("FlexDMD.Resources.udmd-f14by26.fnt"s, RGB(255, 255, 255), RGB(255, 255, 255)));
   m_singleLineFonts.push_back(new FontDef("FlexDMD.Resources.udmd-f12by24.fnt"s, RGB(255, 255, 255), RGB(255, 255, 255)));
   m_singleLineFonts.push_back(new FontDef("FlexDMD.Resources.udmd-f7by13.fnt"s, RGB(255, 255, 255), RGB(255, 255, 255)));

   m_pScoreBoard = new ScoreBoard(m_pFlexDMD,
      m_pFlexDMD->NewFont(m_pScoreFontText->GetPath(), m_pScoreFontText->GetTint(), m_pScoreFontText->GetBorderTint(), m_pScoreFontText->GetBorderSize()),
      m_pFlexDMD->NewFont(m_pScoreFontNormal->GetPath(), m_pScoreFontNormal->GetTint(), m_pScoreFontNormal->GetBorderTint(), m_pScoreFontNormal->GetBorderSize()),
      m_pFlexDMD->NewFont(m_pScoreFontHighlight->GetPath(), m_pScoreFontHighlight->GetTint(), m_pScoreFontHighlight->GetBorderTint(), m_pScoreFontHighlight->GetBorderSize()));
   m_pScoreBoard->SetVisible(false);

   m_pFlexDMD->GetStage()->AddActor(m_pScoreBoard);
   m_pFlexDMD->GetStage()->AddActor(m_pQueue);
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

/*STDMETHODIMP UltraDMD::SetVisibleVirtualDMD(VARIANT_BOOL bHide, VARIANT_BOOL *pRetVal)
{
   bool wasVisible = m_visible;

   m_visible = (bHide == VARIANT_TRUE);

   *pRetVal = wasVisible ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}*/

void UltraDMD::Clear()
{
   VP::SurfaceGraphics* pGraphics = m_pFlexDMD->GetGraphics();
   pGraphics->SetColor(RGB(0, 0, 0));
   pGraphics->Clear();
   m_pScoreBoard->SetVisible(false);
   if (m_pQueue->IsFinished())
      m_pQueue->SetVisible(false);
}

Label* UltraDMD::GetFittedLabel(const string& text, float fillBrightness, float outlineBrightness)
{
   for (const auto& pFontDef : m_singleLineFonts) {
      Label* pLabel = new Label(m_pFlexDMD, GetFont(pFontDef->GetPath(), fillBrightness, outlineBrightness), text, string());
      pLabel->SetPosition((m_pFlexDMD->GetWidth() - pLabel->GetWidth()) / 2.f, (m_pFlexDMD->GetHeight() - pLabel->GetHeight()) / 2.f);
      if ((pLabel->GetX() >= 0 && pLabel->GetY() >= 0) || pFontDef == m_singleLineFonts[m_singleLineFonts.size() - 1])
         return pLabel;
      pLabel->Release();
    }
    return nullptr;
}

Font* UltraDMD::GetFont(const string& path, float brightness, float outlineBrightness)
{
   brightness = brightness > 1.f ? 1.f : brightness;
   outlineBrightness = outlineBrightness > 1.f ? 1.f : outlineBrightness;
   ColorRGBA32 baseColor = m_pFlexDMD->GetRenderMode() == RenderMode_DMD_RGB ? m_pFlexDMD->GetColor() : RGB(255, 255, 255);

   ColorRGBA32 tint = brightness >= 0.0f ? RGB(
      SDL_min((GetRValue(baseColor) * brightness), 255),
      SDL_min((GetGValue(baseColor) * brightness), 255),
      SDL_min((GetBValue(baseColor) * brightness), 255)) : RGB(0, 0, 0);

   if (outlineBrightness >= 0.0f) {
      ColorRGBA32 borderTint = RGB(
         SDL_min((GetRValue(baseColor) * outlineBrightness), 255),
         SDL_min((GetGValue(baseColor) * outlineBrightness), 255),
         SDL_min((GetBValue(baseColor) * outlineBrightness), 255));

      return m_pFlexDMD->NewFont(path, tint, borderTint, 1);
   }

   return m_pFlexDMD->NewFont(path, tint, RGB(255, 255, 255), 0);
}

void UltraDMD::SetScoreboardBackgroundImage(const string& filename, int selectedBrightness, int unselectedBrightness)
{
   m_pScoreBoard->SetBackground(ResolveImage(filename, false));
   m_pScoreBoard->SetFonts(
      GetFont(m_pScoreFontNormal->GetPath(), unselectedBrightness / 15.0f, -1),
      GetFont(m_pScoreFontHighlight->GetPath(), selectedBrightness / 15.0f, -1),
      GetFont(m_pScoreFontText->GetPath(), unselectedBrightness / 15.0f, -1));
}

Actor* UltraDMD::ResolveImage(const string& filename, bool useFrame)
{
   int key;
   std::unordered_map<int, BaseDef*>::iterator k;
   if (try_parse_int(filename, key) && ((k = m_preloads.find(key)) != m_preloads.end())) {
      VideoDef* pVideoDef = dynamic_cast<VideoDef*>(k->second);
      if (pVideoDef) {
         Video* pActor = Video::Create(m_pFlexDMD, pVideoDef->GetVideoFilename(), string(), pVideoDef->GetLoop());
         if (pActor) {
            pActor->SetLoop(pVideoDef->GetLoop());
            pActor->SetScaling(pVideoDef->GetScaling());
            pActor->SetAlignment(pVideoDef->GetAlignment());
         }
         return pActor;
      }
      else {
         ImageSequenceDef* pImageSequenceDef = dynamic_cast<ImageSequenceDef*>(k->second);
         if (pImageSequenceDef) {
            ImageSequence* pVideo = ImageSequence::Create(m_pFlexDMD, m_pFlexDMD->GetAssetManager(), pImageSequenceDef->GetImages(), string(), pImageSequenceDef->GetFPS(), pImageSequenceDef->GetLoop());
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
       std::replace(path.begin(), path.end(), ',', '|');
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

int UltraDMD::CreateAnimationFromImages(int fps, bool loop, const string& imagelist)
{
   int id = m_nextId;
   string szImagelist = imagelist;
   std::replace(szImagelist.begin(), szImagelist.end(), ',', '|');
   m_preloads[id] = new ImageSequenceDef(szImagelist, fps, loop);
   m_nextId++;
   return id;
}

int UltraDMD::RegisterVideo(int videoStretchMode, bool loop, const string& videoFilename)
{
   VideoDef* pVideoDef = new VideoDef(videoFilename, loop);

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
      VideoDef* pObject = dynamic_cast<VideoDef*>(kv.second);
      if (pObject) {
         if (*pObject == *pVideoDef) {
            return kv.first;
         }
      }
   }

   int id = m_nextId;
   m_preloads[id] = pVideoDef;
   m_nextId++;
   return id;
}

void UltraDMD::DisplayVersionInfo()
{
   m_pScoreBoard->SetVisible(false);
   m_pQueue->SetVisible(false);
}

void UltraDMD::DisplayScoreboard(int cPlayers, int highlightedPlayer, int score1, int score2, int score3, int score4, const string& lowerLeft, const string& lowerRight)
{
   m_pScoreBoard->SetNPlayers(cPlayers);
   m_pScoreBoard->SetHighlightedPlayer(highlightedPlayer);
   m_pScoreBoard->SetScore(score1, score2, score3, score4);
   m_pScoreBoard->GetLowerLeft()->SetText(lowerLeft);
   m_pScoreBoard->GetLowerRight()->SetText(lowerRight);
   if (m_pQueue->IsFinished()) {
      m_pQueue->SetVisible(false);
      m_pScoreBoard->SetVisible(true);
   }
}

void UltraDMD::DisplayScoreboard00(int cPlayers, int highlightedPlayer, int score1, int score2, int score3, int score4, const string& lowerLeft, const string& lowerRight)
{
   DisplayScoreboard(cPlayers, highlightedPlayer, score1, score2, score3, score4, lowerLeft, lowerRight);
}

void UltraDMD::DisplayScene00(const string& background, const string& toptext, int topBrightness, const string& bottomtext, int bottomBrightness, int animateIn, int pauseTime, int animateOut)
{
   DisplayScene00ExWithId(string(), false, background, toptext, topBrightness, -15, bottomtext, bottomBrightness, -15, animateIn, pauseTime, animateOut);
}

void UltraDMD::DisplayScene00Ex(const string& background, const string& toptext, int topBrightness, int topOutlineBrightness, const string& bottomtext, int bottomBrightness, int bottomOutlineBrightness,
   int animateIn, int pauseTime, int animateOut)
{
   DisplayScene00ExWithId(string(), false, background, toptext, topBrightness, topOutlineBrightness, bottomtext, bottomBrightness, bottomOutlineBrightness, animateIn, pauseTime, animateOut);
}

void UltraDMD::DisplayScene00ExWithId(const string& sceneId, bool cancelPrevious, const string& background, const string& toptext, int topBrightness, int topOutlineBrightness, const string& bottomtext,
   int bottomBrightness, int bottomOutlineBrightness, int animateIn, int pauseTime, int animateOut)
{
   if (cancelPrevious && !sceneId.empty()) {
      Scene* pScene = m_pQueue->GetActiveScene();
      if (pScene != nullptr && pScene->GetName() == sceneId)
         m_pQueue->RemoveScene(sceneId);
   }
   m_pScoreBoard->SetVisible(false);
   m_pQueue->SetVisible(true);

   if (!toptext.empty() && !bottomtext.empty()) {
      Font* pFontTop = GetFont(m_pTwoLinesFontTop->GetPath(), topBrightness / 15.0f, topOutlineBrightness / 15.0f);
      Font* pFontBottom = GetFont(m_pTwoLinesFontBottom->GetPath(), bottomBrightness / 15.0f, bottomOutlineBrightness / 15.0f);
      TwoLineScene* pScene = new TwoLineScene(m_pFlexDMD, ResolveImage(background, true), toptext, pFontTop, bottomtext, pFontBottom, (AnimationType)animateIn, pauseTime / 1000.0f, (AnimationType)animateOut, sceneId);
      m_pQueue->Enqueue(pScene);
   }
   else if (!toptext.empty()) {
      Label* pLabel = GetFittedLabel(toptext, topBrightness / 15.0f, topOutlineBrightness / 15.0f);
      SingleLineScene* pScene = new SingleLineScene(m_pFlexDMD, ResolveImage(background, true), toptext, pLabel->GetFont(), (AnimationType)animateIn, pauseTime / 1000.0f, (AnimationType)animateOut, false, sceneId);
      pLabel->Release();
      m_pQueue->Enqueue(pScene);
   }
   else if (!bottomtext.empty()) {
      Label* pLabel = GetFittedLabel(bottomtext, bottomBrightness / 15.0f, bottomOutlineBrightness / 15.0f);
      SingleLineScene* pScene = new SingleLineScene(m_pFlexDMD, ResolveImage(background, true), bottomtext, pLabel->GetFont(), (AnimationType)animateIn, pauseTime / 1000.0f, (AnimationType)animateOut, false, sceneId);
      pLabel->Release();
      m_pQueue->Enqueue(pScene);
   }
   else {
      BackgroundScene* pScene = new BackgroundScene(m_pFlexDMD, ResolveImage(background, true), (AnimationType)animateIn, pauseTime / 1000.0f, (AnimationType)animateOut, sceneId);
      m_pQueue->Enqueue(pScene);
   }
}

void UltraDMD::ModifyScene00(const string& id, const string& toptext, const string& bottomtext)
{
   Scene* pScene = m_pQueue->GetActiveScene();
   if (pScene != nullptr && !id.empty() && pScene->GetName() == id) {
      TwoLineScene* pScene2 = dynamic_cast<TwoLineScene*>(pScene);
      if (pScene2 != nullptr)
         pScene2->SetText(toptext, bottomtext);
      SingleLineScene* pScene1 = dynamic_cast<SingleLineScene*>(pScene);
      if (pScene1 != nullptr)
         pScene1->SetText(toptext);
   }
}

void UltraDMD::ModifyScene00Ex(const string& id, const string& toptext, const string& bottomtext, int pauseTime)
{
   Scene* pScene = m_pQueue->GetActiveScene();
   if (pScene != nullptr && !id.empty() && pScene->GetName() == id) {
      TwoLineScene* pScene2 = dynamic_cast<TwoLineScene*>(pScene);
      if (pScene2 != nullptr)
         pScene2->SetText(toptext, bottomtext);
      SingleLineScene* pScene1 = dynamic_cast<SingleLineScene*>(pScene);
      if (pScene1 != nullptr)
         pScene1->SetText(toptext);
      pScene->SetPause((float)(pScene->GetTime() + pauseTime / 1000.0));
   }
}

void UltraDMD::DisplayScene01(const string& sceneId, const string& background, const string& text, int textBrightness, int textOutlineBrightness, int animateIn, int pauseTime, int animateOut)
{
   Font* pFont = GetFont(m_singleLineFonts[0]->GetPath(), textBrightness / 15.0f, textOutlineBrightness / 15.0f);
   SingleLineScene* pScene = new SingleLineScene(m_pFlexDMD, ResolveImage(background, false), text, pFont, (AnimationType)animateIn, pauseTime / 1000.0f, (AnimationType)animateOut, true, sceneId);
   m_pScoreBoard->SetVisible(false);
   m_pQueue->SetVisible(true);
   m_pQueue->Enqueue(pScene);
}

void UltraDMD::DisplayText(const string& text, int textBrightness, int textOutlineBrightness)
{
   m_pScoreBoard->SetVisible(false);
   if (m_pQueue->IsFinished()) {
      m_pQueue->SetVisible(false);
      Label* pLabel = GetFittedLabel(text, textBrightness / 15.0f, textOutlineBrightness / 15.0f);
      pLabel->Draw(m_pFlexDMD->GetGraphics());
      delete pLabel;
   }
}

void UltraDMD::ScrollingCredits(const string& background, const string& text, int textBrightness, int animateIn, int pauseTime, int animateOut)
{
   m_pScoreBoard->SetVisible(false);

   vector<string> lines;
   string szText = text;
   std::replace(szText.begin(), szText.end(), '|', '\n');
   std::stringstream ss(szText);
   string line;
   while (std::getline(ss, line, '\n'))
      lines.push_back(line);

   Font* pFont = GetFont(m_pScoreFontText->GetPath(), textBrightness / 15.0f, -1);
   ScrollingCreditsScene* pScene = new ScrollingCreditsScene(m_pFlexDMD, ResolveImage(background, false), lines, pFont, (AnimationType)animateIn, pauseTime / 1000.0f, (AnimationType)animateOut, "");
   m_pQueue->SetVisible(true);
   m_pQueue->Enqueue(pScene);
}
