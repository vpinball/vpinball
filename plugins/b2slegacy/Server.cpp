// license:GPLv3+

#include "common.h"
#include "Server.h"
#include "plugins/LoggingPlugin.h"

using namespace std::string_literals;

#include "forms/FormBackglass.h"
#include "forms/FormDMD.h"

#include "classes/B2SVersionInfo.h"
#include "controls/B2SPictureBox.h"
#include "controls/B2SLEDBox.h"
#include "controls/B2SReelBox.h"
#include "classes/LEDDisplayDigitLocation.h"
#include "classes/CollectData.h"
#include "dream7/Dream7Display.h"
#include "classes/AnimationInfo.h"
#include "classes/PictureBoxAnimation.h"
#include "classes/B2SScreen.h"
#include "utils/PinMAMEAPI.h"


namespace B2SLegacy {

Server::Server(MsgPluginAPI* msgApi, uint32_t endpointId, VPXPluginAPI* vpxApi, ScriptClassDef* pinmameClassDef, int pinmameMemberStartIndex)
   : m_msgApi(msgApi)
   , m_vpxApi(vpxApi)
   , m_endpointId(endpointId)
   , m_onGetAuxRendererId(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER))
   , m_onAuxRendererChgId(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG))
   , m_onDevChangedMsgId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_ON_SRC_CHG_MSG))
   , m_pinmameClassDef(pinmameClassDef)
   , m_pinmameMemberStartIndex(pinmameMemberStartIndex)
   , m_pinmameApi(pinmameClassDef ? new PinMAMEAPI(this, pinmameClassDef) : nullptr)
{
   m_pB2SSettings = new B2SSettings(m_msgApi, endpointId);
   m_pB2SData = new B2SData(this, m_pB2SSettings, m_vpxApi);
   m_pFormBackglass = nullptr;
   m_isVisibleStateSet = false;
   m_lastTopVisible = false;
   m_lastSecondVisible = false;
   m_lampThreshold = 0;
   m_giStringThreshold = 4;
   m_changedLampsCalled = false;
   m_changedSolenoidsCalled = false;
   m_changedGIStringsCalled = false;
   m_changedMechsCalled = false;
   m_changedLEDsCalled = false;
   m_pCollectLampsData = new B2SCollectData(m_pB2SSettings->GetLampsSkipFrames());
   m_pCollectSolenoidsData = new B2SCollectData(m_pB2SSettings->GetSolenoidsSkipFrames());
   m_pCollectGIStringsData = new B2SCollectData(m_pB2SSettings->GetGIStringsSkipFrames());
   m_pCollectLEDsData = new B2SCollectData(m_pB2SSettings->GetLEDsSkipFrames());
   m_pTimer = new Timer();
   m_pTimer->SetInterval(37);
   m_pTimer->SetElapsedListener(std::bind(&Server::TimerElapsed, this, std::placeholders::_1));
   m_szPath = "./";

   memset(&m_deviceStateSrc, 0, sizeof(m_deviceStateSrc));
   m_nSolenoids = 0;
   m_GIIndex = -1;
   m_nGIs = 0;
   m_lampIndex = -1;
   m_nLamps = 0;
   m_mechIndex = -1;
   m_nMechs = 0;

   msgApi->SubscribeMsg(endpointId, m_onGetAuxRendererId, OnGetRendererStatic, this);
   msgApi->SubscribeMsg(endpointId, m_onDevChangedMsgId, OnDevSrcChangedStatic, this);
   msgApi->BroadcastMsg(endpointId, m_onAuxRendererChgId, nullptr);
}

Server::~Server()
{
   m_msgApi->UnsubscribeMsg(m_onGetAuxRendererId, OnGetRendererStatic);
   m_msgApi->UnsubscribeMsg(m_onDevChangedMsgId, OnDevSrcChangedStatic);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);
   m_msgApi->ReleaseMsgID(m_onGetAuxRendererId);
   m_msgApi->ReleaseMsgID(m_onAuxRendererChgId);
   m_msgApi->ReleaseMsgID(m_onDevChangedMsgId);

   delete m_pinmameApi;
   m_pinmameApi = nullptr;

   delete[] m_deviceStateSrc.deviceDefs;

   if (m_onDestroyHandler)
      m_onDestroyHandler(this);

   delete m_pTimer;
   delete m_pFormBackglass;
   delete m_pB2SData;
   delete m_pCollectLampsData;
   delete m_pCollectSolenoidsData;
   delete m_pCollectGIStringsData;
   delete m_pCollectLEDsData;
   delete m_pB2SSettings;
}

int Server::OnRender(VPXRenderContext2D* const renderCtx, void* context)
{
   if (!m_pFormBackglass)
      return 0;

   if (!m_ready) {
      if (!m_pFormBackglass->IsValid())
         return 0;

      m_pFormBackglass->Start();
      m_ready = true;
   }

   if (renderCtx->window == VPXWindowId::VPXWINDOW_Backglass) {
      if (!m_pB2SSettings->IsHideB2SBackglass()) {
         SDL_Rect& size = m_pFormBackglass->GetB2SScreen()->GetBackglassSize();
         renderCtx->srcWidth = static_cast<float>(size.w);
         renderCtx->srcHeight = static_cast<float>(size.h);
         m_pFormBackglass->OnPaint(renderCtx);
         return 1;
      }
   }
   else if (renderCtx->window == VPXWindowId::VPXWINDOW_ScoreView) {
      if (m_pFormBackglass->GetFormDMD()) {
         SDL_Rect& size = m_pFormBackglass->GetB2SScreen()->GetDMDSize();
         renderCtx->srcWidth = static_cast<float>(size.w);
         renderCtx->srcHeight = static_cast<float>(size.h);
         m_pFormBackglass->GetFormDMD()->OnPaint(renderCtx);
         return 1;
      }
   }

   return 0;
}

void Server::OnDevSrcChanged(const unsigned int msgId, void* userData, void* msgData)
{
   delete[] m_deviceStateSrc.deviceDefs;
   memset(&m_deviceStateSrc, 0, sizeof(m_deviceStateSrc));
   m_nSolenoids = 0;
   m_GIIndex = -1;
   m_nGIs = 0;
   m_lampIndex = -1;
   m_nLamps = 0;
   m_mechIndex = -1;
   m_nMechs = 0;

   unsigned int pinmameEndpoint = m_msgApi->GetPluginEndpoint("PinMAME");
   if (pinmameEndpoint == 0)
      return;

   unsigned int getDevSrcMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_GET_SRC_MSG);
   GetDevSrcMsg getSrcMsg = { 0, 0, nullptr };
   m_msgApi->SendMsg(m_endpointId, getDevSrcMsgId, pinmameEndpoint, &getSrcMsg);
   vector<DevSrcId> entries(getSrcMsg.count);
   getSrcMsg = { getSrcMsg.count, 0, entries.data() };
   m_msgApi->SendMsg(m_endpointId, getDevSrcMsgId, pinmameEndpoint, &getSrcMsg);
   m_msgApi->ReleaseMsgID(getDevSrcMsgId);
   for (unsigned int i = 0; i < getSrcMsg.count; i++)
   {
      if (getSrcMsg.entries[i].id.endpointId == pinmameEndpoint)
      {
         m_deviceStateSrc = getSrcMsg.entries[i];
         if (getSrcMsg.entries[i].deviceDefs)
         {
            m_deviceStateSrc.deviceDefs = new DeviceDef[getSrcMsg.entries[i].nDevices];
            memcpy(m_deviceStateSrc.deviceDefs, getSrcMsg.entries[i].deviceDefs, getSrcMsg.entries[i].nDevices * sizeof(DeviceDef));
         }
         break;
      }
   }

   if (m_deviceStateSrc.deviceDefs == nullptr)
      return;

   for (unsigned int i = 0; i < m_deviceStateSrc.nDevices; i++)
   {
      if (m_deviceStateSrc.deviceDefs[i].groupId == 0x0100)
      {
         if (m_GIIndex == -1)
            m_GIIndex = i;
         m_nGIs++;
      }
      else if (m_deviceStateSrc.deviceDefs[i].groupId == 0x0200)
      {
         if (m_lampIndex == -1)
            m_lampIndex = i;
         m_nLamps++;
      }
      else if (m_deviceStateSrc.deviceDefs[i].groupId == 0x0300)
      {
         if (m_mechIndex == -1)
            m_mechIndex = i;
         m_nMechs++;
      }
      else if ((m_GIIndex == -1) && (m_lampIndex == -1))
      {
         m_nSolenoids++;
      }
   }

   LOGI("B2SLegacy: Device state updated - Solenoids: %d, Lamps: %d, GI: %d, Mechs: %d",
        m_nSolenoids, m_nLamps, m_nGIs, m_nMechs);
}

