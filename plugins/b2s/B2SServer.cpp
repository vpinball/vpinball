// license:GPLv3+

#include "common.h"

#include "B2SServer.h"

#include <random>


namespace B2S
{

B2SServer* B2SServer::m_singleton = nullptr;

B2SServer::B2SServer(const MsgPluginAPI* const msgApi, unsigned int endpointId, const VPXPluginAPI* const vpxApi, ScriptClassDef* serverClassDef)
   : m_controllerClassProxy(msgApi, endpointId, "PinMAME_"s, "PinMAME_Controller"s, "B2S_"s, serverClassDef)
   , m_controllerProxy(m_controllerClassProxy)
   , m_msgApi(msgApi)
   , m_endpointId(endpointId)
   , m_vpxApi(vpxApi)
   , m_onGetAuxRendererId(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER))
   , m_onAuxRendererChgId(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG))
   , m_ancillaryRendererDef({ "B2S", "B2S Backglass & FullDMD", "Renderer for directb2s backglass files", this, OnRender })
   , m_onStateChangeEventId(msgApi->GetMsgID("B2S", "OnStateChange"))
   , m_exposedControllers(msgApi, endpointId, CTLPI_CONTROLLERS_GET_MSG, CTLPI_CONTROLLERS_ON_CHG_MSG)
   , m_exposedStates(msgApi, endpointId, CTLPI_STATE_GET_SRC_MSG, CTLPI_STATE_ON_SRC_CHG_MSG)
{
   m_singleton = this;

   VPXTableInfo tableInfo;
   m_vpxApi->GetTableInfo(&tableInfo);

   // Search for an exact match (same file name with .directb2s extension)
   const std::filesystem::path tablePath(tableInfo.path);
   std::filesystem::path b2sFilename = find_case_insensitive_file_path(tablePath.parent_path() / tablePath.filename().replace_extension(".directb2s"));

   // Search for a file matching the template 'foldername.directb2s' for file layout where tables are located in a folder with their companion files (b2s, pup, flex, music, ...)
   if (b2sFilename.empty())
   {
      std::filesystem::path folderName = tablePath.parent_path().filename();
      folderName += ".directb2s"sv;
      b2sFilename = find_case_insensitive_file_path(tablePath.parent_path() / folderName);
   }

   if (!b2sFilename.empty())
   {
      auto loadFile = [](const std::filesystem::path& path)
      {
         std::shared_ptr<B2STable> b2s;
         try
         {
            tinyxml2::XMLDocument b2sTree;
            b2sTree.LoadFile(path.string().c_str());
            if (b2sTree.FirstChildElement("DirectB2SData"))
               b2s = std::make_shared<B2STable>(*b2sTree.FirstChildElement("DirectB2SData"));
         }
         catch (...)
         {
            LOGE("Failed to load B2S file: " + path.string());
         }
         return b2s;
      };
      // B2S file format is heavily unoptimized so perform loading asynchronously (all assets are directly included in the XML file using Base64 encoding)
      m_loadedB2S = std::async(std::launch::async, loadFile, b2sFilename);
   }

   m_msgApi->SubscribeMsg(m_endpointId, m_onGetAuxRendererId, OnGetRenderer, this);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);

   m_b2sName = "b2s::"sv;
   SetB2SName(""s);
}

B2SServer::~B2SServer()
{
   if (m_loadedB2S.valid())
      m_loadedB2S.get();
   m_renderer = nullptr;

   m_gameRunning = false;
   m_exposedControllers.ClearItems();
   m_exposedStates.ClearItems();

   m_msgApi->UnsubscribeMsg(m_onGetAuxRendererId, OnGetRenderer, this);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);
   m_msgApi->ReleaseMsgID(m_onGetAuxRendererId);
   m_msgApi->ReleaseMsgID(m_onAuxRendererChgId);
   m_msgApi->ReleaseMsgID(m_onStateChangeEventId);

   if (m_onDestroyHandler)
      m_onDestroyHandler(this);

   m_singleton = nullptr;
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

string B2SServer::GetB2SName() const { return m_b2sName; }

