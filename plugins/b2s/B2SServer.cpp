// license:GPLv3+

#include "common.h"

#include "B2SServer.h"

#include <random>


namespace B2S
{

B2SServer* B2SServer::m_singleton = nullptr;

B2SServer::B2SServer(const MsgPluginAPI* const msgApi, unsigned int endpointId, const VPXPluginAPI* const vpxApi, ScriptClassDef* serverClassDef)
   : m_controllerClassProxy(msgApi, endpointId, "PinMAME_", "PinMAME_Controller", "B2S_", serverClassDef)
   , m_controllerProxy(m_controllerClassProxy)
   , m_msgApi(msgApi)
   , m_endpointId(endpointId)
   , m_vpxApi(vpxApi)
   , m_onGetAuxRendererId(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER))
   , m_onAuxRendererChgId(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG))
   , m_ancillaryRendererDef({ "B2S", "B2S Backglass & FullDMD", "Renderer for directb2s backglass files", this, OnRender })
   , m_onControllersChangedId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_CONTROLLERS_ON_CHG_MSG))
   , m_getControllersId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_CONTROLLERS_GET_MSG))
   , m_onGetStateSrcId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_STATE_GET_SRC_MSG))
   , m_onStateSrcChgId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_STATE_ON_SRC_CHG_MSG))
   , m_onStateChangeEventId(msgApi->GetMsgID("B2S", "OnStateChange"))
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

   m_msgApi->SubscribeMsg(m_endpointId, m_getControllersId, OnGetControllers, this);

   m_msgApi->SubscribeMsg(m_endpointId, m_onGetAuxRendererId, OnGetRenderer, this);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);

   m_stateSrc.id.endpointId = m_endpointId;
   m_stateSrc.nGroups = 0;
   m_stateSrc.groupDefs = nullptr;
   m_stateSrc.GetState = GetState;
   m_stateSrc.SetState = SetState;
   m_msgApi->SubscribeMsg(m_endpointId, m_onGetStateSrcId, OnGetStateSrc, this);
   UpdateStateSrc();
   
   m_b2sName = "b2s::";
   SetB2SName("");
}

B2SServer::~B2SServer()
{
   if (m_loadedB2S.valid())
      m_loadedB2S.get();
   m_renderer = nullptr;

   if (m_gameRunning)
   {
      m_gameRunning = false;
      m_msgApi->BroadcastMsg(m_endpointId, m_onControllersChangedId, nullptr);
   }
   m_msgApi->ReleaseMsgID(m_onControllersChangedId);
   m_msgApi->ReleaseMsgID(m_getControllersId);
   m_msgApi->UnsubscribeMsg(m_getControllersId, OnGetControllers, this);

   if (m_lampStates.size() > 0)
   {
      m_lampStates.clear();
      UpdateStateSrc();
   }

   m_msgApi->UnsubscribeMsg(m_onGetAuxRendererId, OnGetRenderer, this);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);
   m_msgApi->ReleaseMsgID(m_onGetAuxRendererId);
   m_msgApi->ReleaseMsgID(m_onAuxRendererChgId);

   m_msgApi->UnsubscribeMsg(m_onGetStateSrcId, OnGetStateSrc, this);
   m_msgApi->ReleaseMsgID(m_onGetStateSrcId);
   m_msgApi->ReleaseMsgID(m_onStateSrcChgId);

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
      m_msgApi->BroadcastMsg(m_endpointId, m_onControllersChangedId, nullptr);
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
         m_msgApi->BroadcastMsg(m_endpointId, m_onControllersChangedId, nullptr);
      }
   }
   else if (methodName == "Stop"sv)
   {
      if (m_gameRunning)
      {
         m_gameRunning = false;
         m_msgApi->BroadcastMsg(m_endpointId, m_onControllersChangedId, nullptr);
      }
   }
}

// Controller