int Server::OnRenderStatic(VPXRenderContext2D* ctx, void* userData)
{
   return static_cast<Server*>(userData)->OnRender(ctx, userData);
}

void Server::OnGetRendererStatic(const unsigned int, void* userData, void* msgData)
{
   auto me = static_cast<Server*>(userData);
   auto msg = static_cast<GetAncillaryRendererMsg*>(msgData);

   const AncillaryRendererDef backglassEntry = { "B2SLegacy", "B2S Legacy Backglass", "Renderer for B2S legacy backglass files", me, OnRenderStatic };
   const AncillaryRendererDef dmdEntry = { "B2SLegacyDMD", "B2S Legacy DMD", "Renderer for B2S legacy DMD files", me, OnRenderStatic };

   if (msg->window == VPXWindowId::VPXWINDOW_Backglass) {
      if (msg->count < msg->maxEntryCount)
         msg->entries[msg->count] = backglassEntry;
      msg->count++;
   }
   else if (msg->window == VPXWindowId::VPXWINDOW_ScoreView) {
      if (msg->count < msg->maxEntryCount)
         msg->entries[msg->count] = dmdEntry;
      msg->count++;
   }
}

void Server::OnDevSrcChangedStatic(const unsigned int msgId, void* userData, void* msgData)
{
   static_cast<Server*>(userData)->OnDevSrcChanged(msgId, userData, msgData);
}

void Server::ForwardPinMAMECall(int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet)
{
   if (m_pinmameApi)
      m_pinmameApi->HandleCall(memberIndex + m_pinmameMemberStartIndex, m_pinmameMemberStartIndex, pArgs, pRet);
}

void Server::TimerElapsed(Timer* pTimer)
{
   // have a look for important pollings
   static int counter = 0;
   static bool callLamps = false;
   static bool callSolenoids = false;
   static bool callGIStrings = false;
   static bool callLEDs = false;
   static bool logged = false;

   if (counter <= 25) {
      counter++;
      callLamps = !m_changedLampsCalled && (m_pB2SData->IsUseRomLamps() || m_pB2SData->IsUseAnimationLamps());
      callSolenoids = !m_changedSolenoidsCalled && (m_pB2SData->IsUseRomSolenoids() || m_pB2SData->IsUseAnimationSolenoids());
      callGIStrings = !m_changedGIStringsCalled && (m_pB2SData->IsUseRomGIStrings() || m_pB2SData->IsUseAnimationGIStrings());
      callLEDs = !m_changedLEDsCalled && (m_pB2SData->IsUseLEDs() || m_pB2SData->IsUseLEDDisplays() || m_pB2SData->IsUseReels());
   }
   else {
      if (m_pB2SSettings->IsROMControlled()) {
         bool changed = false;
         if (callLamps) {
            if (!m_changedLampsCalled)
               GetChangedLamps();
            else {
               callLamps = false;
               changed = true;
            }
         }
         if (callSolenoids) {
            if (!m_changedSolenoidsCalled)
               GetChangedSolenoids();
            else {
               callSolenoids = false;
               changed = true;
            }
         }
         if (callGIStrings) {
            if (!m_changedGIStringsCalled)
               GetChangedGIStrings();
            else {
               callGIStrings = false;
               changed = true;
            }
         }
         if (callLEDs) {
            if (!m_changedLEDsCalled)
               GetChangedLEDs();
            else {
               callLEDs = false;
               changed = true;
            }
         }

         if (!logged || changed) {
            LOGI("B2S polling status: lamps=%d, solenoids=%d, giStrings=%d, leds=%d", callLamps, callSolenoids, callGIStrings, callLEDs);

            if (!callLamps && !callSolenoids && !callGIStrings && !callLEDs)
               pTimer->Stop();

            logged = true;
         }
      }
   }
}

void Server::Dispose()
{
}

const string& Server::GetB2SServerVersion()
{
   static const string v(B2S_VERSION_STRING);
   return v;
}

double Server::GetB2SBuildVersion()
{
   return B2S_VERSION_MAJOR * 10000.0
      + B2S_VERSION_MINOR * 100.0
      + B2S_VERSION_REVISION
      + B2S_VERSION_BUILD / 10000.0;
}

const string& Server::GetB2SServerDirectory() const
{
   return m_szPath;
}

const string& Server::GetB2SName() const
{
   return m_pB2SSettings->GetB2SName();
}

void Server::SetB2SName(const string& b2sName)
{
   m_pB2SSettings->SetB2SName(b2sName);
}

const string& Server::GetTableName() const
{
   return m_pB2SData->GetTableName();
}

void Server::SetTableName(const string& tableName)
{
   m_pB2SData->SetTableName(tableName);
}

void Server::SetWorkingDir(const string& workingDir)
{
   m_szPath = workingDir;
}

void Server::SetPath(const string& path)
{
   m_szPath = path;
}

void Server::Run(int handle)
{
   Startup();

   ShowBackglassForm();

   m_pTimer->Start();
}

void Server::Stop()
{
   m_pTimer->Stop();
   HideBackglassForm();

   m_pB2SData->Stop();
   KillBackglassForm();
}

bool Server::GetLaunchBackglass() const
{
   return false;
}

void Server::SetLaunchBackglass(bool launchBackglass)
{
}

bool Server::GetPuPHide() const
{
   return false;
}

void Server::SetPuPHide(bool puPHide)
{
}

void Server::GetChangedLamps()
{
   if (!m_pinmameApi)
      return;

   ScriptArray* lampArray = m_pinmameApi->GetChangedLamps();
   if (m_pB2SData->IsLampsData() && lampArray)
      CheckLamps(lampArray);
}

void Server::GetChangedLamps(ScriptVariant* pRet)
{
   m_changedLampsCalled = true;
   if (m_pB2SData->IsLampsData() && pRet && pRet->vArray)
      CheckLamps(pRet->vArray);
}

void Server::GetChangedSolenoids()
{
   if (!m_pinmameApi)
      return;

   ScriptArray* solenoidArray = m_pinmameApi->GetChangedSolenoids();
   if (m_pB2SData->IsSolenoidsData() && solenoidArray)
      CheckSolenoids(solenoidArray);
}

void Server::GetChangedSolenoids(ScriptVariant* pRet)
{
   m_changedSolenoidsCalled = true;
   if (m_pB2SData->IsSolenoidsData() && pRet && pRet->vArray)
      CheckSolenoids(pRet->vArray);
}

void Server::GetChangedGIStrings()
{
   if (!m_pinmameApi)
      return;

   ScriptArray* giStringArray = m_pinmameApi->GetChangedGIStrings();
   if (m_pB2SData->IsGIStringsData() && giStringArray)
      CheckGIStrings(giStringArray);
}

