// license:GPLv3+

#pragma once

#include "common.h"

#include <cstdint>
#include <climits>
#include <future>
#include <cstdint>

#include <unordered_dense.h>

#include "B2SDataModel.h"
#include "B2SRenderer.h"

namespace B2S {

class B2SServer final : public ScriptablePlugin::IScriptProxy
{
public:
   B2SServer(const MsgPluginAPI* const msgApi, unsigned int endpointId, const VPXPluginAPI* const vpxApi, ScriptClassDef* serverClassDef);
   ~B2SServer();

   PSC_IMPLEMENT_REFCOUNT()

   void Dispose() { }
   string GetB2SServerVersion() const { return ""s; }
   double GetB2SBuildVersion() const { return 0.0; }
   string GetB2SServerDirectory() const { return ""s; }
   string GetVPMBuildVersion() const { return ""s; }
   string GetB2SName() const;
   void SetB2SName(const std::string& b2sName);
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
   
   void B2SSetLED(int, int) { } // FIXME
   void B2SSetLED(int, string) { } // FIXME
   void B2SSetLEDDisplay(int, string) { } // FIXME

   // Scores identified by player, multiple digits (generate 'C' plugin events)
   void B2SSetScorePlayer(int playerno, int score);
   void B2SSetScorePlayer1(int score)                        { B2SSetScorePlayer(1, score); }
   void B2SSetScorePlayer2(int score)                        { B2SSetScorePlayer(2, score); }
   void B2SSetScorePlayer3(int score)                        { B2SSetScorePlayer(3, score); }
   void B2SSetScorePlayer4(int score)                        { B2SSetScorePlayer(4, score); }
   void B2SSetScorePlayer5(int score)                        { B2SSetScorePlayer(5, score); }
   void B2SSetScorePlayer6(int score)                        { B2SSetScorePlayer(6, score); }

   // Scores identified by digit, either single or multiple digits (generate 'B' plugin events)
   void B2SSetScore(int digit, int value, bool animate = true);
   void B2SSetScoreDigit(int digit, int value)               { B2SSetScore(digit, value, false); }
   void B2SSetReel(int digit, int value)                     { B2SSetScore(digit, value, true); }
   void B2SSetCredits(int value)                             { m_defaultStateNameMask |= 1ull << 29; B2SSetScore(29, value, true); }
   void B2SSetCredits(int id, int value)                     { B2SSetScore(id, value, true); }
   
   // Illumination and animation states (generate 'E' plugin events)
   // Used to be binary on/off as 1/0 but not validated so script could use any integer value
   // Upgraded to be a float as this can also be driven by emulators with faded lamps & flashers
   void B2SSetData(int id, int value, bool sendPluginEvent = true);
   void B2SSetData(const std::string& group, int value);
   void B2SPulseData(int id)                                 { B2SSetData(id, 1); B2SSetData(id, 0); }
   void B2SPulseData(const std::string& group)               { B2SSetData(group, 1); B2SSetData(group, 0); }
   void B2SSetScoreRollover(int id, int value)               { B2SSetData(id, value); }
   void B2SSetScoreRolloverPlayer1(int value)                { m_defaultStateNameMask |= 1ull << 25; B2SSetData(25, value); }
   void B2SSetScoreRolloverPlayer2(int value)                { m_defaultStateNameMask |= 1ull << 26; B2SSetData(26, value); }
   void B2SSetScoreRolloverPlayer3(int value)                { m_defaultStateNameMask |= 1ull << 27; B2SSetData(27, value); }
   void B2SSetScoreRolloverPlayer4(int value)                { m_defaultStateNameMask |= 1ull << 28; B2SSetData(28, value); }
   void B2SSetPlayerUp(int value)                            { m_defaultStateNameMask |= 1ull << 30; B2SSetData(30, value); }
   void B2SSetPlayerUp(int id, int value)                    { B2SSetData(id, value); }
   void B2SSetCanPlay(int value)                             { m_defaultStateNameMask |= 1ull << 31; B2SSetData(31, value); }
   void B2SSetCanPlay(int id, int value)                     { B2SSetData(id, value); }
   void B2SSetBallInPlay(int value)                          { m_defaultStateNameMask |= 1ull << 32; B2SSetData(32, value); }
   void B2SSetBallInPlay(int id, int value)                  { B2SSetData(id, value); }
   void B2SSetTilt(int value)                                { m_defaultStateNameMask |= 1ull << 33; B2SSetData(33, value); }
   void B2SSetTilt(int id, int value)                        { B2SSetData(id, value); }
   void B2SSetMatch(int value)                               { m_defaultStateNameMask |= 1ull << 34; B2SSetData(34, value); }
   void B2SSetMatch(int id, int value)                       { B2SSetData(id, value); }
   void B2SSetGameOver(int value)                            { m_defaultStateNameMask |= 1ull << 35; B2SSetData(35, value); }
   void B2SSetGameOver(int id, int value)                    { B2SSetData(id, value); }
   void B2SSetShootAgain(int value)                          { m_defaultStateNameMask |= 1ull << 36; B2SSetData(36, value); }
   void B2SSetShootAgain(int id, int value)                  { B2SSetData(id, value); }
   void B2SSetIllumination(const std::string& id, int value) { B2SSetData(id, value); }

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

