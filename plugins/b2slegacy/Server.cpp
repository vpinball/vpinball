// license:GPLv3+

#include "common.h"
#include "Server.h"
#include "plugins/LoggingPlugin.h"

#include "pinmame/PinMAMEPlugin.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

#include "forms/FormBackglass.h"
#include "forms/FormDMD.h"

#include "classes/B2SVersionInfo.h"
#include "controls/B2SPictureBox.h"
#include "controls/B2SLEDBox.h"
#include "controls/B2SReelBox.h"
#include "classes/B2SReelDisplay.h"
#include "classes/LEDDisplayDigitLocation.h"
#include "classes/CollectData.h"
#include "dream7/Dream7Display.h"
#include "classes/AnimationInfo.h"
#include "classes/PictureBoxAnimation.h"
#include "classes/B2SScreen.h"
#include "utils/PinMAMEAPI.h"

#include <algorithm>
#include <random>


namespace B2SLegacy {

Server::Server(MsgPluginAPI* msgApi, uint32_t endpointId, VPXPluginAPI* vpxApi, ScriptClassDef* serverClassDef)
   : m_msgApi(msgApi)
   , m_vpxApi(vpxApi)
   , m_endpointId(endpointId)
   , m_onGetAuxRendererId(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER))
   , m_onAuxRendererChgId(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG))
   , m_onStateChangedMsgId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_STATE_ON_SRC_CHG_MSG))
   , m_onStateChangeEventId(msgApi->GetMsgID("B2S", "OnStateChange:1"))
   , m_pinmameApi(msgApi, endpointId, this, serverClassDef)
   , m_pinmameControllers(
        msgApi, endpointId, CTLPI_CONTROLLERS_GET_MSG, CTLPI_CONTROLLERS_ON_CHG_MSG,
        [](std::vector<ControllerDef>& items)
        {
           const string pinmamePrefix(PMPI_GAMEID_PREFIX);
           std::erase_if(items, [&pinmamePrefix](const ControllerDef& src) { return !string(src.gameId).starts_with(pinmamePrefix); });
        },
        nullptr, [this]() { m_stateSources.Refresh(); })
   , m_stateSources(
        msgApi, endpointId, CTLPI_STATE_GET_SRC_MSG, CTLPI_STATE_ON_SRC_CHG_MSG,
        [this](std::vector<StateSrcId>& items)
        {
           m_pinmameControllers.With(
              [&items](const std::vector<ControllerDef>& controllers)
              {
                 std::erase_if(items,
                    [&controllers](const StateSrcId& source)
                    {
                       return std::find_if(controllers.begin(), controllers.end(), [source](const ControllerDef& ctrl) { return ctrl.endpointId == source.id.endpointId; })
                          == controllers.end();
                    });
              });
        },
        nullptr, nullptr)
   , m_exposedControllers(msgApi, endpointId, CTLPI_CONTROLLERS_GET_MSG, CTLPI_CONTROLLERS_ON_CHG_MSG)
   , m_exposedStates(msgApi, endpointId, CTLPI_STATE_GET_SRC_MSG, CTLPI_STATE_ON_SRC_CHG_MSG)
{
   m_pB2SSettings = new B2SSettings(m_msgApi, endpointId);
   m_pB2SData = new B2SData(this, m_pB2SSettings, m_vpxApi);
   m_pCollectLampsData = new B2SCollectData(m_pB2SSettings->GetLampsSkipFrames());
   m_pCollectSolenoidsData = new B2SCollectData(m_pB2SSettings->GetSolenoidsSkipFrames());
   m_pCollectGIStringsData = new B2SCollectData(m_pB2SSettings->GetGIStringsSkipFrames());
   m_pCollectLEDsData = new B2SCollectData(m_pB2SSettings->GetLEDsSkipFrames());
   m_pTimer = new Timer();
   m_pTimer->SetInterval(37);
   m_pTimer->SetElapsedListener(std::bind(&Server::TimerElapsed, this, std::placeholders::_1));

   m_msgApi->SubscribeMsg(m_endpointId, m_onGetAuxRendererId, OnGetRendererStatic, this);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);

   m_stateSources.Subscribe();
   m_pinmameControllers.Subscribe();
}