void Server::GetChangedGIStrings(ScriptVariant* pRet)
{
   m_changedGIStringsCalled = true;
   if (m_pB2SData->IsGIStringsData() && pRet && pRet->vArray)
      CheckGIStrings(pRet->vArray);
}

void Server::GetChangedLEDs()
{
   if (!m_pinmameApi)
      return;

   ScriptArray* ledArray = m_pinmameApi->GetChangedLEDs();
   if (m_pB2SData->IsLEDsData() && ledArray)
      CheckLEDs(ledArray);
}

void Server::GetChangedLEDs(ScriptVariant* pRet)
{
   m_changedLEDsCalled = true;
   if (m_pB2SData->IsLEDsData() && pRet && pRet->vArray)
      CheckLEDs(pRet->vArray);
}

void Server::SetSwitch(int switchId, bool value)
{
   if (m_pinmameApi)
      m_pinmameApi->SetSwitch(switchId, value);
}

void Server::B2SSetData(int id, int value)
{
   MyB2SSetData(id, value);
}

void Server::B2SSetData(const string& name, int value)
{
   MyB2SSetData(name, value);
}

void Server::B2SPulseData(int id)
{
   MyB2SSetData(id, 1);
   MyB2SSetData(id, 0);
}

void Server::B2SPulseData(const string& name)
{
   MyB2SSetData(name, 1);
   MyB2SSetData(name, 0);
}

void Server::B2SSetPos(int id, int xpos, int ypos)
{
   MyB2SSetPos(id, xpos, ypos);
}

void Server::B2SSetPos(const string& name, int xpos, int ypos)
{
   MyB2SSetPos(string_to_int(name, 0), xpos, ypos);
}

void Server::B2SSetIllumination(const string& name, int value)
{
   MyB2SSetData(name, value);
}

void Server::B2SSetLED(int digit, int value)
{
   MyB2SSetLED(digit, value);
}

void Server::B2SSetLED(int digit, const string& text)
{
   MyB2SSetLED(digit, text);
}

void Server::B2SSetLEDDisplay(int display, const string& text)
{
   MyB2SSetLEDDisplay(display, text);
}

void Server::B2SSetReel(int digit, int value)
{
   MyB2SSetScore(digit, value, true);
}

void Server::B2SSetScore(int display, int value)
{
   MyB2SSetScore(GetFirstDigitOfDisplay(display), value, false);
}

void Server::B2SSetScorePlayer(int playerno, int score)
{
   MyB2SSetScorePlayer(playerno, score);
}

void Server::B2SSetScorePlayer1(int score)
{
   MyB2SSetScore(1, score, false);
}

void Server::B2SSetScorePlayer2(int score)
{
   MyB2SSetScore(2, score, false);
}

void Server::B2SSetScorePlayer3(int score)
{
   MyB2SSetScore(3, score, false);
}

void Server::B2SSetScorePlayer4(int score)
{
   MyB2SSetScore(4, score, false);
}

void Server::B2SSetScorePlayer5(int score)
{
   MyB2SSetScore(5, score, false);
}

void Server::B2SSetScorePlayer6(int score)
{
   MyB2SSetScore(6, score, false);
}

void Server::B2SSetScoreDigit(int digit, int value)
{
   MyB2SSetScore(digit, value, false);
}

void Server::B2SSetScoreRollover(int id, int value)
{
   MyB2SSetData(id, value);
}

void Server::B2SSetScoreRolloverPlayer1(int value)
{
   MyB2SSetData(25, value);
}

void Server::B2SSetScoreRolloverPlayer2(int value)
{
   MyB2SSetData(26, value);
}

void Server::B2SSetScoreRolloverPlayer3(int value)
{
   MyB2SSetData(27, value);
}

void Server::B2SSetScoreRolloverPlayer4(int value)
{
   MyB2SSetData(28, value);
}

void Server::B2SSetCredits(int value)
{
   MyB2SSetScore(29, value, false);
}

void Server::B2SSetCredits(int digit, int value)
{
   MyB2SSetScore(digit, value, false);
}

void Server::B2SSetPlayerUp(int value)
{
   MyB2SSetData(30, value);
}

void Server::B2SSetPlayerUp(int id, int value)
{
   MyB2SSetData(id, value);
}

void Server::B2SSetCanPlay(int value)
{
   MyB2SSetData(31, value);
}

void Server::B2SSetCanPlay(int id, int value)
{
   MyB2SSetData(id, value);
}

void Server::B2SSetBallInPlay(int value)
{
   MyB2SSetData(32, value);
}

void Server::B2SSetBallInPlay(int id, int value)
{
   MyB2SSetData(id, value);
}

void Server::B2SSetTilt(int value)
{
   MyB2SSetData(33, value);
}

void Server::B2SSetTilt(int id, int value)
{
   MyB2SSetData(id, value);
}

void Server::B2SSetMatch(int value)
{
   MyB2SSetData(34, value);
}

void Server::B2SSetMatch(int id, int value)
{
   MyB2SSetData(id, value);
}

void Server::B2SSetGameOver(int value)
{
   MyB2SSetData(35, value);
}

void Server::B2SSetGameOver(int id, int value)
{
   MyB2SSetData(id, value);
}

void Server::B2SSetShootAgain(int value)
{
   MyB2SSetData(36, value);
}

void Server::B2SSetShootAgain(int id, int value)
{
   MyB2SSetData(id, value);
}

void Server::B2SStartAnimation(const string& animationname)
{
   MyB2SStartAnimation(animationname, false);
}

void Server::B2SStartAnimation(const string& animationname, bool playreverse)
{
   MyB2SStartAnimation(animationname, playreverse);
}

void Server::B2SStartAnimationReverse(const string& animationname)
{
   MyB2SStartAnimation(animationname, true);
}

void Server::B2SStopAnimation(const string& animationname)
{
   MyB2SStopAnimation(animationname);
}

void Server::B2SStopAllAnimations()
{
   MyB2SStopAllAnimations();
}

bool Server::B2SIsAnimationRunning(const string& animationname) const
{
   return MyB2SIsAnimationRunning(animationname);
}

void Server::StartAnimation(const string& animationname)
{
   MyB2SStartAnimation(animationname, false);
}

void Server::StartAnimation(const string& animationname, bool playreverse)
{
   MyB2SStartAnimation(animationname, playreverse);
}

void Server::StopAnimation(const string& animationname)
{
   MyB2SStopAnimation(animationname);
}

void Server::B2SStartRotation()
{
   MyB2SStartRotation();
}

void Server::B2SStopRotation()
{
   MyB2SStopRotation();
}

void Server::B2SShowScoreDisplays()
{
   MyB2SShowScoreDisplays();
}

void Server::B2SHideScoreDisplays()
{
   MyB2SHideScoreDisplays();
}

void Server::B2SStartSound(const string& soundname)
{
   MyB2SPlaySound(soundname);
}

void Server::B2SPlaySound(const string& soundname)
{
   MyB2SPlaySound(soundname);
}

void Server::B2SStopSound(const string& soundname)
{
   MyB2SStopSound(soundname);
}

void Server::B2SMapSound(int digit, const string& soundname)
{
}

