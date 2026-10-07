// license:GPLv3+

#pragma once

#include "common.h"

#include <cstdint>
#include <climits>
#include <future>
#include <mutex>
#include <cstdint>

#include <unordered_dense.h>

#include "B2SDataModel.h"
#include "B2SRenderer.h"

namespace B2S {

class B2SServer final : public PinballPlugin::Scriptable::IScriptProxy
{
public:
   B2SServer(const MsgPluginAPI* const msgApi, unsigned int endpointId, const VPXPluginAPI* const vpxApi, ScriptClassDef* serverClassDef);
   ~B2SServer();

   PSC_IMPLEMENT_REFCOUNT()

   void Dispose() { }
   string GetB2SServerVersion() const;
   double GetB2SBuildVersion() const;
   string GetB2SServerDirectory() const;
   string GetVPMBuildVersion() const { return ""s; }
   string GetB2SName() const;
   void SetB2SName(const string& b2sName);
   string GetTableName() const { return m_tableName; }
   void SetTableName(const string& tableName);
   void SetWorkingDir(const string& workingDir);
   void SetPath(const string& path) { SetWorkingDir(path); }
   bool GetLaunchBackglass() const { return m_launchBackglass; }
   void SetLaunchBackglass(bool launchBackglass) { m_launchBackglass = launchBackglass; }
   bool GetLockDisplay() const { return false; }
   void SetLockDisplay(bool lockDisplay) { }
   bool GetPuPHide() const { return false; }
   void SetPuPHide(bool puPHide) { }

   void B2SSetPos(int id, int x, int y);
   void B2SSetPos(int id, const string& x, int y)
   {
      if (is_string_numeric(x, nullptr))
         B2SSetPos(id, string_to_int(x, 0), y);
   }
   void B2SSetPos(int id, int x, const string& y)
   {
      if (is_string_numeric(y, nullptr))
         B2SSetPos(id, x, string_to_int(y, 0));
   }
   void B2SSetPos(int id, const string& x, const string& y)
   {
      if (is_string_numeric(x, nullptr) && is_string_numeric(y, nullptr))
         B2SSetPos(id, string_to_int(x, 0), string_to_int(y, 0));
   }
   void B2SSetPos(const string& name, int x, int y)
   {
      if (is_string_numeric(name, nullptr))
         B2SSetPos(string_to_int(name, 0), x, y);
   }
   void B2SSetPos(const string& name, const string& x, int y)
   {
      if (is_string_numeric(name, nullptr) && is_string_numeric(x, nullptr))
         B2SSetPos(string_to_int(name, 0), string_to_int(x, 0), y);
   }
   void B2SSetPos(const string& name, int x, const string& y)
   {
      if (is_string_numeric(name, nullptr) && is_string_numeric(y, nullptr))
         B2SSetPos(string_to_int(name, 0), x, string_to_int(y, 0));
   }
   void B2SSetPos(const string& name, const string& x, const string& y)
   {
      if (is_string_numeric(name, nullptr) && is_string_numeric(x, nullptr) && is_string_numeric(y, nullptr))
         B2SSetPos(string_to_int(name, 0), string_to_int(x, 0), string_to_int(y, 0));
   }

   void B2SSetLED(int digit, int value);
   void B2SSetLED(int digit, const string& value);
   void B2SSetLEDDisplay(int display, const string& text);

   // Scores identified by player, multiple digits (generate 'C' plugin events)
   void B2SSetScorePlayer(int playerno, int score);
   void B2SSetScorePlayer1(int score)                        { B2SSetScorePlayer(1, score); }
   void B2SSetScorePlayer2(int score)                        { B2SSetScorePlayer(2, score); }
   void B2SSetScorePlayer3(int score)                        { B2SSetScorePlayer(3, score); }
   void B2SSetScorePlayer4(int score)                        { B2SSetScorePlayer(4, score); }
   void B2SSetScorePlayer5(int score)                        { B2SSetScorePlayer(5, score); }
   void B2SSetScorePlayer6(int score)                        { B2SSetScorePlayer(6, score); }

