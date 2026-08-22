// license:GPLv3+

#pragma once

#include "common.h"
#include <functional>
#include <mutex>
#include <unordered_dense.h>
#include "forms/FormBackglass.h"
#include "classes/B2SCollectData.h"
#include "plugins/ControllerPlugin.h"
#include "utils/PinMAMEAPI.h"

namespace B2SLegacy {

class PinMAMEAPI;

class Server : public PinballPlugin::Scriptable::IScriptProxy
{
public:
   Server(MsgPluginAPI* msgApi, uint32_t endpointId, VPXPluginAPI* vpxApi, ScriptClassDef* pinmameClassDef);
   ~Server();

   PSC_IMPLEMENT_REFCOUNT()

   void Dispose();
   static const string& GetB2SServerVersion();
   static double GetB2SBuildVersion();
   const string& GetB2SServerDirectory() const;
   const string& GetB2SName() const;
   void SetB2SName(const string& b2sName);
   const string& GetTableName() const;
   void SetTableName(const string& tableName);
   void SetWorkingDir(const string& workingDir);
   void SetPath(const string& path);
   void Run(int handle);
   void Stop();
   bool GetLaunchBackglass() const;
   void SetLaunchBackglass(bool launchBackglass);
   bool GetPuPHide() const;
   void SetPuPHide(bool puPHide);
   void B2SSetData(int id, int value);
   void B2SSetData(const string& name, int value);
   void B2SSetData(int id, const string& value);
   void B2SSetData(const string& name, const string& value);
   void B2SPulseData(int id);
   void B2SPulseData(const string& name);
   void B2SSetPos(int id, int xpos, int ypos);
   void B2SSetPos(int id, int xpos, const string& ypos);
   void B2SSetPos(int id, const string& xpos, int ypos);
   void B2SSetPos(int id, const string& xpos, const string& ypos);
   void B2SSetPos(const string& id, int xpos, int ypos);
   void B2SSetPos(const string& id, int xpos, const string& ypos);
   void B2SSetPos(const string& id, const string& xpos, int ypos);
   void B2SSetPos(const string& id, const string& xpos, const string& ypos);
   void B2SSetIllumination(const string& name, int value);
   void B2SSetLED(int digit, int value);
   void B2SSetLED(int digit, const string& text);
   void B2SSetLEDDisplay(int display, const string& text);
   void B2SSetReel(int digit, int value);
   void B2SSetScore(int display, int value);
   void B2SSetScorePlayer(int playerno, int score);
   void B2SSetScorePlayer1(int score);
   void B2SSetScorePlayer2(int score);
   void B2SSetScorePlayer3(int score);
   void B2SSetScorePlayer4(int score);
   void B2SSetScorePlayer5(int score);
   void B2SSetScorePlayer6(int score);
   void B2SSetScoreDigit(int digit, int value);
   void B2SSetScoreRollover(int id, int value);
   void B2SSetScoreRolloverPlayer1(int value);
   void B2SSetScoreRolloverPlayer2(int value);
   void B2SSetScoreRolloverPlayer3(int value);
   void B2SSetScoreRolloverPlayer4(int value);
   void B2SSetCredits(int value);
   void B2SSetCredits(int digit, int value);
   void B2SSetPlayerUp(int value);
   void B2SSetPlayerUp(int id, int value);
   void B2SSetCanPlay(int value);
   void B2SSetCanPlay(int id, int value);
   void B2SSetBallInPlay(int value);
   void B2SSetBallInPlay(int id, int value);
   void B2SSetTilt(int value);
   void B2SSetTilt(int id, int value);
   void B2SSetMatch(int value);
   void B2SSetMatch(int id, int value);
   void B2SSetGameOver(int value);
   void B2SSetGameOver(int id, int value);
   void B2SSetShootAgain(int value);
   void B2SSetShootAgain(int id, int value);
   void B2SStartAnimation(const string& animationname);
   void B2SStartAnimation(const string& animationname, bool playreverse);
   void B2SStartAnimationReverse(const string& animationname);
   void B2SStopAnimation(const string& animationname);
   void B2SStopAllAnimations();
   bool B2SIsAnimationRunning(const string& animationname) const;
   void StartAnimation(const string& animationname);
   void StartAnimation(const string& animationname, bool playreverse);
   void StopAnimation(const string& animationname);
   void B2SStartRotation();
   void B2SStopRotation();
   void B2SShowScoreDisplays();
   void B2SHideScoreDisplays();
   void B2SStartSound(const string& soundname);
   void B2SPlaySound(const string& soundname);
   void B2SStopSound(const string& soundname);
   void B2SMapSound(int digit, const string& soundname);
   string GetVPMBuildVersion() const { return ""s; }
   bool GetLockDisplay() const { return false; }
   void SetLockDisplay(bool lockDisplay) { }
   FormBackglass* GetFormBackglass() const { return m_pFormBackglass; }
   B2SSettings* GetB2SSettings() const { return m_pB2SSettings; }
   uint32_t GetEndpointId() const { return m_endpointId; }
   void SetOnDestroyHandler(std::function<void(Server*)> handler) { m_onDestroyHandler = handler; }
   float GetState(int b2sId) const;
   int GetPlayerScore(int playerno) const;
   int GetScoreDigit(int digit) const;
   void GetChangedLamps();
   void GetChangedLamps(ScriptVariant* pRet);
   void GetChangedSolenoids();
   void GetChangedSolenoids(ScriptVariant* pRet);
   void GetChangedGIStrings();
   void GetChangedGIStrings(ScriptVariant* pRet);
   void GetChangedLEDs(ScriptVariant* pRet);
   void GetChangedLEDs();
   void SetSwitch(int switchId, bool value);
   void CheckGetMech(int number, int mech);
   int OnRender(VPXRenderContext2D* const renderCtx, void* context);