void Server::MyB2SSetData(int id, int value)
{
   if (m_pB2SData->IsBackglassRunning()) {
      // Handle top/second light switching based on ROM IDs
      if ((m_pFormBackglass->GetTopRomIDType() == eRomIDType_Lamp && m_pFormBackglass->GetTopRomID() == id) ||
          (m_pFormBackglass->GetSecondRomIDType() == eRomIDType_Lamp && m_pFormBackglass->GetSecondRomID() == id)) {

         bool topVisible = m_lastTopVisible;
         bool secondVisible = m_lastSecondVisible;

         if (m_pFormBackglass->GetTopRomIDType() == eRomIDType_Lamp && m_pFormBackglass->GetTopRomID() == id) {
            topVisible = (value != 0);
            if (m_pFormBackglass->IsTopRomInverted())
               topVisible = !topVisible;
         }
         else if (m_pFormBackglass->GetSecondRomIDType() == eRomIDType_Lamp && m_pFormBackglass->GetSecondRomID() == id) {
            secondVisible = (value != 0);
            if (m_pFormBackglass->IsSecondRomInverted())
               secondVisible = !secondVisible;
         }

         if (m_lastTopVisible != topVisible || m_lastSecondVisible != secondVisible || !m_isVisibleStateSet) {
            m_pB2SData->SetOffImageVisible(false);
            m_isVisibleStateSet = true;
            m_lastTopVisible = topVisible;
            m_lastSecondVisible = secondVisible;

            if (topVisible && secondVisible)
               m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetTopAndSecondLightImage());
            else if (topVisible)
               m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetTopLightImage());
            else if (secondVisible)
               m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetSecondLightImage());
            else {
               m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetDarkImage());
               m_pB2SData->SetOffImageVisible(true);
            }
         }
      }

      // Handle individual illumination controls
      if (m_pB2SData->GetUsedRomLampIDs()->contains(id)) {
         for(const auto& pBase : (*m_pB2SData->GetUsedRomLampIDs())[id]) {
            B2SPictureBox* pPicbox = dynamic_cast<B2SPictureBox*>(pBase);
            if (pPicbox && (!m_pB2SData->IsUseIlluminationLocks() || pPicbox->GetGroupName().empty() || !m_pB2SData->GetIlluminationLocks()->contains(pPicbox->GetGroupName()))) {
               if (pPicbox->GetRomIDValue() > 0)
                  pPicbox->SetVisible(pPicbox->GetRomIDValue() == value);
               else {
                  bool visible = (value != 0);
                  if (pPicbox->IsRomInverted())
                     visible = !visible;
                  pPicbox->SetVisible(visible);
               }
            }
         }
      }
   }
}

void Server::MyB2SSetData(const string& groupname, int value)
{
   if (!m_pB2SData->IsBackglassRunning())
      return;

   // Only do the illumination stuff if the group has a name
   if (!groupname.empty() && m_pB2SData->GetIlluminationGroups()->contains(groupname)) {
      // Get all matching picture boxes
      for(const auto& pPicbox : (*m_pB2SData->GetIlluminationGroups())[groupname]) {
         if (pPicbox->GetPictureBoxType() == ePictureBoxType_StandardImage) {
            if (pPicbox->GetRomIDValue() > 0)
               pPicbox->SetVisible(pPicbox->GetRomIDValue() == value);
            else
               pPicbox->SetVisible(value != 0);
         }
      }
   }
}

void Server::MyB2SSetPos(int id, int xpos, int ypos)
{
   if (!m_pB2SData->IsBackglassRunning())
      return;

   const SDL_FRect& rescaleBackglass = m_pFormBackglass->GetScaleFactor();

   if (m_pB2SData->GetUsedRomLampIDs()->contains(id)) {
      for (const auto& pBase : (*m_pB2SData->GetUsedRomLampIDs())[id]) {
         B2SPictureBox* pPicbox = dynamic_cast<B2SPictureBox*>(pBase);
         if (pPicbox && (!m_pB2SData->IsUseIlluminationLocks() || pPicbox->GetGroupName().empty() || !m_pB2SData->GetIlluminationLocks()->contains(pPicbox->GetGroupName()))) {
            if (pPicbox->GetLeft() != xpos || pPicbox->GetTop() != ypos) {
               pPicbox->SetLeft(xpos);
               pPicbox->SetTop(ypos);
               const SDL_FRect& rectF = pPicbox->GetRectangleF();
               pPicbox->SetRectangleF({ static_cast<float>(pPicbox->GetLeft()) / rescaleBackglass.w,
                                        static_cast<float>(pPicbox->GetTop()) / rescaleBackglass.h,
                                        rectF.w, rectF.h });
               if (pPicbox->GetParent())
                  pPicbox->GetParent()->Invalidate();
            }
         }
      }
   }
}

void Server::CheckGetMech(int number, int mech)
{
   if (!m_ready)
      return;

   if (number > 0) {
      int mechid = number;
      int mechvalue = mech;

      if (!m_pB2SSettings->GetBackglassFileVersion().empty() && m_pB2SSettings->GetBackglassFileVersion() <= "1.1")
         mechvalue -= 1;

      if (m_pB2SData->GetUsedRomMechIDs()->contains(mechid)) {
         if ((*m_pB2SData->GetRotatingPictureBox())[mechid] && m_pB2SData->GetRotatingImages()->contains(mechid) && !(*m_pB2SData->GetRotatingImages())[mechid].empty() && (*m_pB2SData->GetRotatingImages())[mechid].contains(mechvalue)) {
            (*m_pB2SData->GetRotatingPictureBox())[mechid]->SetBackgroundImage((*m_pB2SData->GetRotatingImages())[mechid][mechvalue]);
            (*m_pB2SData->GetRotatingPictureBox())[mechid]->SetVisible(true);
         }
      }
   }
}

