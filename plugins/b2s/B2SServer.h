// license:GPLv3+

#pragma once

#include "common.h"

#include <cstdint>
#include <climits>
#include <future>

#include <unordered_dense.h>

#include "B2SDataModel.h"
#include "B2SRenderer.h"

namespace B2S {

class B2SServer final
{
public:
   B2SServer(const MsgPluginAPI* const msgApi, unsigned int endpointId, const VPXPluginAPI* const vpxApi, ScriptClassDef* pinmameClassDef);
   ~B2SServer();

   PSC_IMPLEMENT_REFCOUNT()

   void Dispose() { }
   string GetB2SServerVersion() const { return ""s; }
   double GetB2SBuildVersion() const { return 0.0; }
   string GetB2SServerDirectory() const { return ""s; }
   string GetVPMBuildVersion() const { return ""s; }
   string GetB2SName() const { return ""s; }
   void SetB2SName(const std::string& b2sName) { }
   string GetTableName() const { return ""s; }
   void SetTableName(const std::string& tableName) { }
   void SetWorkingDir(const std::string& workingDir) { }
   void SetPath(const std::string& path) { }
   bool GetLaunchBackglass() const { return false; }
   void SetLaunchBackglass(bool launchBackglass) { }
   bool GetLockDisplay() const { return false; }
   void SetLockDisplay(bool lockDisplay) { }
   bool GetPuPHide() const { return false; }
   void SetPuPHide(bool puPHide) { }

   void B2SSetPos(int id, int x, int y) { } // FIXME
   void B2SSetPos(string name, int x, int y) { } // FIXME
   void B2SSetIllumination(string, int) { } // FIXME
   void B2SSetLED(int, int) { } // FIXME
   void B2SSetLED(int, string) { } // FIXME
   void B2SSetLEDDisplay(int, string) { } // FIXME
   void B2SSetReel(int, int) { } // FIXME
   void B2SSetScore(int, int) { } // FIXME
   void B2SSetScorePlayer(int playerno, int score);
   void B2SSetScorePlayer1(int score) { B2SSetScorePlayer(1, score); }
   void B2SSetScorePlayer2(int score) { B2SSetScorePlayer(2, score); }
   void B2SSetScorePlayer3(int score) { B2SSetScorePlayer(3, score); }
   void B2SSetScorePlayer4(int score) { B2SSetScorePlayer(4, score); }
   void B2SSetScorePlayer5(int score) { B2SSetScorePlayer(5, score); }
   void B2SSetScorePlayer6(int score) { B2SSetScorePlayer(6, score); }
   void B2SSetScoreDigit(int digit, int value);

   void B2SSetData(int id, int value);
   void B2SSetData(const std::string& group, int value);
   void B2SPulseData(int id)                   { B2SSetData(id, 1); B2SSetData(id, 0); }
   void B2SPulseData(const std::string& group) { B2SSetData(group, 1); B2SSetData(group, 0); }
   void B2SSetScoreRollover(int id, int value) { B2SSetData(id, value); }
   void B2SSetScoreRolloverPlayer1(int value)  { B2SSetData(25, value); }
   void B2SSetScoreRolloverPlayer2(int value)  { B2SSetData(26, value); }
   void B2SSetScoreRolloverPlayer3(int value)  { B2SSetData(27, value); }
   void B2SSetScoreRolloverPlayer4(int value)  { B2SSetData(28, value); }
   void B2SSetCredits(int value)               { B2SSetData(29, value); }
   void B2SSetCredits(int id, int value)       { B2SSetData(id, value); }
   void B2SSetPlayerUp(int value)              { B2SSetData(30, value); }
   void B2SSetPlayerUp(int id, int value)      { B2SSetData(id, value); }
   void B2SSetCanPlay(int value)               { B2SSetData(31, value); }
   void B2SSetCanPlay(int id, int value)       { B2SSetData(id, value); }
   void B2SSetBallInPlay(int value)            { B2SSetData(32, value); }
   void B2SSetBallInPlay(int id, int value)    { B2SSetData(id, value); }
   void B2SSetTilt(int value)                  { B2SSetData(33, value); }
   void B2SSetTilt(int id, int value)          { B2SSetData(id, value); }
   void B2SSetMatch(int value)                 { B2SSetData(34, value); }
   void B2SSetMatch(int id, int value)         { B2SSetData(id, value); }
   void B2SSetGameOver(int value)              { B2SSetData(35, value); }
   void B2SSetGameOver(int id, int value)      { B2SSetData(id, value); }
   void B2SSetShootAgain(int value)            { B2SSetData(36, value); }
   void B2SSetShootAgain(int id, int value)    { B2SSetData(id, value); }

   void B2SStartAnimation(const string& animationName, bool reverse = false) { StartAnimation(animationName, reverse); }
   void B2SStartAnimationReverse(const string& animationName)                { StartAnimation(animationName, true); }
   void B2SStopAnimation(const string& animationName)                        { StopAnimation(animationName); }
   void B2SStopAllAnimations() { } // FIXME
   bool GetB2SIsAnimationRunning(const string& animationName) const { return false; } // FIXME
   void StartAnimation(const string& animationName, bool reverse = false) { } // FIXME
   void StopAnimation(const string& animationName) { } // FIXME
   void B2SStartRotation() { } // FIXME
   void B2SStopRotation() { } // FIXME
   void B2SShowScoreDisplays() { } // FIXME
   void B2SHideScoreDisplays() { } // FIXME
   void B2SStartSound(const string& soundName) { } // FIXME
   void B2SPlaySound(const string& soundName) { } // FIXME
   void B2SStopSound(const string& soundName) { } // FIXME
   void B2SMapSound(int digit, const string& soundName) { } // FIXME

   void ForwardPinMAMECall(int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet);

   void SetOnDestroyHandler(std::function<void(B2SServer*)> handler) { m_onDestroyHandler = handler; }
   float GetState(int b2sId) const;
   float GetScoreDigit(int digit) const;
   int GetPlayerScore(int player) const;

private:
   const MsgPluginAPI* const m_msgApi;
   const unsigned int m_endpointId;
   const VPXPluginAPI* const m_vpxApi;
   const unsigned int m_onGetAuxRendererId;
   const unsigned int m_onAuxRendererChgId;

   std::future<std::shared_ptr<B2STable>> m_loadedB2S;
   std::unique_ptr<B2SRenderer> m_renderer = nullptr;

   const ScriptClassDef* m_pinmameClassDef;
   void* const m_pinmame;

   std::function<void(B2SServer*)> m_onDestroyHandler;
   ankerl::unordered_dense::map<int, float> m_states;
   
   ankerl::unordered_dense::map<int, int> m_playerScores;
   ankerl::unordered_dense::map<int, float> m_scoreDigits;

   static int OnRender(VPXRenderContext2D* ctx, void*);
   static void OnGetRenderer(const unsigned int, void*, void* msgData);
};

}
