#pragma once

#include "common.h"
#include "FlexDMD.h"
#include "scenes/Sequence.h"
#include "actors/ScoreBoard.h"
#include "actors/Image.h"
#include "resources/ResourceDef.h"
#include "VPXPlugin.h"

#include <unordered_map>

class UltraDMD
{
public:
   UltraDMD(FlexDMD* pFlexDMD);
   ~UltraDMD();

   PSC_IMPLEMENT_REFCOUNT()

   void LoadSetup() { /* Unimplemented */ }
   void Init() { m_pFlexDMD->SetRun(true); }
   void Uninit() { m_pFlexDMD->SetRun(false); }
   static int GetMajorVersion() { return 1; }
   static int GetMinorVersion() { return 9999; }
   static int GetBuildNumber() { return 9999; }
   bool SetVisibleVirtualDMD(bool visible) { /* Unimplemented */ return false; }
   bool SetFlipY(bool flipY) { /* Unimplemented */ return false; }
   bool IsRendering() const { return !m_pQueue->IsFinished(); }
   void CancelRendering() { m_pQueue->RemoveAllScenes(); }
   void CancelRenderingWithId(const string& sceneId) { m_pQueue->RemoveScene(sceneId); }
   void Clear();
   void SetProjectFolder(const string& basePath) { m_pFlexDMD->SetProjectFolder(basePath); }
   void SetVideoStretchMode(int mode) { m_stretchMode = mode; }
   void SetScoreboardBackgroundImage(const string& filename, int selectedBrightness, int unselectedBrightness);
   int CreateAnimationFromImages(int fps, bool loop, const string& imagelist);
   int RegisterVideo(int videoStretchMode, bool loop, const string& videoFilename);
   int RegisterFont(const string& file) { /* Unimplemented */ return 0; }
   void UnregisterFont(int registeredFont) { /* Unimplemented */ }
   void DisplayVersionInfo();
   void DisplayScoreboard(int cPlayers, int highlightedPlayer, int score1, int score2, int score3, int score4, const string& lowerLeft, const string& lowerRight);
   void DisplayScoreboard00(int cPlayers, int highlightedPlayer, int score1, int score2, int score3, int score4, const string& lowerLeft, const string& lowerRight);
   void DisplayScene00(const string& background, const string& toptext, int topBrightness, const string& bottomtext, int bottomBrightness, int animateIn, int pauseTime, int animateOut);
   void DisplayScene00Ex(const string& background, const string& toptext, int topBrightness, int topOutlineBrightness, const string& bottomtext, int bottomBrightness, int bottomOutlineBrightness, int animateIn, int pauseTime, int animateOut);
   void DisplayScene00ExWithId(const string& sceneId, bool cancelPrevious, const string& background, const string& toptext, int topBrightness, int topOutlineBrightness, const string& bottomtext, int bottomBrightness, int bottomOutlineBrightness, int animateIn, int pauseTime, int animateOut);
   void ModifyScene00(const string& id, const string& toptext, const string& bottomtext);
   void ModifyScene00Ex(const string& id, const string& toptext, const string& bottomtext, int pauseTime);
   void DisplayScene01(const string& sceneId, const string& background, const string& text, int textBrightness, int textOutlineBrightness, int animateIn, int pauseTime, int animateOut);
   void DisplayText(const string& text, int textBrightness, int textOutlineBrightness);
   void ScrollingCredits(const string& background, const string& text, int textBrightness, int animateIn, int pauseTime, int animateOut);

private:
   Actor* ResolveImage(const string& filename, bool useFrame);
   Font* GetFont(const string& path, float brightness, float outlineBrightness);
   Label* GetFittedLabel(const string& text, float fillBrightness, float outlineBrightness);

   FlexDMD* m_pFlexDMD;
   Sequence* m_pQueue;
   ScoreBoard* m_pScoreBoard;
   FontDef* m_pScoreFontText;
   FontDef* m_pScoreFontNormal;
   FontDef* m_pScoreFontHighlight;
   FontDef* m_pTwoLinesFontTop;
   FontDef* m_pTwoLinesFontBottom;
   vector<FontDef*> m_singleLineFonts;
   bool m_visible = true;
   int m_stretchMode = 0;
   int m_nextId = 1;
   std::unordered_map<int, BaseDef*> m_preloads;
};