void Server::CheckLamps(ScriptArray* psa)
{
   if (m_deviceStateSrc.deviceDefs == nullptr || m_nLamps == 0)
      return;

   for (unsigned int i = 0; i < m_nLamps; i++)
   {
      if (m_deviceStateSrc.deviceDefs[m_lampIndex + i].groupId == 0x0200)
      {
         const int lampId = m_deviceStateSrc.deviceDefs[m_lampIndex + i].deviceId;
         const float state = m_deviceStateSrc.GetFloatState(m_lampIndex + i);
         const int lampState = static_cast<int>(state * 255.0f);

         if (m_pB2SData->IsUseRomLamps() || m_pB2SData->IsUseAnimationLamps()) {
            // collect illumination data
            if (m_pFormBackglass->GetTopRomIDType() == eRomIDType_Lamp && m_pFormBackglass->GetTopRomID() == lampId)
               m_pCollectLampsData->Add(lampId, new CollectData((int)lampState, eCollectedDataType_TopImage));
            else if (m_pFormBackglass->GetSecondRomIDType() == eRomIDType_Lamp && m_pFormBackglass->GetSecondRomID() == lampId)
               m_pCollectLampsData->Add(lampId, new CollectData((int)lampState, eCollectedDataType_SecondImage));
            if (m_pB2SData->GetUsedRomLampIDs()->contains(lampId))
               m_pCollectLampsData->Add(lampId, new CollectData((int)lampState, eCollectedDataType_Standard));

            // collect animation data
            if (m_pB2SData->GetUsedAnimationLampIDs()->contains(lampId) || m_pB2SData->GetUsedRandomAnimationLampIDs()->contains(lampId))
               m_pCollectLampsData->Add(lampId, new CollectData((int)lampState, eCollectedDataType_Animation));
         }
      }
   }

   // one collection loop is done
   m_pCollectLampsData->DataAdded();

   if (!m_ready)
      return;

   // maybe show the collected data
   if (m_pCollectLampsData->ShowData()) {
      m_pCollectLampsData->Lock();

      for (const auto& [key, pCollectData] : *m_pCollectLampsData) {
         int lampId = key;
         bool lampState = (pCollectData->GetState() > 0);
         int datatypes = pCollectData->GetTypes();

         // illumination stuff
         if ((datatypes & eCollectedDataType_TopImage) || (datatypes & eCollectedDataType_SecondImage)) {
            bool topvisible = m_lastTopVisible;
            bool secondvisible = m_lastSecondVisible;
            if (datatypes & eCollectedDataType_TopImage) {
               topvisible = lampState;
               if (m_pFormBackglass->IsTopRomInverted())
                  topvisible = !topvisible;
            }
            else if (datatypes & eCollectedDataType_SecondImage) {
               secondvisible = lampState;
               if (m_pFormBackglass->IsSecondRomInverted())
                  topvisible = !topvisible;
            }
            if (m_lastTopVisible != topvisible || m_lastSecondVisible != secondvisible || !m_isVisibleStateSet) {
               m_pB2SData->SetOffImageVisible(false);
               m_isVisibleStateSet = true;
               m_lastTopVisible = topvisible;
               m_lastSecondVisible = secondvisible;
               if (topvisible && secondvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetTopAndSecondLightImage());
               else if (topvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetTopLightImage());
               else if (secondvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetSecondLightImage());
               else {
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetDarkImage());
                  m_pB2SData->SetOffImageVisible(true);
               }
            }
         }
         if (datatypes & eCollectedDataType_Standard) {
            for (const auto& pBase : (*m_pB2SData->GetUsedRomLampIDs())[lampId]) {
               B2SPictureBox* pPicbox = dynamic_cast<B2SPictureBox*>(pBase);
               if (pPicbox && (!m_pB2SData->IsUseIlluminationLocks() || pPicbox->GetGroupName().empty() || !m_pB2SData->GetIlluminationLocks()->contains(pPicbox->GetGroupName()))) {
                  bool visible = lampState;
                  if (pPicbox->IsRomInverted())
                     visible = !visible;
                  if (m_pB2SData->IsUseRotatingImage() && m_pB2SData->GetRotatingPictureBox() && (*m_pB2SData->GetRotatingPictureBox())[0] && pPicbox == (*m_pB2SData->GetRotatingPictureBox())[0]) {
                     if (visible)
                        m_pFormBackglass->StartRotation();
                     else
                        m_pFormBackglass->StopRotation();
                  }
                  else
                     pPicbox->SetVisible(visible);
               }
            }
         }

         // animation stuff
         if (datatypes & eCollectedDataType_Animation) {
            if (m_pB2SData->GetUsedAnimationLampIDs()->contains(lampId)) {
               for (const auto& animation : (*m_pB2SData->GetUsedAnimationLampIDs())[lampId]) {
                  bool start = lampState;
                  if (animation->IsInverted())
                     start = !start;
                  if (start)
                     m_pFormBackglass->StartAnimation(animation->GetAnimationName());
                  else
                     m_pFormBackglass->StopAnimation(animation->GetAnimationName());
               }
            }
            // random animation start
            if (m_pB2SData->GetUsedRandomAnimationLampIDs()->contains(lampId)) {
               bool start = lampState;
               bool isrunning = false;
               if (start) {
                  for (const auto& matchinganimation : (*m_pB2SData->GetUsedRandomAnimationLampIDs())[lampId]) {
                     if (m_pFormBackglass->IsAnimationRunning(matchinganimation->GetAnimationName())) {
                        isrunning = true;
                        break;
                     }
                  }
               }
               if (start) {
                  if (!isrunning) {
                     int random = RandomStarter((int)(*m_pB2SData->GetUsedRandomAnimationLampIDs())[lampId].size());
                     auto& animation = (*m_pB2SData->GetUsedRandomAnimationLampIDs())[lampId][random];
                     m_lastRandomStartedAnimation = animation->GetAnimationName();
                     m_pFormBackglass->StartAnimation(m_lastRandomStartedAnimation);
                  }
               }
               else {
                  if (!m_lastRandomStartedAnimation.empty()) {
                     m_pFormBackglass->StopAnimation(m_lastRandomStartedAnimation);
                     m_lastRandomStartedAnimation.clear();
                  }
               }
            }
         }
      }
      m_pCollectLampsData->Unlock();

      // reset all current data
      m_pCollectLampsData->ClearData(m_pB2SSettings->GetLampsSkipFrames());
   }
}

void Server::CheckSolenoids(ScriptArray* psa)
{
   if (m_deviceStateSrc.deviceDefs == nullptr || m_nSolenoids == 0)
      return;

   for (unsigned int i = 0; i < m_nSolenoids; i++)
   {
      const float state = m_deviceStateSrc.GetFloatState(i);
      const int solenoidId = i + 1;
      const int solenoidState = static_cast<int>(state * 255.0f);

      if (m_pB2SData->IsUseRomSolenoids() || m_pB2SData->IsUseAnimationSolenoids()) {
         // collect illumination data
         if (m_pFormBackglass->GetTopRomIDType() == eRomIDType_Solenoid && m_pFormBackglass->GetTopRomID() == solenoidId)
            m_pCollectSolenoidsData->Add(solenoidId, new CollectData(solenoidState, eCollectedDataType_TopImage));
         else if (m_pFormBackglass->GetSecondRomIDType() == eRomIDType_Solenoid && m_pFormBackglass->GetSecondRomID() == solenoidId)
            m_pCollectSolenoidsData->Add(solenoidId, new CollectData(solenoidState, eCollectedDataType_SecondImage));
         if (m_pB2SData->GetUsedRomSolenoidIDs()->contains(solenoidId))
            m_pCollectSolenoidsData->Add(solenoidId, new CollectData(solenoidState, eCollectedDataType_Standard));

         // collect animation data
         if (m_pB2SData->GetUsedAnimationSolenoidIDs()->contains(solenoidId) || m_pB2SData->GetUsedRandomAnimationSolenoidIDs()->contains(solenoidId))
            m_pCollectSolenoidsData->Add(solenoidId, new CollectData(solenoidState, eCollectedDataType_Animation));
      }
   }

   // one collection loop is done
   m_pCollectSolenoidsData->DataAdded();

   if (!m_ready)
      return;

   // maybe show the collected data
   if (m_pCollectSolenoidsData->ShowData()) {
      m_pCollectSolenoidsData->Lock();

      for (const auto& [key, pCollectData] : *m_pCollectSolenoidsData) {
         int solenoidId = key;
         int solenoidState = pCollectData->GetState();
         int datatypes = pCollectData->GetTypes();

         // illumination stuff
         if ((datatypes & eCollectedDataType_TopImage) || (datatypes & eCollectedDataType_SecondImage)) {
            bool topvisible = m_lastTopVisible;
            bool secondvisible = m_lastSecondVisible;
            if (datatypes & eCollectedDataType_TopImage) {
               topvisible = (solenoidState != 0);
               if (m_pFormBackglass->IsTopRomInverted())
                  topvisible = !topvisible;
            }
            else if (datatypes & eCollectedDataType_SecondImage) {
               secondvisible = (solenoidState != 0);
               if (m_pFormBackglass->IsSecondRomInverted())
                  topvisible = !topvisible;
            }
            if (m_lastTopVisible != topvisible || m_lastSecondVisible != secondvisible || !m_isVisibleStateSet) {
               m_pB2SData->SetOffImageVisible(false);
               m_isVisibleStateSet = true;
               m_lastTopVisible = topvisible;
               m_lastSecondVisible = secondvisible;
               if (topvisible && secondvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetTopAndSecondLightImage());
               else if (topvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetTopLightImage());
               else if (secondvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetSecondLightImage());
               else {
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetDarkImage());
                  m_pB2SData->SetOffImageVisible(true);
               }
            }
         }
         if (datatypes & eCollectedDataType_Standard) {
            for (const auto& pBase : (*m_pB2SData->GetUsedRomSolenoidIDs())[solenoidId]) {
               B2SPictureBox* pPicbox = dynamic_cast<B2SPictureBox*>(pBase);
               if (pPicbox && (!m_pB2SData->IsUseIlluminationLocks() || pPicbox->GetGroupName().empty() || !m_pB2SData->GetIlluminationLocks()->contains(pPicbox->GetGroupName()))) {
                  bool visible = (solenoidState != 0);
                  if (pPicbox->IsRomInverted())
                     visible = !visible;
                  if (m_pB2SData->IsUseRotatingImage() && m_pB2SData->GetRotatingPictureBox() && (*m_pB2SData->GetRotatingPictureBox())[0] && pPicbox == (*m_pB2SData->GetRotatingPictureBox())[0]) {
                     if (visible)
                        m_pFormBackglass->StartRotation();
                     else
                        m_pFormBackglass->StopRotation();
                  }
                  else
                     pPicbox->SetVisible(visible);
               }
            }
         }

         // animation stuff
         if (datatypes & eCollectedDataType_Animation) {
            if (m_pB2SData->GetUsedAnimationSolenoidIDs()->contains(solenoidId)) {
               for (const auto& animation : (*m_pB2SData->GetUsedAnimationSolenoidIDs())[solenoidId]) {
                  bool start = (solenoidState != 0);
                  if (animation->IsInverted())
                     start = !start;
                  if (start)
                     m_pFormBackglass->StartAnimation(animation->GetAnimationName());
                  else
                     m_pFormBackglass->StopAnimation(animation->GetAnimationName());
               }
            }
            // random animation start
            if (m_pB2SData->GetUsedRandomAnimationSolenoidIDs()->contains(solenoidId)) {
               bool start = (solenoidState != 0);
               bool isrunning = false;
               if (start) {
                  for (const auto& matchinganimation : (*m_pB2SData->GetUsedRandomAnimationSolenoidIDs())[solenoidId]) {
                     if (m_pFormBackglass->IsAnimationRunning(matchinganimation->GetAnimationName())) {
                        isrunning = true;
                        break;
                     }
                  }
               }
               if (start) {
                  if (!isrunning) {
                     int random = RandomStarter((int)(*m_pB2SData->GetUsedRandomAnimationSolenoidIDs())[solenoidId].size());
                     auto& animation = (*m_pB2SData->GetUsedRandomAnimationSolenoidIDs())[solenoidId][random];
                     m_lastRandomStartedAnimation = animation->GetAnimationName();
                     m_pFormBackglass->StartAnimation(m_lastRandomStartedAnimation);
                  }
               }
               else {
                  if (!m_lastRandomStartedAnimation.empty()) {
                     m_pFormBackglass->StopAnimation(m_lastRandomStartedAnimation);
                     m_lastRandomStartedAnimation.clear();
                  }
               }
            }
         }
      }
      m_pCollectSolenoidsData->Unlock();

      // reset all current data
      m_pCollectSolenoidsData->ClearData(m_pB2SSettings->GetSolenoidsSkipFrames());
   }
}