   void ForwardCall(void* me, int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet) override { m_pinmameApi.HandleCall(memberIndex, pArgs, pRet); }

private:
   void TimerElapsed(Timer* pTimer);
   void CheckLamps(ScriptArray* psa);
   void CheckSolenoids(ScriptArray* psa);
   void CheckGIStrings(ScriptArray* psa);
   void CheckLEDs(ScriptArray* psa);
   void MyB2SSetData(int id, int value);
   void MyB2SSetData(const string& groupname, int value);
   void MyB2SSetPos(int id, int xpos, int ypos);
   void MyB2SSetLED(int digit, int value);
   void MyB2SSetLED(int digit, const string& value);
   void MyB2SSetLEDDisplay(int display, const string& szText);
   int GetFirstDigitOfDisplay(int display) const;
   void MyB2SSetScore(int digit, int value, bool animateReelChange);
   void MyB2SSetScore(int digit, int score);
   void MyB2SSetScorePlayer(int playerno, int score);
   void MyB2SStartAnimation(const string& animationname, bool playreverse);
   void MyB2SStopAnimation(const string& animationname);
   void MyB2SStopAllAnimations();
   bool MyB2SIsAnimationRunning(const string& animationname) const;
   void MyB2SStartRotation();
   void MyB2SStopRotation();
   void MyB2SShowScoreDisplays();
   void MyB2SHideScoreDisplays();
   void MyB2SShowOrHideScoreDisplays(bool visible);
   void MyB2SPlaySound(const string& soundname);
   void MyB2SStopSound(const string& soundname);
   void Startup();
   void ShowBackglassForm();
   void HideBackglassForm();
   void KillBackglassForm();
   static int RandomStarter(int top);

   B2SSettings* m_pB2SSettings = nullptr;
   B2SData* m_pB2SData = nullptr;
   FormBackglass* m_pFormBackglass = nullptr;
   bool m_isVisibleStateSet = false;
   bool m_lastTopVisible = false;
   bool m_lastSecondVisible = false;
   int m_lampThreshold = 0;
   int m_giStringThreshold = 4;
   bool m_changedLampsCalled = false;
   bool m_changedSolenoidsCalled = false;
   bool m_changedGIStringsCalled = false;
   bool m_changedMechsCalled = false;
   bool m_changedLEDsCalled = false;
   string m_lastRandomStartedAnimation;
   B2SCollectData* m_pCollectLampsData = nullptr;
   B2SCollectData* m_pCollectSolenoidsData = nullptr;
   B2SCollectData* m_pCollectGIStringsData = nullptr;
   B2SCollectData* m_pCollectLEDsData = nullptr;
   string m_szPath = "./";
   Timer* m_pTimer = nullptr;

   static Server* m_singleton;

   PinballPlugin::Controller::CtrlItemConsumer<ControllerDef> m_pinmameControllers;
   mutable PinballPlugin::Controller::CtrlItemConsumer<StateSrcId> m_stateSources;

   string m_controllerGameId;
   bool m_gameRunning = false;
   ankerl::unordered_dense::map<int, float> m_b2sStates;
   ankerl::unordered_dense::map<int, int> m_playerScores;
   ankerl::unordered_dense::map<int, int> m_scoreDigits;
   const unsigned int m_onStateChangeEventId;
   PinballPlugin::Controller::CtrlItemProvider<ControllerDef> m_exposedControllers;
   PinballPlugin::Controller::CtrlItemProvider<StateSrcId> m_exposedStates;
   void UpdateStateSrc();
   mutable std::mutex m_stateMutex;
   vector<StateDef> m_lampStateDefs;
   vector<string> m_lampStateNames;
   vector<int> m_lampStateIds;
   vector<StateDef> m_playerScoreStateDefs;
   vector<string> m_playerScoreNames;
   vector<int> m_playerScoreIds;
   vector<StateDef> m_scoreDigitStateDefs;
   vector<string> m_scoreDigitNames;
   vector<int> m_scoreDigitIds;
   static void MSGPIAPI GetLampState(CtlResId id, unsigned int inputIndex, void* pResult);
   static void MSGPIAPI GetPlayerScore(CtlResId id, unsigned int inputIndex, void* pResult);
   static void MSGPIAPI GetScoreDigit(CtlResId id, unsigned int inputIndex, void* pResult);

   MsgPluginAPI* const m_msgApi;
   VPXPluginAPI* const m_vpxApi;
   const uint32_t m_endpointId;

   const unsigned int m_onGetAuxRendererId;
   const unsigned int m_onAuxRendererChgId;
   const unsigned int m_onStateChangedMsgId;

   PinMAMEAPI m_pinmameApi;

   std::function<void(Server*)> m_onDestroyHandler;

   static int OnRenderStatic(VPXRenderContext2D* ctx, void* userData);
   static void OnGetRendererStatic(const unsigned int msgId, void* userData, void* msgData);

   bool m_ready = false;
};

}