Server::~Server()
{
   m_gameRunning = false;
   m_exposedControllers.ClearItems();
   m_exposedStates.ClearItems();

   m_stateSources.Unsubscribe();
   m_pinmameControllers.Unsubscribe();

   m_msgApi->UnsubscribeMsg(m_onGetAuxRendererId, OnGetRendererStatic, this);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);
   m_msgApi->ReleaseMsgID(m_onGetAuxRendererId);
   m_msgApi->ReleaseMsgID(m_onAuxRendererChgId);
   m_msgApi->ReleaseMsgID(m_onStateChangedMsgId);
   m_msgApi->ReleaseMsgID(m_onStateChangeEventId);

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

   Timer::ServicePendingTimers();

   if (!m_ready) {
      if (!m_pFormBackglass->IsValid())
         return 0;

      m_pFormBackglass->Start();
      m_ready = true;
   }

   if (renderCtx->window == VPXWindowId::VPXWINDOW_Backglass) {
      if (!m_pB2SSettings->IsHideB2SBackglass()) {
         renderCtx->srcWidth = static_cast<float>(m_pFormBackglass->GetWidth());
         renderCtx->srcHeight = static_cast<float>(m_pFormBackglass->GetHeight());
         m_pFormBackglass->OnPaint(renderCtx);
         return 1;
      }
   }
   else if (renderCtx->window == VPXWindowId::VPXWINDOW_ScoreView) {
      if (m_pFormBackglass->GetFormDMD()) {
         renderCtx->srcWidth = static_cast<float>(m_pFormBackglass->GetFormDMD()->GetWidth());
         renderCtx->srcHeight = static_cast<float>(m_pFormBackglass->GetFormDMD()->GetHeight());
         m_pFormBackglass->GetFormDMD()->OnPaint(renderCtx);
         return 1;
      }
   }

   return 0;
}

struct B2SPluginEvent
{
   uint8_t type;
   int32_t index;
   int32_t value;
};