void B2SServer::OnGetControllers(const unsigned int, void* userData, void* msgData)
{
   if (auto me = static_cast<B2SServer*>(userData); me->m_gameRunning)
   {
      auto msg = static_cast<GetControllersMsg*>(msgData);
      if (msg->count < msg->maxEntryCount)
      {
         msg->entries[msg->count].endpointId = me->m_endpointId;
         msg->entries[msg->count].gameId = me->m_controllerGameId.c_str();
      }
      msg->count++;
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
   // Discard our state block while we rebuild to prevent invalid concurrent access by  a client of the previous state block
   {
      const std::lock_guard<std::mutex> lock(m_stateSrcMutex);
      m_stateSrc.nStates = 0;
   }
   m_msgApi->BroadcastMsg(m_endpointId, m_onStateSrcChgId, nullptr);

   // Setup new state block
   uint16_t index = 0;
   std::unique_lock<std::mutex> lock(m_stateSrcMutex);
   delete[] m_stateSrc.stateDefs;
   m_stateSrc = { };
   m_stateSrc.nStates = static_cast<unsigned int>(m_lampStates.size() + m_playerScores.size() + m_scoreDigits.size());
   if (m_stateSrc.nStates == 0)
      return;
   m_stateSrc.id = { m_endpointId, 0 };
   m_stateSrc.nGroups = static_cast<unsigned int>(m_stateGroupDefs.size());
   m_stateSrc.groupDefs = m_stateGroupDefs.data();
   m_stateSrc.stateDefs = new StateDef[m_stateSrc.nStates]();
   m_stateSrc.GetState = &GetState;
   m_stateSrc.SetState = &SetState;
   m_stateSrcNames.resize(m_stateSrc.nStates);
   for (const auto& [id, v] : m_lampStates)
   {
      m_stateSrcNames[index] = std::format("Illumination #{}", id);
      if (m_defaultStateNameMask & (1ull << id))
      {
         switch (id)
         {
         case 25: m_stateSrcNames[index] = "Player #1 Active"; break;
         case 26: m_stateSrcNames[index] = "Player #2 Active"; break;
         case 27: m_stateSrcNames[index] = "Player #3 Active"; break;
         case 28: m_stateSrcNames[index] = "Player #4 Active"; break;
         case 30: m_stateSrcNames[index] = "Player Up"; break;
         case 31: m_stateSrcNames[index] = "Can Play"; break;
         case 32: m_stateSrcNames[index] = "Ball In Play"; break;
         case 33: m_stateSrcNames[index] = "Tilt"; break;
         case 34: m_stateSrcNames[index] = "Match"; break;
         case 35: m_stateSrcNames[index] = "Game Over"; break;
         case 36: m_stateSrcNames[index] = "Shoot Again"; break;
         }
      }
      m_stateSrc.stateDefs[index].name = m_stateSrcNames[index].c_str();
      m_stateSrc.stateDefs[index].id.groupId = 0x0001;
      m_stateSrc.stateDefs[index].id.stateId = id;
      m_stateSrc.stateDefs[index].typeMask = CTLPI_STATE_TYPE_FLOAT | CTLPI_STATE_TYPE_UINT8;
      index++;
   }
   for (const auto& [id, v] : m_playerScores)
   {
      m_stateSrcNames[index] = std::format("Player Score #{}", id);
      if (id == 29 && m_defaultStateNameMask & (1ull << id))
         m_stateSrcNames[index] = "Credits";
      m_stateSrc.stateDefs[index].name = m_stateSrcNames[index].c_str();
      m_stateSrc.stateDefs[index].id.groupId = 0x0002;
      m_stateSrc.stateDefs[index].id.stateId = id;
      m_stateSrc.stateDefs[index].typeMask = CTLPI_STATE_TYPE_INT32 | CTLPI_STATE_TYPE_INT64;
      index++;
   }
   for (const auto& [id, v] : m_scoreDigits)
   {
      m_stateSrcNames[index] = std::format("Digit Score #{}", id);
      m_stateSrc.stateDefs[index].name = m_stateSrcNames[index].c_str();
      m_stateSrc.stateDefs[index].id.groupId = 0x0003;
      m_stateSrc.stateDefs[index].id.stateId = id;
      m_stateSrc.stateDefs[index].typeMask = CTLPI_STATE_TYPE_INT32 | CTLPI_STATE_TYPE_INT64;
      index++;
   }
   lock.unlock();

   m_msgApi->BroadcastMsg(m_endpointId, m_onStateSrcChgId, nullptr);
}

void B2SServer::OnGetStateSrc(const unsigned int, void* userData, void* msgData)
{
   if (auto me = static_cast<B2SServer*>(userData); me->m_stateSrc.nStates)
   {
      auto msg = static_cast<GetStateSrcMsg*>(msgData);
      if (msg->count < msg->maxEntryCount)
         memcpy(&msg->entries[msg->count], &me->m_stateSrc, sizeof(StateSrcId));
      msg->count++;
   }
}

int MSGPIAPI B2SServer::GetState(unsigned int inputIndex, int type, void* pResult)
{
   if (B2SServer::m_singleton == nullptr)
      return -1;
   int id;
   uint32_t groupId;
   {
      const std::lock_guard<std::mutex> lock(m_singleton->m_stateSrcMutex);
      if (inputIndex >= m_singleton->m_stateSrc.nStates)
         return -1;
      id = m_singleton->m_stateSrc.stateDefs[inputIndex].id.stateId;
      groupId = m_singleton->m_stateSrc.stateDefs[inputIndex].id.groupId;
   }
   switch (groupId)
   {
   case 0x0001:
   {
      // Normalized lamps 0..1 or 0..255
      float val = m_singleton->GetLampState(id);
      switch (type)
      {
      case CTLPI_STATE_TYPE_UINT8: *static_cast<uint8_t*>(pResult) = static_cast<uint8_t>(val * 255.f); return 0;
      case CTLPI_STATE_TYPE_FLOAT: *static_cast<float*>(pResult) = val; return 0;
      }
      break;
   }
   case 0x0002:
   case 0x0003:
   {
      // Scores, credits and other generic states
      int val = groupId == 0x0002 ? m_singleton->GetPlayerScore(id) : m_singleton->GetScoreDigit(id);
      switch (type)
      {
      case CTLPI_STATE_TYPE_INT32: *static_cast<int32_t*>(pResult) = static_cast<int32_t>(val); return 0;
      case CTLPI_STATE_TYPE_INT64: *static_cast<int64_t*>(pResult) = static_cast<int64_t>(val); return 0;
      }
   }
   }
   return -1;
}

int MSGPIAPI B2SServer::SetState(unsigned int inputIndex, int type, void* pResult) { return -1; }


// B2SSetScore / B2SSetScorePlayer

void B2SServer::B2SSetScore(int digit, int value, bool animate)
{
   bool isNewState;
   {
      const std::lock_guard<std::mutex> lock(m_stateSrcMutex);
      const auto it = m_scoreDigits.find(digit);
      isNewState = it == m_scoreDigits.end();
      if (isNewState)
         m_scoreDigits[digit] = value;
      else
         it->second = value;
   }
   if (isNewState)
      UpdateStateSrc();
   B2SPluginEvent event { 'B', digit, value };
   m_msgApi->BroadcastMsg(m_endpointId, m_onStateChangeEventId, &event);
}

int B2SServer::GetScoreDigit(int digit) const
{
   const std::lock_guard<std::mutex> lock(m_stateSrcMutex);
   const auto it = m_scoreDigits.find(digit);
   return it == m_scoreDigits.end() ? 0 : it->second;
}

void B2SServer::B2SSetScorePlayer(int playerno, int score)
{
   bool isNewState;
   {
      const std::lock_guard<std::mutex> lock(m_stateSrcMutex);
      const auto it = m_playerScores.find(playerno);
      isNewState = it == m_playerScores.end();
      if (isNewState)
         m_playerScores[playerno] = score;
      else
         it->second = score;
   }
   if (isNewState)
      UpdateStateSrc();
   B2SPluginEvent event { 'C', playerno, score };
   m_msgApi->BroadcastMsg(m_endpointId, m_onStateChangeEventId, &event);
}

int B2SServer::GetPlayerScore(int player) const
{
   const std::lock_guard<std::mutex> lock(m_stateSrcMutex);
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
   bool isNewState;
   {
      const std::lock_guard<std::mutex> lock(m_stateSrcMutex);
      const auto it = m_lampStates.find(b2sId);
      isNewState = it == m_lampStates.end();
      if (isNewState)
         m_lampStates[b2sId] = static_cast<float>(value);
      else
         it->second = static_cast<float>(value);
   }
   if (isNewState)
      UpdateStateSrc();

   if (sendPluginEvent)
   {
      B2SPluginEvent event { 'E', b2sId, value };
      m_msgApi->BroadcastMsg(m_endpointId, m_onStateChangeEventId, &event);
   }
}

void B2SServer::B2SSetData(const std::string& group, int value)
{
   // Same as B2SSetData, applied to a group of illumination elements, but does not broadcast a plugin event
   // FIXME implement
   assert(false);
}

float B2SServer::GetLampState(int b2sId) const
{
   const std::lock_guard<std::mutex> lock(m_stateSrcMutex);
   const auto it = m_lampStates.find(b2sId);
   return it == m_lampStates.end() ? 0.f : it->second;
}

}
