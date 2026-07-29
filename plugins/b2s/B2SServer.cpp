// license:GPLv3+

#include "common.h"

#include "B2SServer.h"

namespace B2S
{

extern const char* B2SGetGlobalPath();

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
   , m_onGameStartId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_GAME_START))
   , m_onGameEndId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_GAME_END))
   , m_onGetStateSrcId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_STATE_GET_SRC_MSG))
   , m_onStateSrcChgId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_STATE_ON_SRC_CHG_MSG))
   , m_onStateChangeEventId(msgApi->GetMsgID("B2S", "OnStateChange"))
{
   m_singleton = this;

   VPXTableInfo tableInfo;
   m_vpxApi->GetTableInfo(&tableInfo);

   // Search for an exact match (same file name with .directb2s extension)
   const std::filesystem::path tablePath(tableInfo.path);
   LOGI("B2S: Searching for directb2s for table: " + tablePath.string());
   LOGI("B2S: table parent: " + tablePath.parent_path().string() + ", filename: " + tablePath.filename().string());
   std::filesystem::path b2sFilename = find_case_insensitive_file_path(tablePath.parent_path() / tablePath.filename().replace_extension(".directb2s"));
   LOGI("B2S: Local search result: " + (b2sFilename.empty() ? "EMPTY" : b2sFilename.string()));

   // Search for a file matching the template 'foldername.directb2s' for file layout where tables are located in a folder with their companion files (b2s, pup, flex, music, ...)
   if (b2sFilename.empty())
   {
      std::filesystem::path folderName = tablePath.parent_path().filename();
      folderName += ".directb2s"sv;
      b2sFilename = find_case_insensitive_file_path(tablePath.parent_path() / folderName);
      LOGI("B2S: Folder-name search result: " + (b2sFilename.empty() ? "EMPTY" : b2sFilename.string()));
   }

   // Fallback: search in the global B2S path setting (for Android SAF workaround)
   if (b2sFilename.empty())
   {
       const std::string b2sBasePath = B2SGetGlobalPath();
       LOGI("B2S: B2SPath fallback value: '" + b2sBasePath + "'");
      if (!b2sBasePath.empty())
      {
         const std::filesystem::path b2sPath(b2sBasePath);
         const std::filesystem::path b2sFile = tablePath.filename().replace_extension(".directb2s");
         const std::filesystem::path searchPath = b2sPath / b2sFile;
         LOGI("B2S: Searching B2SPath: " + searchPath.string());
         b2sFilename = find_case_insensitive_file_path(searchPath);
         LOGI("B2S: B2SPath search result: " + (b2sFilename.empty() ? "EMPTY" : b2sFilename.string()));
      }
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

   m_stateSrc.id.endpointId = m_endpointId;
   m_stateSrc.nGroups = 0;
   m_stateSrc.groupDefs = nullptr;
   m_stateSrc.GetState = GetState;
   m_stateSrc.SetState = SetState;
   m_msgApi->SubscribeMsg(m_endpointId, m_onGetStateSrcId, OnGetStateSrc, this);
   UpdateStateSrc();
}

B2SServer::~B2SServer()
{
   if (m_loadedB2S.valid())
      m_loadedB2S.get();
   m_renderer = nullptr;

   if (m_gameRunning)
   {
      CtlOnGameStateChgMsg msg = { m_endpointId, m_b2sName.c_str() };
      m_msgApi->BroadcastMsg(m_endpointId, m_onGameEndId, reinterpret_cast<void*>(&msg));
   }
   m_msgApi->ReleaseMsgID(m_onGameStartId);
   m_msgApi->ReleaseMsgID(m_onGameEndId);

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

string B2SServer::GetB2SName() const { return m_b2sName; }

void B2SServer::SetB2SName(const std::string& b2sName)
{
   if (b2sName == m_b2sName)
      return;
   if (m_gameRunning)
   {
      CtlOnGameStateChgMsg msg = { m_endpointId, m_b2sName.c_str() };
      m_msgApi->BroadcastMsg(m_endpointId, m_onGameEndId, reinterpret_cast<void*>(&msg));
   }
   m_b2sName = b2sName;
   if (m_gameRunning)
   {
      CtlOnGameStateChgMsg msg = { m_endpointId, m_b2sName.c_str() };
      m_msgApi->BroadcastMsg(m_endpointId, m_onGameStartId, reinterpret_cast<void*>(&msg));
   }
}

int MSGPIAPI B2SServer::SetState(unsigned int inputIndex, int type, void* pResult) { return -1; }

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
         CtlOnGameStateChgMsg msg = { m_endpointId, m_b2sName.c_str() };
         m_msgApi->BroadcastMsg(m_endpointId, m_onGameStartId, reinterpret_cast<void*>(&msg));
      }
   }
   else if (methodName == "Stop"sv)
   {
      if (m_gameRunning)
      {
         m_gameRunning = false;
         CtlOnGameStateChgMsg msg = { m_endpointId, m_b2sName.c_str() };
         m_msgApi->BroadcastMsg(m_endpointId, m_onGameEndId, reinterpret_cast<void*>(&msg));
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
   // Discard our state block while we rebuild to prevent invalid concurrent access by  a client of the previous state block
   m_stateSrc.nStates = 0;
   m_msgApi->BroadcastMsg(m_endpointId, m_onStateSrcChgId, nullptr);

   // Setup new state block
   uint16_t index = 0;
   delete[] m_stateSrc.stateDefs;
   m_stateSrc = { };
   m_stateSrc.nStates = static_cast<unsigned int>(m_lampStates.size() + m_playerScores.size() + m_scoreDigits.size());
   if (m_stateSrc.nStates == 0)
      return;
   m_stateSrc.id = { m_endpointId, 0 };
   m_stateSrc.nGroups = static_cast<unsigned int>(m_stateGroupDefs.size());
   m_stateSrc.groupDefs = m_stateGroupDefs.data();
   m_stateSrc.stateDefs = new StateDef[m_stateSrc.nStates];
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
      m_stateSrc.stateDefs[index].desc = nullptr;
      m_stateSrc.stateDefs[index].id.groupId = 0x0001;
      m_stateSrc.stateDefs[index].id.stateId = static_cast<uint16_t>(id);
      m_stateSrc.stateDefs[index].typeMask = CTLPI_STATE_TYPE_FLOAT | CTLPI_STATE_TYPE_DOUBLE | CTLPI_STATE_TYPE_UINT8 | CTLPI_STATE_TYPE_UINT16 | CTLPI_STATE_TYPE_UINT32
         | CTLPI_STATE_TYPE_UINT64 | CTLPI_STATE_TYPE_INT8 | CTLPI_STATE_TYPE_INT16 | CTLPI_STATE_TYPE_INT32 | CTLPI_STATE_TYPE_INT64;
      m_stateSrc.stateDefs[index].writable = 0;
      index++;
   }
   for (const auto& [id, v] : m_playerScores)
   {
      m_stateSrcNames[index] = std::format("Player Score #{}", id);
      if (id == 29 && m_defaultStateNameMask & (1ull << id))
         m_stateSrcNames[index] = "Credits";
      m_stateSrc.stateDefs[index].name = m_stateSrcNames[index].c_str();
      m_stateSrc.stateDefs[index].desc = nullptr;
      m_stateSrc.stateDefs[index].id.groupId = 0x0002;
      m_stateSrc.stateDefs[index].id.stateId = static_cast<uint16_t>(id);
      m_stateSrc.stateDefs[index].typeMask = CTLPI_STATE_TYPE_FLOAT | CTLPI_STATE_TYPE_DOUBLE | CTLPI_STATE_TYPE_UINT8 | CTLPI_STATE_TYPE_UINT16 | CTLPI_STATE_TYPE_UINT32
         | CTLPI_STATE_TYPE_UINT64 | CTLPI_STATE_TYPE_INT8 | CTLPI_STATE_TYPE_INT16 | CTLPI_STATE_TYPE_INT32 | CTLPI_STATE_TYPE_INT64;
      m_stateSrc.stateDefs[index].writable = 0;
      index++;
   }
   for (const auto& [id, v] : m_scoreDigits)
   {
      m_stateSrcNames[index] = std::format("Digit Score #{}", id);
      m_stateSrc.stateDefs[index].name = m_stateSrcNames[index].c_str();
      m_stateSrc.stateDefs[index].desc = nullptr;
      m_stateSrc.stateDefs[index].id.groupId = 0x0003;
      m_stateSrc.stateDefs[index].id.stateId = static_cast<uint16_t>(id);
      m_stateSrc.stateDefs[index].typeMask = CTLPI_STATE_TYPE_FLOAT | CTLPI_STATE_TYPE_DOUBLE | CTLPI_STATE_TYPE_UINT8 | CTLPI_STATE_TYPE_UINT16 | CTLPI_STATE_TYPE_UINT32
         | CTLPI_STATE_TYPE_UINT64 | CTLPI_STATE_TYPE_INT8 | CTLPI_STATE_TYPE_INT16 | CTLPI_STATE_TYPE_INT32 | CTLPI_STATE_TYPE_INT64;
      m_stateSrc.stateDefs[index].writable = 0;
      index++;
   }
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
   if (B2SServer::m_singleton == nullptr || inputIndex >= m_singleton->m_stateSrc.nStates)
      return -1;
   const int id = m_singleton->m_stateSrc.stateDefs[inputIndex].id.stateId;
   double val;
   switch (m_singleton->m_stateSrc.stateDefs[inputIndex].id.groupId)
   {
   case 0x0001: val = static_cast<double>(m_singleton->GetLampState(id)); break;
   case 0x0002: val = static_cast<double>(m_singleton->GetPlayerScore(id)); break;
   case 0x0003: val = static_cast<double>(m_singleton->GetScoreDigit(id)); break;
   default: return -1;
   }
   switch (type)
   {
   case CTLPI_STATE_TYPE_UINT8: *static_cast<uint8_t*>(pResult) = static_cast<uint8_t>(val); return 0;
   case CTLPI_STATE_TYPE_UINT16: *static_cast<uint16_t*>(pResult) = static_cast<uint16_t>(val); return 0;
   case CTLPI_STATE_TYPE_UINT32: *static_cast<uint32_t*>(pResult) = static_cast<uint32_t>(val); return 0;
   case CTLPI_STATE_TYPE_UINT64: *static_cast<uint64_t*>(pResult) = static_cast<uint64_t>(val); return 0;
   case CTLPI_STATE_TYPE_INT8: *static_cast<int8_t*>(pResult) = static_cast<int8_t>(val); return 0;
   case CTLPI_STATE_TYPE_INT16: *static_cast<int16_t*>(pResult) = static_cast<int16_t>(val); return 0;
   case CTLPI_STATE_TYPE_INT32: *static_cast<int32_t*>(pResult) = static_cast<int32_t>(val); return 0;
   case CTLPI_STATE_TYPE_INT64: *static_cast<int64_t*>(pResult) = static_cast<int64_t>(val); return 0;
   case CTLPI_STATE_TYPE_FLOAT: *static_cast<float*>(pResult) = static_cast<float>(val); return 0;
   case CTLPI_STATE_TYPE_DOUBLE: *static_cast<double*>(pResult) = val; return 0;
   default: return -1;
   }
}