void Server::UpdateStateSrc()
{
   m_exposedStates.ClearItems();

   {
      const std::lock_guard lock(m_stateMutex);
      {
         m_lampStateDefs.clear();
         m_lampStateIds.clear();
         m_lampStateIds.reserve(m_b2sStates.size());
         for (const auto& [id, _] : m_b2sStates)
            m_lampStateIds.push_back({ this, id });
         std::sort(m_lampStateIds.begin(), m_lampStateIds.end(), [](const CallContext& a, const CallContext& b) { return a.id < b.id; });
         m_lampStateNames.resize(m_b2sStates.size());
         for (size_t index = 0; index < m_lampStateIds.size(); ++index)
         {
            const auto id = m_lampStateIds[index].id;
            m_lampStateNames[index] = std::format("Illumination #{}", id);
            m_lampStateDefs.emplace_back(StateDef {
               m_lampStateNames[index].c_str(), nullptr, static_cast<uint32_t>(id), CTLPI_STATE_FORMAT_FLOAT, CTLPI_STATE_TYPE_CUSTOM, &m_lampStateIds[index], GetLampState, nullptr });
            index++;
         }
      }

      {
         m_playerScoreStateDefs.clear();
         m_playerScoreIds.clear();
         m_playerScoreIds.reserve(m_playerScores.size());
         for (const auto& [id, _] : m_playerScores)
            m_playerScoreIds.push_back({ this, id });
         std::sort(m_playerScoreIds.begin(), m_playerScoreIds.end(), [](const CallContext& a, const CallContext& b) { return a.id < b.id; });
         m_playerScoreNames.resize(m_playerScores.size());
         for (size_t index = 0; index < m_playerScoreIds.size(); ++index)
         {
            const auto id = m_playerScoreIds[index].id;
            m_playerScoreNames[index] = std::format("Player Score #{}", id);
            m_playerScoreStateDefs.emplace_back(StateDef {
               m_playerScoreNames[index].c_str(), nullptr, static_cast<uint32_t>(id), CTLPI_STATE_FORMAT_INT64, CTLPI_STATE_TYPE_CUSTOM, &m_playerScoreIds[index], GetPlayerScore, nullptr });
         }
      }

      {
         m_scoreDigitStateDefs.clear();
         m_scoreDigitIds.clear();
         m_scoreDigitIds.reserve(m_scoreDigits.size());
         for (const auto& [id, _] : m_scoreDigits)
            m_scoreDigitIds.push_back({ this, id });
         std::sort(m_scoreDigitIds.begin(), m_scoreDigitIds.end(), [](const CallContext& a, const CallContext& b) { return a.id < b.id; });
         m_scoreDigitNames.resize(m_scoreDigits.size());
         for (size_t index = 0; index < m_scoreDigitIds.size(); ++index)
         {
            const auto id = m_scoreDigitIds[index].id;
            m_scoreDigitNames[index] = std::format("Digit Score #{}", id);
            m_scoreDigitStateDefs.emplace_back(StateDef {
               m_scoreDigitNames[index].c_str(), nullptr, static_cast<uint32_t>(id), CTLPI_STATE_FORMAT_INT64, CTLPI_STATE_TYPE_CUSTOM, &m_scoreDigitIds[index], GetScoreDigit, nullptr });
         }
      }
   }

   m_exposedStates.AddItems({ //
      { .id = { m_endpointId, 1 }, //
         .name = "Illuminations",
         .desc = "Lamp states",
         .nStates = static_cast<unsigned int>(m_lampStateDefs.size()),
         .stateDefs = m_lampStateDefs.data() },
      { .id = { m_endpointId, 2 },
         .name = "Scores (players)",
         .desc = "Player score",
         .nStates = static_cast<unsigned int>(m_playerScoreStateDefs.size()),
         .stateDefs = m_playerScoreStateDefs.data() },
      { .id = { m_endpointId, 3 },
         .name = "Scores (digits)",
         .desc = "Individual digit (reel) scores",
         .nStates = static_cast<unsigned int>(m_scoreDigitStateDefs.size()),
         .stateDefs = m_scoreDigitStateDefs.data() } });
}

void MSGPIAPI Server::GetLampState(void* callContext, void* pResult)
{
   auto ctx = static_cast<CallContext*>(callContext);
   *static_cast<float*>(pResult) = ctx->me->GetState(ctx->id);
}

void MSGPIAPI Server::GetPlayerScore(void* callContext, void* pResult)
{
   auto ctx = static_cast<CallContext*>(callContext);
   *static_cast<int64_t*>(pResult) = static_cast<int64_t>(ctx->me->GetPlayerScore(ctx->id));
}

void MSGPIAPI Server::GetScoreDigit(void* callContext, void* pResult)
{
   auto ctx = static_cast<CallContext*>(callContext);
   *static_cast<int64_t*>(pResult) = static_cast<int64_t>(ctx->me->GetScoreDigit(ctx->id));
}

float Server::GetState(int b2sId) const
{
   const std::lock_guard lock(m_stateMutex);
   const auto it = m_b2sStates.find(b2sId);
   return it == m_b2sStates.end() ? 0.f : it->second;
}

int Server::GetPlayerScore(int playerno) const
{
   const std::lock_guard lock(m_stateMutex);
   const auto it = m_playerScores.find(playerno);
   return it == m_playerScores.end() ? 0 : it->second;
}