void B2SServer::SetB2SName(const std::string& b2sName)
{
   if (b2sName == m_b2sName)
      return;
   m_b2sName = b2sName;
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

int B2SServer::OnRender(VPXRenderContext2D* ctx, void* userData)
{
   if ((ctx->window != VPXWindowId::VPXWINDOW_Backglass) && (ctx->window != VPXWindowId::VPXWINDOW_ScoreView))
      return false;

   auto me = static_cast<B2SServer*>(userData);
   if (me->m_renderer)
      return me->m_renderer->Render(ctx, me);

   if (me->m_loadedB2S.valid())
   {
      if (me->m_loadedB2S.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
      {
         me->m_renderer = std::make_unique<B2SRenderer>(me->m_msgApi, me->m_endpointId, me->m_loadedB2S.get());
         me->m_renderer->Render(ctx, me);
      }
      return true; // Until loaded, we assume that the file will succeed loading with the expected backglass/score view
   }

   return false;
}

void B2SServer::OnGetRenderer(const unsigned int, void* userData, void* msgData)
{
   auto me = static_cast<B2SServer*>(userData);
   auto msg = static_cast<GetAncillaryRendererMsg*>(msgData);
   if ((msg->window == VPXWindowId::VPXWINDOW_Backglass) || (msg->window == VPXWindowId::VPXWINDOW_ScoreView))
   {
      if (msg->count < msg->maxEntryCount)
         msg->entries[msg->count] = me->m_ancillaryRendererDef;
      msg->count++;
   }
}

void B2SServer::ForwardCall(void* me, int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet)
{
   m_controllerProxy.ForwardCall(me, memberIndex, pArgs, pRet);

   const char* methodName = m_controllerProxy.GetProxyClass().GetProxyClassDef()->members[memberIndex].name.name;
   if (methodName == "GameName"sv)
   {
      SetB2SName("");
   }
   else if (methodName == "Run"sv)
   {
      if (!m_gameRunning)
      {
         m_gameRunning = true;
         m_exposedControllers.SetItem({ m_endpointId, m_controllerGameId.c_str() });
      }
   }
   else if (methodName == "Stop"sv)
   {
      if (m_gameRunning)
      {
         m_gameRunning = false;
         m_exposedControllers.ClearItems();
      }
   }
}

// Game states

struct B2SPluginEvent
{
   uint8_t type;
   int32_t index;
   int32_t value;
};

void B2SServer::UpdateStateSrc()
{
   m_exposedStates.ClearItems();

   {
      const std::lock_guard lock(m_stateMutex);
      {
         m_lampStateDefs.clear();
         m_lampStateIds.clear();
         m_lampStateIds.reserve(m_lampStates.size());
         for (const auto& [id, _] : m_lampStates)
            m_lampStateIds.push_back(id);
         std::sort(m_lampStateIds.begin(), m_lampStateIds.end());
         m_lampStateNames.resize(m_lampStates.size());
         int index = 0;
         for (const auto id : m_lampStateIds)
         {
            m_lampStateNames[index] = std::format("Illumination #{}", id);
            if (m_defaultStateNameMask & (1ull << id))
            {
               switch (id)
               {
               case 25: m_lampStateNames[index] = "Player #1 Active"sv; break;
               case 26: m_lampStateNames[index] = "Player #2 Active"sv; break;
               case 27: m_lampStateNames[index] = "Player #3 Active"sv; break;
               case 28: m_lampStateNames[index] = "Player #4 Active"sv; break;
               case 30: m_lampStateNames[index] = "Player Up"sv; break;
               case 31: m_lampStateNames[index] = "Can Play"sv; break;
               case 32: m_lampStateNames[index] = "Ball In Play"sv; break;
               case 33: m_lampStateNames[index] = "Tilt"sv; break;
               case 34: m_lampStateNames[index] = "Match"sv; break;
               case 35: m_lampStateNames[index] = "Game Over"sv; break;
               case 36: m_lampStateNames[index] = "Shoot Again"sv; break;
               }
            }
            m_lampStateDefs.emplace_back(
               StateDef { m_lampStateNames[index].c_str(), nullptr, static_cast<uint32_t>(id), CTLPI_STATE_FORMAT_FLOAT, CTLPI_STATE_TYPE_CUSTOM, GetLampState, nullptr });
            index++;
         }
      }

      {
         m_playerScoreStateDefs.clear();
         m_playerScoreIds.clear();
         m_playerScoreIds.reserve(m_playerScores.size());
         for (const auto& [id, _] : m_playerScores)
            m_playerScoreIds.push_back(id);
         std::sort(m_playerScoreIds.begin(), m_playerScoreIds.end());
         m_playerScoreNames.resize(m_playerScores.size());
         int index = 0;
         for (const auto id : m_playerScoreIds)
         {
            m_playerScoreNames[index] = std::format("Player Score #{}", id);
            if (id == 29 && m_defaultStateNameMask & (1ull << id))
               m_playerScoreNames[index] = "Credits"sv;
            m_playerScoreStateDefs.emplace_back(
               StateDef { m_playerScoreNames[index].c_str(), nullptr, static_cast<uint32_t>(id), CTLPI_STATE_FORMAT_INT64, CTLPI_STATE_TYPE_CUSTOM, GetPlayerScore, nullptr });
            index++;
         }
      }

      {
         m_scoreDigitStateDefs.clear();
         m_scoreDigitIds.clear();
         m_scoreDigitIds.reserve(m_scoreDigits.size());
         for (const auto& [id, _] : m_scoreDigits)
            m_scoreDigitIds.push_back(id);
         std::sort(m_scoreDigitIds.begin(), m_scoreDigitIds.end());
         m_scoreDigitNames.resize(m_scoreDigits.size());
         int index = 0;
         for (const auto id : m_scoreDigitIds)
         {
            m_scoreDigitNames[index] = std::format("Digit Score #{}", id);
            m_scoreDigitStateDefs.emplace_back(
               StateDef { m_scoreDigitNames[index].c_str(), nullptr, static_cast<uint32_t>(id), CTLPI_STATE_FORMAT_INT64, CTLPI_STATE_TYPE_CUSTOM, GetScoreDigit, nullptr });
            index++;
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

void MSGPIAPI B2SServer::GetLampState(CtlResId id, unsigned int inputIndex, void* pResult)
{
   assert(m_singleton);
   assert(id.endpointId == m_singleton->m_endpointId && id.resId == 1);
   int srcId;
   {
      const std::lock_guard lock(m_singleton->m_stateMutex);
      if (inputIndex >= m_singleton->m_lampStateIds.size())
         return;
      srcId = m_singleton->m_lampStateIds[inputIndex];
   }
   *static_cast<float*>(pResult) = m_singleton->GetLampState(srcId);
}

void MSGPIAPI B2SServer::GetPlayerScore(CtlResId id, unsigned int inputIndex, void* pResult)
{
   assert(m_singleton);
   assert(id.endpointId == m_singleton->m_endpointId && id.resId == 2);
   int srcId;
   {
      const std::lock_guard lock(m_singleton->m_stateMutex);
      srcId = m_singleton->m_playerScoreIds[inputIndex];
   }
   *static_cast<int64_t*>(pResult) = static_cast<int64_t>(m_singleton->GetPlayerScore(srcId));
}

void MSGPIAPI B2SServer::GetScoreDigit(CtlResId id, unsigned int inputIndex, void* pResult)
{
   assert(m_singleton);
   assert(id.endpointId == m_singleton->m_endpointId && id.resId == 3);
   int srcId;
   {
      const std::lock_guard lock(m_singleton->m_stateMutex);
      srcId = m_singleton->m_scoreDigitIds[inputIndex];
   }
   *static_cast<int64_t*>(pResult) = static_cast<int64_t>(m_singleton->GetScoreDigit(srcId));
}


// B2SSetScore / B2SSetScorePlayer

void B2SServer::B2SSetScore(int digit, int value, bool animate)
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
}

int B2SServer::GetScoreDigit(int digit) const
{
   std::lock_guard lock(m_stateMutex);
   const auto it = m_scoreDigits.find(digit);
   return it == m_scoreDigits.end() ? 0 : it->second;
}

void B2SServer::B2SSetScorePlayer(int playerno, int score)
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
}

int B2SServer::GetPlayerScore(int player) const
{
   std::lock_guard lock(m_stateMutex);
   const auto it = m_playerScores.find(player);
   return it == m_playerScores.end() ? 0 : it->second;
}


// B2SSetData and helpers that are allowed to:
// - change main image illumination
// - change picture box state (illumination, flipbook)
// - change reel illumination
// - start/stop animations

void B2SServer::B2SSetData(int b2sId, int value, bool sendPluginEvent)
{
   LOGD(std::format("B2SSetData {}={}", b2sId, value));

   bool sourceChanged = false;
   {
      const std::lock_guard lock(m_stateMutex);
      const auto [it, inserted] = m_lampStates.try_emplace(b2sId, static_cast<float>(value));
      if (!inserted)
         it->second = static_cast<float>(value);
      sourceChanged = inserted;
   }

   if (sourceChanged)
      UpdateStateSrc();

   if (sendPluginEvent)
   {
      B2SPluginEvent event { 'E', b2sId, value };
      m_msgApi->BroadcastMsg(m_endpointId, m_onStateChangeEventId, &event);
   }
}

void B2SServer::B2SSetData(const std::string& group, int value)
{
   std::lock_guard lock(m_stateMutex);
   // Same as B2SSetData, applied to a group of illumination elements, but does not broadcast a plugin event
   // FIXME implement
   assert(false);
}

float B2SServer::GetLampState(int b2sId) const
{
   std::lock_guard lock(m_stateMutex);
   const auto it = m_lampStates.find(b2sId);
   return it == m_lampStates.end() ? 0.f : it->second;
}

}
