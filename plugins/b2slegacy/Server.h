// license:GPLv3+

#pragma once

#include "common.h"
#include <functional>
#include "forms/FormBackglass.h"
#include "classes/B2SCollectData.h"
#include "plugins/ControllerPlugin.h"

namespace B2SLegacy {

class PinMAMEAPI;

class Server
{
public:
   Server(MsgPluginAPI* msgApi, uint32_t endpointId, VPXPluginAPI* vpxApi, ScriptClassDef* pinmameClassDef, int pinmameMemberStartIndex);
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
   void B2SPulseData(int id);
   void B2SPulseData(const string& name);
   void B2SSetPos(int id, int xpos, int ypos);
   void B2SSetPos(const string& name, int xpos, int ypos);
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
   PinMAMEAPI* GetPinMAMEApi() const { return m_pinmameApi; }
   uint32_t GetEndpointId() const { return m_endpointId; }
   void ForwardPinMAMECall(int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet);
   void SetOnDestroyHandler(std::function<void(Server*)> handler) { m_onDestroyHandler = handler; }
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
   void OnDevSrcChanged(const unsigned int msgId, void* userData, void* msgData);

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

   B2SSettings* m_pB2SSettings;
   B2SData* m_pB2SData;
   FormBackglass* m_pFormBackglass;
   bool m_isVisibleStateSet;
   bool m_lastTopVisible;
   bool m_lastSecondVisible;
   int m_lampThreshold;
   int m_giStringThreshold;
   bool m_changedLampsCalled;
   bool m_changedSolenoidsCalled;
   bool m_changedGIStringsCalled;
   bool m_changedMechsCalled;
   bool m_changedLEDsCalled;
   string m_lastRandomStartedAnimation;
   B2SCollectData* m_pCollectLampsData;
   B2SCollectData* m_pCollectSolenoidsData;
   B2SCollectData* m_pCollectGIStringsData;
   B2SCollectData* m_pCollectLEDsData;
   string m_szPath;
   Timer* m_pTimer;

   DevSrcId m_deviceStateSrc;
   unsigned int m_nSolenoids;
   int m_GIIndex;
   unsigned int m_nGIs;
   int m_lampIndex;
   unsigned int m_nLamps;
   int m_mechIndex;
   unsigned int m_nMechs;

   MsgPluginAPI* const m_msgApi;
   VPXPluginAPI* const m_vpxApi;
   const uint32_t m_endpointId;

   const unsigned int m_onGetAuxRendererId;
   const unsigned int m_onAuxRendererChgId;
   const unsigned int m_onDevChangedMsgId;

   ScriptClassDef* const m_pinmameClassDef;
   const int m_pinmameMemberStartIndex;
   PinMAMEAPI* m_pinmameApi;

   std::function<void(Server*)> m_onDestroyHandler;

   static int OnRenderStatic(VPXRenderContext2D* ctx, void* userData);
   static void OnGetRendererStatic(const unsigned int, void*, void* msgData);
   static void OnDevSrcChangedStatic(const unsigned int msgId, void* userData, void* msgData);

   bool m_ready = false;
};

}