int Server::GetScoreDigit(int digit) const
{
   const std::lock_guard lock(m_stateMutex);
   const auto it = m_scoreDigits.find(digit);
   return it == m_scoreDigits.end() ? 0 : it->second;
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
            LOGI(std::format("B2S polling status: lamps={}, solenoids={}, giStrings={}, leds={}", callLamps, callSolenoids, callGIStrings, callLEDs));

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

static std::string CreateGuidString()
{
   std::random_device rd;
   std::mt19937_64 gen(rd());
   std::uniform_int_distribution<uint64_t> dist;

   uint64_t hi = dist(gen);
   uint64_t lo = dist(gen);

   // Set UUID version (4) and variant bits per RFC 4122
   hi = (hi & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
   lo = (lo & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

   char buf[37];
   std::snprintf(buf, sizeof(buf),
      "%08llx-%04llx-%04llx-%04llx-%012llx",
      (hi >> 32) & 0xFFFFFFFFULL,
      (hi >> 16) & 0xFFFFULL,
      hi & 0xFFFFULL,
      (lo >> 48) & 0xFFFFULL,
      lo & 0xFFFFFFFFFFFFULL);

   return std::string(buf);
}

const string& Server::GetB2SName() const
{
   return m_pB2SSettings->GetB2SName();
}

void Server::SetB2SName(const string& b2sName)
{
   if (b2sName == m_pB2SSettings->GetB2SName())
      return;

   m_pB2SSettings->SetB2SName(b2sName);

   string id = trim_string(b2sName);
   if (id.empty())
      m_controllerGameId = "b2s::" + CreateGuidString();
   else
      m_controllerGameId = "b2s::" + string_to_lower(id);

   if (m_gameRunning)
      m_exposedControllers.SetItem({ m_endpointId, m_controllerGameId.c_str() });
   else
      m_exposedControllers.ClearItems();
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

   if (!m_gameRunning)
   {
      m_gameRunning = true;
      m_exposedControllers.SetItem({ m_endpointId, m_controllerGameId.c_str() });
   }
}

void Server::Stop()
{
   m_pTimer->Stop();
   HideBackglassForm();

   m_pB2SData->Stop();
   KillBackglassForm();

   if (m_gameRunning)
   {
      m_gameRunning = false;
      m_exposedControllers.ClearItems();
   }
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
   ScriptArray* lampArray = m_pinmameApi.GetChangedLamps();
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
   ScriptArray* solenoidArray = m_pinmameApi.GetChangedSolenoids();
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
   ScriptArray* giStringArray = m_pinmameApi.GetChangedGIStrings();
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
   ScriptArray* ledArray = m_pinmameApi.GetChangedLEDs();
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
   m_pinmameApi.SetSwitch(switchId, value);
}

void Server::B2SSetData(int id, int value)
{
   MyB2SSetData(id, value);
}

void Server::B2SSetData(const string& name, int value)
{
   MyB2SSetData(name, value);
}

void Server::B2SSetData(int id, const string& value)
{
   int result;
   if (is_string_numeric(value, &result))
      MyB2SSetData(id, result);
}

void Server::B2SSetData(const string& name, const string& value)
{
   int result;
   if (is_string_numeric(value, &result))
      MyB2SSetData(name, result);
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

void Server::B2SSetPos(int id, int xpos, const string& ypos)
{
   int result;
   if (is_string_numeric(ypos, &result))
      MyB2SSetPos(id, xpos, result);
}

void Server::B2SSetPos(int id, const string& xpos, int ypos)
{
   int result;
   if (is_string_numeric(xpos, &result))
      MyB2SSetPos(id, result, ypos);
}

void Server::B2SSetPos(int id, const string& xpos, const string& ypos)
{
   int resultx, resulty;
   if (is_string_numeric(xpos, &resultx) && is_string_numeric(ypos, &resulty))
      MyB2SSetPos(id, resultx, resulty);
}

void Server::B2SSetPos(const string& id, int xpos, int ypos)
{
   int result;
   if (is_string_numeric(id, &result))
      MyB2SSetPos(result, xpos, ypos);
}

void Server::B2SSetPos(const string& id, int xpos, const string& ypos)
{
   int resultid, resulty;
   if (is_string_numeric(id, & resultid) && is_string_numeric(ypos, &resulty))
      MyB2SSetPos(resultid, xpos, resulty);
}

void Server::B2SSetPos(const string& id, const string& xpos, int ypos)
{
   int resultid, resultx;
   if (is_string_numeric(id, &resultid) && is_string_numeric(xpos, &resultx))
      MyB2SSetPos(resultid, resultx, ypos);
}

void Server::B2SSetPos(const string& id, const string& xpos, const string& ypos)
{
   int resultid, resultx, resulty;
   if (is_string_numeric(id, &resultid) && is_string_numeric(xpos, &resultx) && is_string_numeric(ypos, &resulty))
      MyB2SSetPos(resultid, resultx, resulty);
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

// Reel method(s)
void Server::B2SSetReel(int digit, int value)
{
   MyB2SSetScore(digit, value, true);
}

// Score: 1-24
void Server::B2SSetScore(int display, int value)
{
   MyB2SSetScore(GetFirstDigitOfDisplay(display), value);
}

void Server::B2SSetScorePlayer(int playerno, int score)
{
   MyB2SSetScorePlayer(playerno, score);
}

void Server::B2SSetScorePlayer1(int score)
{
   MyB2SSetScorePlayer(1, score);
}

void Server::B2SSetScorePlayer2(int score)
{
   MyB2SSetScorePlayer(2, score);
}

void Server::B2SSetScorePlayer3(int score)
{
   MyB2SSetScorePlayer(3, score);
}

void Server::B2SSetScorePlayer4(int score)
{
   MyB2SSetScorePlayer(4, score);
}

void Server::B2SSetScorePlayer5(int score)
{
   MyB2SSetScorePlayer(5, score);
}

void Server::B2SSetScorePlayer6(int score)
{
   MyB2SSetScorePlayer(6, score);
}

void Server::B2SSetScoreDigit(int digit, int value)
{
   MyB2SSetScore(digit, value, false);
}

// Score rollover: 25-28
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

// Credits: 29
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
   bool sourceChanged = false;
   {
      const std::lock_guard lock(m_stateMutex);
      const auto [it, inserted] = m_b2sStates.try_emplace(id, static_cast<float>(value));
      if (!inserted)
         it->second = static_cast<float>(value);
      sourceChanged = inserted;
   }

   if (sourceChanged)
      UpdateStateSrc();

   B2SPluginEvent event { 'E', id, value };
   m_msgApi->BroadcastMsg(m_endpointId, m_onStateChangeEventId, &event);

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
   m_stateSources.With(
      [this](const std::vector<StateSrcId>& stateSources)
      {
      const auto it = std::find_if(stateSources.begin(), stateSources.end(), [](const StateSrcId& src) { return src.id.resId == PMPI_GROUP_VPM_LAMP; });
      if (it == stateSources.end())
         return;
      const StateSrcId& pinmameStateSrc = *it;

      for (unsigned int i = 0; i < pinmameStateSrc.nStates; i++)
      {
         const StateDef& def = pinmameStateSrc.stateDefs[i];
         if (def.dataFormat != CTLPI_STATE_FORMAT_UINT8 || def.GetState == nullptr)
            continue;
         uint8_t state;
         def.GetState(def.callContext, &state);
         const int lampState = static_cast<int>(state);
         const int lampId = def.mappingId;

         if (m_pB2SData->IsUseRomLamps() || m_pB2SData->IsUseAnimationLamps())
         {
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
   });

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
               B2SPictureBox* const pPicbox = dynamic_cast<B2SPictureBox*>(pBase);
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
   m_stateSources.With(
      [this](const std::vector<StateSrcId>& stateSources)
      {
         const auto it = std::find_if(stateSources.begin(), stateSources.end(), [](const StateSrcId& src) { return src.id.resId == PMPI_GROUP_VPM_SOLENOID; });
         if (it == stateSources.end())
            return;
         const StateSrcId& pinmameStateSrc = *it;

         for (unsigned int i = 0; i < pinmameStateSrc.nStates; i++)
         {
            const StateDef& def = pinmameStateSrc.stateDefs[i];
            if (def.dataFormat != CTLPI_STATE_FORMAT_UINT8 || def.GetState == nullptr)
               continue;
            uint8_t state;
            def.GetState(def.callContext, &state);
            const int solenoidState = static_cast<int>(state);
            const int solenoidId = def.mappingId;

            if (m_pB2SData->IsUseRomSolenoids() || m_pB2SData->IsUseAnimationSolenoids())
            {
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
      });

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
               B2SPictureBox* const pPicbox = dynamic_cast<B2SPictureBox*>(pBase);
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
   m_stateSources.With(
      [this](const std::vector<StateSrcId>& stateSources)
      {
         const auto it = std::find_if(stateSources.begin(), stateSources.end(), [](const StateSrcId& src) { return src.id.resId == PMPI_GROUP_VPM_GI; });
         if (it == stateSources.end())
            return;
         const StateSrcId& pinmameStateSrc = *it;

         for (unsigned int i = 0; i < pinmameStateSrc.nStates; i++)
         {
            const StateDef& def = pinmameStateSrc.stateDefs[i];
            if (def.dataFormat != CTLPI_STATE_FORMAT_UINT8 || def.GetState == nullptr)
               continue;
            uint8_t state;
            def.GetState(def.callContext, &state);
            const int giStringBool = state > m_giStringThreshold;
            const int giStringId = def.mappingId;

            if (m_pB2SData->IsUseRomGIStrings() || m_pB2SData->IsUseAnimationGIStrings())
            {
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
      });

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
               B2SPictureBox* const pPicbox = dynamic_cast<B2SPictureBox*>(pBase);
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

   int uCount = psa->lengths[0];
   const int32_t* const data = reinterpret_cast<int32_t*>(&psa->lengths[2]);
   for (int i = 0; i < uCount; i++) {
      int digit = data[i * 3 + 0];
      int value = data[i * 3 + 2];
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
         int digit = key;
         int value = pCollectData->GetState();

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

   const auto& led = m_pB2SData->GetLEDs()->find("LEDBox" + std::to_string(digit));
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

   const bool useLEDs = m_pB2SData->GetLEDs()->contains("LEDBox" + std::to_string(digit)) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered;
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

   const int digit = GetFirstDigitOfDisplay(display);

   const bool useLEDs = m_pB2SData->GetLEDs()->contains("LEDBox" + std::to_string(digit)) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered;
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
   bool sourceChanged = false;
   {
      const std::lock_guard lock(m_stateMutex);
      const auto [it, inserted] = m_scoreDigits.try_emplace(digit, value);
      if (!inserted)
         it->second = value;
      sourceChanged = inserted;
   }

   if (sourceChanged)
      UpdateStateSrc();

   B2SPluginEvent event { 'B', digit, value };
   m_msgApi->BroadcastMsg(m_endpointId, m_onStateChangeEventId, &event);

   if (m_pB2SData->IsBackglassRunning()) {
      if (digit > 0) {
         const bool useLEDs = (m_pB2SData->GetLEDs()->contains("LEDBox" + std::to_string(digit)) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered);
         const bool useLEDDisplays = (m_pB2SData->GetLEDDisplayDigits()->contains(digit - 1) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Dream7);
         const bool useReels = m_pB2SData->GetReels()->contains("ReelBox" + std::to_string(digit));

         if (useLEDs) {
            // Rendered LEDs are used
            const string ledname = "LEDBox" + std::to_string(digit);
            (*m_pB2SData->GetLEDs())[ledname]->SetText(std::to_string(value));
         }
         else if (useLEDDisplays) {
            // Dream 7 displays are used
            LEDDisplayDigitLocation* pLEDDisplayDigit = (*m_pB2SData->GetLEDDisplayDigits())[digit - 1];
            pLEDDisplayDigit->GetLEDDisplay()->SetValue(pLEDDisplayDigit->GetDigit(), std::to_string(value));
         }
         else if (useReels) {
            // Reels are used
            const string reelname = "ReelBox" + std::to_string(digit);
            (*m_pB2SData->GetReels())[reelname]->SetText(value, animateReelChange);
         }
      }
   }
}

void Server::MyB2SSetScore(int digit, int score)
{
   if (m_pB2SData->IsBackglassRunning()) {
      if (digit > 0) {
         const bool useLEDs = (m_pB2SData->GetLEDs()->contains("LEDBox" + std::to_string(digit)) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Rendered);
         const bool useLEDDisplays = (m_pB2SData->GetLEDDisplayDigits()->contains(digit - 1) && m_pB2SSettings->GetUsedLEDType() == eLEDTypes_Dream7);
         const bool useReels = m_pB2SData->GetReels()->contains("ReelBox" + std::to_string(digit));

         if (useLEDs) {
            // Check the passed digit
            const string led = "LEDBox" + std::to_string(digit);

            // Get all necessary display data
            const int startdigit = (*m_pB2SData->GetLEDs())[led]->GetStartDigit();
            const int digits = (*m_pB2SData->GetLEDs())[led]->GetDigits();
            const string scoreAsString = string(digits - std::to_string(score).length(), ' ') + std::to_string(score);

            // Set digits
            for (int i = startdigit + digits - 1; i >= startdigit; i--)
               (*m_pB2SData->GetLEDs())["LEDBox" + std::to_string(i)]->SetText(string(1,scoreAsString[i - startdigit]));
         }
         else if (useLEDDisplays) {
            LEDDisplayDigitLocation* pLEDDisplayDigit = (*m_pB2SData->GetLEDDisplayDigits())[digit - 1];

            // Get all necessary display data
            const int digits = pLEDDisplayDigit->GetLEDDisplay()->GetDigits();
            const string scoreAsString = string(digits - std::to_string(score).length(), ' ') + std::to_string(score);

            // Set digits
            for (int i = digits - 1; i >= 0; i--)
               pLEDDisplayDigit->GetLEDDisplay()->SetValue(i, string(1,scoreAsString[i]));
         }
         else if (useReels) {
            // Get the necessary infos
            const string reel = "ReelBox" + std::to_string(digit);
            const int id = (*m_pB2SData->GetReels())[reel]->GetDisplayID();

            // Set value
            if (m_pB2SData->GetReelDisplays()->contains(id))
               (*m_pB2SData->GetReelDisplays())[id]->SetScore(score);
         }
      }
   }
}

void Server::MyB2SSetScorePlayer(int playerno, int score)
{
   bool sourceChanged = false;
   {
      const std::lock_guard lock(m_stateMutex);
      const auto [it, inserted] = m_playerScores.try_emplace(playerno, score);
      if (!inserted)
         it->second = score;
      sourceChanged = inserted;
   }

   if (sourceChanged)
      UpdateStateSrc();

   B2SPluginEvent event { 'C', playerno, score };
   m_msgApi->BroadcastMsg(m_endpointId, m_onStateChangeEventId, &event);

   if (m_pB2SData->IsBackglassRunning()) {
      if (playerno > 0) {
         // Set score to player class
         if (m_pB2SData->GetPlayers()->contains(playerno))
            (*m_pB2SData->GetPlayers())[playerno]->SetScore(m_pB2SData, score);
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

    LOGI("B2S table filename set to '"s + tableInfo.path + '\'');
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
   // Clear the running flag before destroying the form, like the B2S server does. The data
   // setters gate on IsBackglassRunning(), so a script that keeps pushing B2S data after the
   // backglass is killed then stops touching the freed form instead of dereferencing it.
   m_pB2SData->SetBackglassVisible(false);
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