   void SetOnDestroyHandler(std::function<void(B2SServer*)> handler) { m_onDestroyHandler = handler; }
   float GetLampState(int b2sId) const;
   int GetScoreDigit(int digit) const;
   int GetPlayerScore(int player) const;

   void ForwardCall(void* me, int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet) override;

private:
   ScriptablePlugin::ScriptClassProxy m_controllerClassProxy;
   ScriptablePlugin::ScriptObjectProxy m_controllerProxy;

   const MsgPluginAPI* const m_msgApi;
   const unsigned int m_endpointId;
   const VPXPluginAPI* const m_vpxApi;

   std::future<std::shared_ptr<B2STable>> m_loadedB2S;
   std::function<void(B2SServer*)> m_onDestroyHandler;

   static B2SServer* m_singleton;

   // Renderer
   std::unique_ptr<B2SRenderer> m_renderer = nullptr;
   const unsigned int m_onGetAuxRendererId;
   const unsigned int m_onAuxRendererChgId;
   const AncillaryRendererDef m_ancillaryRendererDef;
   static int OnRender(VPXRenderContext2D* ctx, void*);
   static void OnGetRenderer(const unsigned int, void*, void* msgData);

   // Controller state
   string m_b2sName;
   string m_controllerGameId;
   bool m_gameRunning = false;
   const unsigned int m_onControllersChangedId;
   const unsigned int m_getControllersId;
   static void OnGetControllers(const unsigned int, void*, void* msgData);
   uint64_t m_defaultStateNameMask = 0;
   ankerl::unordered_dense::map<int, float> m_lampStates;
   ankerl::unordered_dense::map<int, int> m_playerScores;
   ankerl::unordered_dense::map<int, int> m_scoreDigits;
   const unsigned int m_onGetStateSrcId;
   const unsigned int m_onStateSrcChgId;
   const unsigned int m_onStateChangeEventId;
   StateSrcId m_stateSrc { };
   vector<StateGroupDef> m_stateGroupDefs = { { "Illuminations", "", 0x0001 }, { "Scores (players)", "", 0x0002 }, { "Scores (digits)", "", 0x0003 } };
   vector<string> m_stateSrcNames;
   void UpdateStateSrc();
   static void OnGetStateSrc(const unsigned int, void*, void* msgData);
   static int MSGPIAPI GetState(unsigned int inputIndex, int type, void* pResult);
   static int MSGPIAPI SetState(unsigned int inputIndex, int type, void* pResult);
};

}