   // Scores identified by digit, either single or multiple digits (generate 'B' plugin events)
   void B2SSetScore(int display, int value);
   void B2SSetScoreDigit(int digit, int value);
   void B2SSetReel(int digit, int value);
   void B2SSetCredits(int value)
   {
      m_defaultStateNameMask |= 1ull << 29;
      ApplyScoreDigit(29, value, false);
   }
   void B2SSetCredits(int id, int value) { ApplyScoreDigit(id, value, false); }

   // Illumination and animation states (generate 'E' plugin events)
   // Used to be binary on/off as 1/0 but not validated so script could use any integer value
   // Upgraded to be a float as this can also be driven by emulators with faded lamps & flashers
   void B2SSetData(int id, int value, bool sendPluginEvent = true);
   void B2SSetData(int id, const string& value, bool sendPluginEvent = true);
   void B2SSetData(const string& group, int value);
   void B2SSetData(const string& group, const string& value);
   void B2SPulseData(int id)                                 { B2SSetData(id, 1); B2SSetData(id, 0); }
   void B2SPulseData(const string& group)               { B2SSetData(group, 1); B2SSetData(group, 0); }
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
   void B2SStopAllAnimations();
   bool GetB2SIsAnimationRunning(const string& animationName) const;
   void StartAnimation(const string& animationName, bool reverse = false);
   void StopAnimation(const string& animationName);
   void B2SStartRotation();
   void B2SStopRotation();
   void B2SShowScoreDisplays() { m_scoreDisplaysHidden = false; }
   void B2SHideScoreDisplays() { m_scoreDisplaysHidden = true; }
   void B2SStartSound(const string& soundName);
   void B2SPlaySound(const string& soundName);
   void B2SStopSound(const string& soundName);
   void B2SMapSound(int digit, const string& soundName) { } // Not implemented in the reference either

   void SetOnDestroyHandler(std::function<void(B2SServer*)> handler) { m_onDestroyHandler = handler; }
   float GetLampState(int b2sId) const;
   bool GetScriptedLampState(int b2sId, float& state) const; // True when the script wrote this lamp id through B2SSetData
   bool GetBulbState(const B2SBulb& bulb, float& state) const;
   int GetScoreDigit(int digit) const;
   int GetScoreDigitSegments(int digit) const; // Explicit B2SSetLED segment mask, -1 = none
   bool ConsumeScoreDigitRoll(int digit); // Returns and clears the rolling flag set by B2SSetScore/B2SSetReel
   int GetPlayerScore(int player) const;

   // Animation engine glue: the renderer drives the animations and applies their effects through this interface
   B2SAnimationEffects GetAnimationEffects();
   void PulseSwitch(int switchId);
   bool IsIlluminationLocked(const string& group) const;
   bool AreScoreDisplaysHidden() const { return m_scoreDisplaysHidden; }

   // Plugin settings
   static void RegisterSettings(const MsgPluginAPI* msgApi, unsigned int endpointId);

   // Pending lamp-id writes from B2SSetData that drive animation triggers (drained by the render thread)
   std::vector<std::pair<int, int>> DrainAnimationTriggers();

   int GetAnimationSlowDown(const string& name) const;
   int GetAllAnimationSlowDown() const;
   int GetUsedLEDType() const;
   int GetHideB2SDMD() const;
   int GetHideDMD() const; // -1 = no override, 1 = force hidden

   void ForwardCall(void* me, int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet) override;

private:
   PinballPlugin::Scriptable::ScriptClassProxy m_controllerClassProxy;
   PinballPlugin::Scriptable::ScriptObjectProxy m_controllerProxy;

   const MsgPluginAPI* const m_msgApi;
   const unsigned int m_endpointId;
   const VPXPluginAPI* const m_vpxApi;
   const std::thread::id m_msgApiThreadId { std::this_thread::get_id() };