// B2SSetScore / B2SSetScorePlayer

void B2SServer::B2SSetScore(int digit, int value, bool animate)
{
   const auto it = m_scoreDigits.find(digit);
   if (it == m_scoreDigits.end())
   {
      m_scoreDigits[digit] = value;
      UpdateStateSrc();
   }
   else
      it->second = value;
   B2SPluginEvent event { 'B', digit, value };
   m_msgApi->BroadcastMsg(m_endpointId, m_onStateChangeEventId, &event);
}

int B2SServer::GetScoreDigit(int digit) const
{
   const auto it = m_scoreDigits.find(digit);
   return it == m_scoreDigits.end() ? 0 : it->second;
}

void B2SServer::B2SSetScorePlayer(int playerno, int score)
{
   const auto it = m_playerScores.find(playerno);
   if (it == m_playerScores.end())
   {
      m_playerScores[playerno] = score;
      UpdateStateSrc();
   }
   else
      it->second = score;
   B2SPluginEvent event { 'C', playerno, score };
   m_msgApi->BroadcastMsg(m_endpointId, m_onStateChangeEventId, &event);
}

int B2SServer::GetPlayerScore(int player) const
{
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
   const auto it = m_lampStates.find(b2sId);
   if (it == m_lampStates.end())
   {
      m_lampStates[b2sId] = static_cast<float>(value);
      UpdateStateSrc();
   }
   else
      it->second = static_cast<float>(value);

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
   const auto it = m_lampStates.find(b2sId);
   return it == m_lampStates.end() ? 0.f : it->second;
}

}