void Server::CheckGIStrings(ScriptArray* psa)
{
   if (m_deviceStateSrc.deviceDefs == nullptr || m_nGIs == 0 || m_GIIndex == -1)
      return;

   for (unsigned int i = 0; i < m_nGIs; i++)
   {
      if (m_deviceStateSrc.deviceDefs[m_GIIndex + i].groupId == 0x0100)
      {
         const int giStringId = m_deviceStateSrc.deviceDefs[m_GIIndex + i].deviceId;
         const float state = m_deviceStateSrc.GetFloatState(m_GIIndex + i);
         const int giStringBool = static_cast<int>(state * 255.0f) > m_giStringThreshold;

         if (m_pB2SData->IsUseRomGIStrings() || m_pB2SData->IsUseAnimationGIStrings()) {
            // collect illumination data
            if (m_pFormBackglass->GetTopRomIDType() == eRomIDType_GIString && m_pFormBackglass->GetTopRomID() == giStringId)
               m_pCollectGIStringsData->Add(giStringId, new CollectData((int)giStringBool, eCollectedDataType_TopImage));
            else if (m_pFormBackglass->GetSecondRomIDType() == eRomIDType_GIString && m_pFormBackglass->GetSecondRomID() == giStringId)
               m_pCollectGIStringsData->Add(giStringId, new CollectData((int)giStringBool, eCollectedDataType_SecondImage));
            if (m_pB2SData->GetUsedRomGIStringIDs()->contains(giStringId))
               m_pCollectGIStringsData->Add(giStringId, new CollectData((int)giStringBool, eCollectedDataType_Standard));

            // collect animation data
            if (m_pB2SData->GetUsedAnimationGIStringIDs()->contains(giStringId) || m_pB2SData->GetUsedRandomAnimationGIStringIDs()->contains(giStringId))
               m_pCollectGIStringsData->Add(giStringId, new CollectData((int)giStringBool, eCollectedDataType_Animation));
         }
      }
   }

   // one collection loop is done
   m_pCollectGIStringsData->DataAdded();

   if (!m_ready)
      return;

   // maybe show the collected data
   if (m_pCollectGIStringsData->ShowData()) {
      m_pCollectGIStringsData->Lock();

      for (const auto& [key, pCollectData] : *m_pCollectGIStringsData) {
         int giStringId = key;
         bool giStringBool = (pCollectData->GetState() > 0);
         int datatypes = pCollectData->GetTypes();

         // illumination stuff
         if ((datatypes & eCollectedDataType_TopImage) || (datatypes & eCollectedDataType_SecondImage)) {
            bool topvisible = m_lastTopVisible;
            bool secondvisible = m_lastSecondVisible;
            if (datatypes & eCollectedDataType_TopImage) {
               topvisible = giStringBool;
               if (m_pFormBackglass->IsTopRomInverted())
                  topvisible = !topvisible;
            }
            else if (datatypes & eCollectedDataType_SecondImage) {
               secondvisible = giStringBool;
               if (m_pFormBackglass->IsSecondRomInverted())
                  topvisible = !topvisible;
            }
            if (m_lastTopVisible != topvisible || m_lastSecondVisible != secondvisible || !m_isVisibleStateSet) {
               m_pB2SData->SetOffImageVisible(false);
               m_isVisibleStateSet = true;
               m_lastTopVisible = topvisible;
               m_lastSecondVisible = secondvisible;
               if (topvisible && secondvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetTopAndSecondLightImage());
               else if (topvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetTopLightImage());
               else if (secondvisible)
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetSecondLightImage());
               else {
                  m_pFormBackglass->SetBackgroundImage(m_pFormBackglass->GetDarkImage());
                  m_pB2SData->SetOffImageVisible(true);
               }
            }
         }
         if (datatypes & eCollectedDataType_Standard) {
            for (const auto& pBase : (*m_pB2SData->GetUsedRomGIStringIDs())[giStringId]) {
               B2SPictureBox* pPicbox = dynamic_cast<B2SPictureBox*>(pBase);
               if (pPicbox && (!m_pB2SData->IsUseIlluminationLocks() || pPicbox->GetGroupName().empty() || !m_pB2SData->GetIlluminationLocks()->contains(pPicbox->GetGroupName()))) {
                  bool visible = giStringBool;
                  if (pPicbox->IsRomInverted())
                     visible = !visible;
                  if (m_pB2SData->IsUseRotatingImage() && m_pB2SData->GetRotatingPictureBox() && (*m_pB2SData->GetRotatingPictureBox())[0] && pPicbox == (*m_pB2SData->GetRotatingPictureBox())[0]) {
                     if (visible)
                        m_pFormBackglass->StartRotation();
                     else
                        m_pFormBackglass->StopRotation();
                  }
                  else
                     pPicbox->SetVisible(visible);
               }
            }
         }

         // animation stuff
         if (datatypes & eCollectedDataType_Animation) {
            if (m_pB2SData->GetUsedAnimationGIStringIDs()->contains(giStringId)) {
               for (const auto& animation : (*m_pB2SData->GetUsedAnimationGIStringIDs())[giStringId]) {
                  bool start = giStringBool;
                  if (animation->IsInverted())
                     start = !start;
                  if (start)
                     m_pFormBackglass->StartAnimation(animation->GetAnimationName());
                  else
                     m_pFormBackglass->StopAnimation(animation->GetAnimationName());
               }
            }
            // random animation start
            if (m_pB2SData->GetUsedRandomAnimationGIStringIDs()->contains(giStringId)) {
               bool start = giStringBool;
               bool isrunning = false;
               if (start) {
                  for (const auto& matchinganimation : (*m_pB2SData->GetUsedRandomAnimationGIStringIDs())[giStringId]) {
                     if (m_pFormBackglass->IsAnimationRunning(matchinganimation->GetAnimationName())) {
                        isrunning = true;
                        break;
                     }
                  }
               }
               if (start) {
                  if (!isrunning) {
                     int random = RandomStarter((int)(*m_pB2SData->GetUsedRandomAnimationGIStringIDs())[giStringId].size());
                     auto& animation = (*m_pB2SData->GetUsedRandomAnimationGIStringIDs())[giStringId][random];
                     m_lastRandomStartedAnimation = animation->GetAnimationName();
                     m_pFormBackglass->StartAnimation(m_lastRandomStartedAnimation);
                  }
               }
               else {
                  if (!m_lastRandomStartedAnimation.empty()) {
                     m_pFormBackglass->StopAnimation(m_lastRandomStartedAnimation);
                     m_lastRandomStartedAnimation.clear();
                  }
               }
            }
         }
      }
      m_pCollectGIStringsData->Unlock();

      // reset all current data
      m_pCollectGIStringsData->ClearData(m_pB2SSettings->GetGIStringsSkipFrames());
   }
}