   std::future<std::shared_ptr<B2STable>> m_loadedB2S;
   mutable std::mutex m_b2sMutex;
   std::shared_ptr<B2STable> m_b2s; // Acquired once the asynchronous load completes
   std::shared_ptr<B2STable> AcquireB2STable();

   // Backglass file discovery (table file name, or folder name for merged layouts)
   static std::shared_ptr<B2STable> LoadB2SFile(const std::filesystem::path& path);
   std::filesystem::path FindB2SFile() const;
   void TryLoadB2S();
   std::filesystem::path m_tableDir;
   string m_tableName; // Table file stem by default, scripts may override through TableName
   std::filesystem::path m_workingDir; // Script override through WorkingDir/SetPath
   std::atomic<bool> m_launchBackglass { true };
   void ApplyScoreDigit(int digit, int value, bool roll);
   void ApplyScoreSegments(int digit, int segMask);
   std::function<void(B2SServer*)> m_onDestroyHandler;

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
   uint64_t m_defaultStateNameMask = 0;
   struct LampState
   {
      std::atomic<float> value { 0.f };
      std::atomic<uint64_t> stamp { 0 };
   };
   std::map<int, LampState> m_lampStates;
   std::map<string, LampState> m_groupStates;
   std::atomic<uint64_t> m_lampStamp { 0 }; // Monotonic write counter used to resolve most recent state writes
   std::atomic<bool> m_scoreDisplaysHidden { false };
   std::map<string, int> m_illuminationLocks; // Ref-counted animation locks, keyed by illumination group (bulb name)
   mutable std::mutex m_illuminationLockMutex;
   std::mutex m_animationTriggerMutex;
   std::vector<std::pair<int, int>> m_pendingAnimationTriggers;
   int m_setSwitchIndex = -1; // Index of the "Switch" setter member in the proxied PinMAME controller class
   std::map<int, std::atomic<int>> m_playerScores;
   struct ScoreDigit
   {
      std::atomic<int> value { 0 };
      std::atomic<bool> roll { false };
      std::atomic<int> segMask { -1 }; // >= 0: explicit CTLPI-ordered segment mask set by B2SSetLED(B2SSetLEDDisplay)
   };
   std::map<int, ScoreDigit> m_scoreDigits;
   const unsigned int m_onStateChangeEventId;
   PinballPlugin::Controller::CtrlItemProvider<ControllerDef> m_exposedControllers;
   PinballPlugin::Controller::CtrlItemProvider<StateSrcId> m_exposedStates;
   void UpdateStateSrc();

   // Embedded sound playback: one audio source, one stream per sound name (MsgAPI thread only)
   PinballPlugin::Controller::CtrlItemProvider<AudioSrcId> m_audioSrc;
   const unsigned int m_onAudioUpdateId;
   uint32_t m_nextSoundStreamId = 1;
   std::map<string, uint32_t> m_soundStreams; // Sound name -> stream resId
   void StartSoundStream(const string& soundName);
   void StopSoundStream(uint32_t streamResId);
   mutable std::mutex m_stateMutex;
   struct CallContext
   {
      B2SServer* me;
      int id;
   };
   vector<StateDef> m_lampStateDefs;
   vector<string> m_lampStateNames;
   vector<CallContext> m_lampStateIds;
   vector<StateDef> m_playerScoreStateDefs;
   vector<string> m_playerScoreNames;
   vector<CallContext> m_playerScoreIds;
   vector<StateDef> m_scoreDigitStateDefs;
   vector<string> m_scoreDigitNames;
   vector<CallContext> m_scoreDigitIds;
   static void MSGPIAPI GetLampState(void* callContext, void* pResult);
   static void MSGPIAPI GetPlayerScore(void* callContext, void* pResult);
   static void MSGPIAPI GetScoreDigit(void* callContext, void* pResult);
};

}