void Server::CheckLEDs(ScriptArray* psa)
{
   if (psa == nullptr || psa->lengths[0] == 0)
      return;

   int digit;
   int value;

   int uCount = psa->lengths[0];
   int32_t* data = reinterpret_cast<int32_t*>(&psa->lengths[2]);
   for (int i = 0; i < uCount; i++) {
      digit = data[i * 3 + 0];
      value = data[i * 3 + 2];
      if (m_pB2SData->IsUseLEDs() || m_pB2SData->IsUseLEDDisplays() || m_pB2SData->IsUseReels())
         m_pCollectLEDsData->Add(digit, new CollectData(value, 0));
   }

   // one collection loop is done
   m_pCollectLEDsData->DataAdded();

   if (!m_ready)
      return;

   // maybe show the collected data
   if (m_pCollectLEDsData->ShowData()) {
      const bool useLEDs = m_pB2SData->IsUseLEDs() && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered;
      const bool useLEDDisplays = m_pB2SData->IsUseLEDDisplays() && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Dream7;
      const bool useReels = m_pB2SData->IsUseReels();

      m_pCollectLEDsData->Lock();

      for (const auto& [key, pCollectData] : *m_pCollectLEDsData) {
         digit = key;
         value = pCollectData->GetState();

         if (useLEDs) {
            // rendered LEDs are used
            const auto& led = m_pB2SData->GetLEDs()->find("LEDBox" + std::to_string(digit + 1));
            if (led != m_pB2SData->GetLEDs()->end())
               led->second->SetValue(value);
         }

         if (useLEDDisplays) {
            // Dream 7 displays are used
            const auto& dream7 = m_pB2SData->GetLEDDisplayDigits()->find(digit);
            if (dream7 != m_pB2SData->GetLEDDisplayDigits()->end())
               dream7->second->GetLEDDisplay()->SetValue(dream7->second->GetDigit(), value);
         }

          if (useReels) {
            // reels are used
            const auto& reel = m_pB2SData->GetReels()->find("ReelBox" + std::to_string(digit + 1));
            if (reel != m_pB2SData->GetReels()->end())
               reel->second->SetValue(value);
         }
      }
      m_pCollectLEDsData->Unlock();

      // reset all current data
      m_pCollectLEDsData->ClearData(m_pB2SSettings->GetLEDsSkipFrames());
   }
}

void Server::MyB2SSetLED(int digit, int value)
{
   if (!m_pB2SData->IsBackglassRunning())
      return;

   const auto& led = m_pB2SData->GetLEDs()->find(string("LEDBox" + std::to_string(digit)));
   const bool useLEDs = led != m_pB2SData->GetLEDs()->end() && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered;
   const auto& dream7 = m_pB2SData->GetLEDDisplayDigits()->find(digit - 1);
   const bool useLEDDisplays = dream7 != m_pB2SData->GetLEDDisplayDigits()->end() && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Dream7;

   if (useLEDs) {
      // Rendered LEDs are used
      led->second->SetValue(value);
   }
   else if (useLEDDisplays) {
      // Dream 7 displays are used
      dream7->second->GetLEDDisplay()->SetValue(dream7->second->GetDigit(), value);
   }
}

void Server::MyB2SSetLED(int digit, const string& value)
{
   if (!m_pB2SData->IsBackglassRunning())
      return;

   const bool useLEDs = m_pB2SData->GetLEDs()->contains(string("LEDBox" + std::to_string(digit))) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered;
   const auto& dream7 = m_pB2SData->GetLEDDisplayDigits()->find(digit - 1);
   const bool useLEDDisplays = dream7 != m_pB2SData->GetLEDDisplayDigits()->end() && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Dream7;

   if (useLEDs) {
      // Rendered LEDs do not support string values
   }
   else if (useLEDDisplays) {
      // Dream 7 displays are used
      dream7->second->GetLEDDisplay()->SetValue(dream7->second->GetDigit(), value);
   }
}

void Server::MyB2SSetLEDDisplay(int display, const string& szText)
{
   if (!m_pB2SData->IsBackglassRunning())
      return;

   int digit = GetFirstDigitOfDisplay(display);

   const bool useLEDs = m_pB2SData->GetLEDs()->contains(string("LEDBox" + std::to_string(digit))) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered;
   //const bool useLEDDisplays = m_pB2SData->GetLEDDisplayDigits()->contains(digit - 1) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Dream7;

   if (useLEDs) {
      // Set text for each character position in the LED display
      for (size_t i = 0; i < szText.length(); i++) {
         int ledDigit = digit + static_cast<int>(i);
         const auto& led = m_pB2SData->GetLEDs()->find("LEDBox" + std::to_string(ledDigit));

         if (led != m_pB2SData->GetLEDs()->end()) {
            // Convert character to appropriate LED value
            char c = szText[i];
            if (c >= '0' && c <= '9') {
               led->second->SetValue(c - '0'); // Convert char digit to int
            }
            else if (c >= 'A' && c <= 'F') {
               led->second->SetValue(c - 'A' + 10); // Hex A-F
            }
            else if (c >= 'a' && c <= 'f') {
               led->second->SetValue(c - 'a' + 10); // Hex a-f
            }
            else if (c == ' ') {
               led->second->SetValue(-1); // Blank/off
            }
            else {
               // For other characters, try to display as numeric value
               led->second->SetValue(static_cast<int>(c) % 16);
            }
         }
      }
   }
   else {
      const auto& dream7 = m_pB2SData->GetLEDDisplayDigits()->find(digit);
      if (dream7 != m_pB2SData->GetLEDDisplayDigits()->end())
         dream7->second->GetLEDDisplay()->SetText(szText);
   }
}

int Server::GetFirstDigitOfDisplay(int display) const
{
   int ret = 0;
   for(const auto& [key, pReelbox] : *m_pB2SData->GetReels()) {
      if (pReelbox->GetDisplayID() == display) {
         ret = pReelbox->GetStartDigit();
         break;
      }
   }
   if (ret == 0) {
      for(const auto& [key, pLedbox] : *m_pB2SData->GetLEDs()) {
         if (pLedbox->GetDisplayID() == display) {
            ret = pLedbox->GetStartDigit();
            break;
         }
      }
   }
   return ret;
}

void Server::MyB2SSetScore(int digit, int value, bool animateReelChange)
{
   if (m_pB2SData->IsBackglassRunning()) {
      if (digit > 0) {
         const auto& led = m_pB2SData->GetLEDs()->find("LEDBox" + std::to_string(digit));
         const bool useLEDs = (led != m_pB2SData->GetLEDs()->end() && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered);
         const auto& dream7 = m_pB2SData->GetLEDDisplayDigits()->find(digit - 1);
         const bool useLEDDisplays = (dream7 != m_pB2SData->GetLEDDisplayDigits()->end() && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Dream7);
         const auto& reel = m_pB2SData->GetReels()->find("ReelBox" + std::to_string(digit));
         const bool useReels = reel != m_pB2SData->GetReels()->end();

         if (useLEDs) {
            // Rendered LEDs are used
            led->second->SetText(std::to_string(value));
         }
         else if (useLEDDisplays) {
            // Dream 7 displays are used
            dream7->second->GetLEDDisplay()->SetValue(dream7->second->GetDigit(), std::to_string(value));
         }
         else if (useReels) {
            // Reels are used
            reel->second->SetText(value, animateReelChange);
         }
      }
   }
}

void Server::MyB2SSetScore(int digit, int score)
{
   if (m_pB2SData->IsBackglassRunning()) {
      if (digit > 0) {
         const auto& led = m_pB2SData->GetLEDs()->find("LEDBox" + std::to_string(digit));
         const bool useLEDs = (led != m_pB2SData->GetLEDs()->end() && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered);
         const auto& dream7 = m_pB2SData->GetLEDDisplayDigits()->find(digit - 1);
         const bool useLEDDisplays = (dream7 != m_pB2SData->GetLEDDisplayDigits()->end() && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Dream7);
         const auto& reel = m_pB2SData->GetReels()->find("ReelBox" + std::to_string(digit));
         const bool useReels = reel != m_pB2SData->GetReels()->end();

         if (useLEDs) {
            // Check the passed digit
            // Get all necessary display data
            const int startdigit = led->second->GetStartDigit();
            const int digits = led->second->GetDigits();
            const string scoreAsString = string(digits - std::to_string(score).length(), ' ') + std::to_string(score);

            // Set digits
            for (int i = startdigit + digits - 1; i >= startdigit; i--)
               (*m_pB2SData->GetLEDs())["LEDBox" + std::to_string(i)]->SetText(string(1,scoreAsString[i - startdigit]));
         }
         else if (useLEDDisplays) {
            // Get all necessary display data
            const int digits = dream7->second->GetLEDDisplay()->GetDigits();
            const string scoreAsString = string(digits - std::to_string(score).length(), ' ') + std::to_string(score);

            // Set digits
            for (int i = digits - 1; i >= 0; i--)
               dream7->second->GetLEDDisplay()->SetValue(i, string(1,scoreAsString[i]));
         }
         else if (useReels) {
            // Reels are used
            // Get all necessary display data
            const int startdigit = reel->second->GetStartDigit();
            const int digits = reel->second->GetDigits();
            const string scoreAsString = string(digits - std::to_string(score).length(), '0') + std::to_string(score);

            // Set digits
            for (int i = startdigit + digits - 1; i >= startdigit; i--)
               (*m_pB2SData->GetReels())["ReelBox" + std::to_string(i)]->SetText(scoreAsString[i - startdigit] - '0', true); // convert char to int
         }
      }
   }
}

void Server::MyB2SSetScorePlayer(int playerno, int score)
{
   if (m_pB2SData->IsBackglassRunning()) {
      if (playerno > 0) {
         const auto& it = m_pB2SData->GetPlayers()->find(playerno);
         if (it != m_pB2SData->GetPlayers()->end())
            it->second->SetScore(m_pB2SData, score);
      }
   }
}

void Server::MyB2SStartAnimation(const string& animationname, bool playreverse)
{
   if (!m_pB2SData->IsBackglassRunning())
      return;

   m_pFormBackglass->StartAnimation(animationname, playreverse);
}

void Server::MyB2SStopAnimation(const string& animationname)
{
   if (!m_pB2SData->IsBackglassRunning())
      return;

   m_pFormBackglass->StopAnimation(animationname);
}

void Server::MyB2SStopAllAnimations()
{
   if (!m_pB2SData->IsBackglassRunning())
      return;

   m_pFormBackglass->StopAllAnimations();
}

bool Server::MyB2SIsAnimationRunning(const string& animationname) const
{
   if (!m_pB2SData->IsBackglassRunning())
      return false;

   return m_pFormBackglass->IsAnimationRunning(animationname);
}

void Server::MyB2SStartRotation()
{
   if (!m_pB2SData->IsBackglassRunning())
      return;

   m_pFormBackglass->StartRotation();
}

void Server::MyB2SStopRotation()
{
   if (!m_pB2SData->IsBackglassRunning())
      return;

   m_pFormBackglass->StopRotation();
}

void Server::MyB2SShowScoreDisplays()
{
   if (!m_pB2SData->IsBackglassRunning())
      return;

   m_pFormBackglass->ShowScoreDisplays();
}

void Server::MyB2SHideScoreDisplays()
{
   if (!m_pB2SData->IsBackglassRunning())
      return;

   m_pFormBackglass->HideScoreDisplays();
}

void Server::MyB2SShowOrHideScoreDisplays(bool visible)
{
   if (visible) {
      MyB2SShowScoreDisplays();
   } else {
      MyB2SHideScoreDisplays();
   }
}

void Server::MyB2SPlaySound(const string& soundname)
{
   if (!m_pB2SData->IsBackglassRunning())
      return;

   m_pFormBackglass->PlaySound(soundname);
}

void Server::MyB2SStopSound(const string& soundname)
{
   if (!m_pB2SData->IsBackglassRunning())
      return;

   m_pFormBackglass->StopSound(soundname);
}

void Server::Startup()
{
    VPXTableInfo tableInfo;
    m_vpxApi->GetTableInfo(&tableInfo);
    m_pB2SData->SetTableFileName(tableInfo.path);

    LOGI("B2S table filename set to '%s'", tableInfo.path);
}

void Server::ShowBackglassForm()
{
   if (!m_pFormBackglass)
      m_pFormBackglass = new FormBackglass(m_vpxApi, m_msgApi, m_endpointId, m_pB2SData);

   m_pFormBackglass->Show();
   m_pFormBackglass->SetTopMost(true);
   m_pFormBackglass->BringToFront();
   m_pFormBackglass->SetTopMost(false);
   m_pB2SData->SetBackglassVisible(true);
}

void Server::HideBackglassForm()
{
   if (m_pFormBackglass)
      m_pFormBackglass->Hide();
}

void Server::KillBackglassForm()
{
   if (m_pFormBackglass) {
      delete m_pFormBackglass;
      m_pFormBackglass = nullptr;
   }
}

int Server::RandomStarter(int top)
{
   static int lastone = -1;
   if (top <= 1)
      return 0;
   int ret;
   do {
      ret = rand() % top;
   } while (ret == lastone);
   lastone = ret;
   return ret;
}

}
