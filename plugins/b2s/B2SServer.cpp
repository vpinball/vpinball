// license:GPLv3+

#include "common.h"

#include "B2SServer.h"

namespace B2S {

B2SServer* B2SServer::m_singleton = nullptr;

B2SServer::B2SServer(const MsgPluginAPI* const msgApi, unsigned int endpointId, const VPXPluginAPI* const vpxApi, ScriptClassDef* pinmameClassDef)
   : m_msgApi(msgApi)
   , m_endpointId(endpointId)
   , m_vpxApi(vpxApi)
   , m_onGetAuxRendererId(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER))
   , m_onAuxRendererChgId(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG))
   , m_ancillaryRendererDef({ "B2S", "B2S Backglass & FullDMD", "Renderer for directb2s backglass files", this, OnRender })
   , m_onGetDevSrcId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_GET_SRC_MSG))
   , m_onDevSrcChgId(msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_ON_SRC_CHG_MSG))
   , m_pinmameClassDef(pinmameClassDef)
   , m_pinmame(pinmameClassDef ? pinmameClassDef->CreateObject() : nullptr)
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
      const string folderName = tablePath.parent_path().filename().string();
      b2sFilename = find_case_insensitive_file_path(tablePath.parent_path() / (folderName + ".directb2s"));
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
            LOGE("Failed to load B2S file: %s", path.c_str());
         }
         return b2s;
      };
      // B2S file format is heavily unoptimized so perform loading asynchronously (all assets are directly included in the XML file using Base64 encoding)
      m_loadedB2S = std::async(std::launch::async, loadFile, b2sFilename);
   }

   m_msgApi->SubscribeMsg(m_endpointId, m_onGetAuxRendererId, OnGetRenderer, this);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);

   m_devSrc.id.endpointId = m_endpointId;
   m_devSrc.GetByteState = GetByteState;
   m_devSrc.GetFloatState = GetFloatState;
   m_devSrc.SetChangeCallback = RegisterStateChangeCallback;
   m_msgApi->SubscribeMsg(m_endpointId, m_onGetDevSrcId, OnGetDevSrc, this);
   UpdateDevSrc();
}

B2SServer::~B2SServer()
{
   if (m_loadedB2S.valid())
      m_loadedB2S.get();
   m_renderer = nullptr;

   m_msgApi->UnsubscribeMsg(m_onGetAuxRendererId, OnGetRenderer);
   m_msgApi->BroadcastMsg(m_endpointId, m_onAuxRendererChgId, nullptr);
   m_msgApi->ReleaseMsgID(m_onGetAuxRendererId);
   m_msgApi->ReleaseMsgID(m_onAuxRendererChgId);

   m_msgApi->UnsubscribeMsg(m_onGetDevSrcId, OnGetDevSrc);
   m_msgApi->ReleaseMsgID(m_onGetDevSrcId);
   m_msgApi->ReleaseMsgID(m_onDevSrcChgId);
   
   delete[] m_devSrc.deviceDefs;

   if (m_pinmame)
      PSC_RELEASE(m_pinmameClassDef, m_pinmame);

   if (m_onDestroyHandler)
      m_onDestroyHandler(this);
   
   m_singleton = nullptr;
}

void B2SServer::OnGetDevSrc(const unsigned int, void* userData, void* msgData)
{
   auto me = static_cast<B2SServer*>(userData);
   auto msg = static_cast<GetDevSrcMsg*>(msgData);

   if (msg->count < msg->maxEntryCount)
      memcpy(&msg->entries[msg->count], &me->m_devSrc, sizeof(DevSrcId));
   msg->count++;
}

void B2SServer::UpdateDevSrc()
{
   delete[] m_devSrc.deviceDefs;
   m_devSrc.nDevices = static_cast<unsigned int>(m_states.size());
   m_devSrc.deviceDefs = new DeviceDef[m_devSrc.nDevices];
   m_devSrcNames.resize(m_devSrc.nDevices);
   uint16_t index = 0;
   for (const auto& [id, v] : m_states)
   {
      m_devSrcNames[index] = std::format("B2S.Data #{}", id);
      m_devSrc.deviceDefs[index].name = m_devSrcNames[index].c_str();
      m_devSrc.deviceDefs[index].groupId = 0x0001;
      m_devSrc.deviceDefs[index].deviceId = id;
      index++;
   }
   m_stateChgCallbacks.clear();
   m_msgApi->BroadcastMsg(m_endpointId, m_onDevSrcChgId, nullptr);
}

uint8_t MSGPIAPI B2SServer::GetByteState(const unsigned int deviceIndex)
{
   if (B2SServer::m_singleton == nullptr || deviceIndex >= m_singleton->m_devSrc.nDevices)
      return 0;
   int b2sId = m_singleton->m_devSrc.deviceDefs[deviceIndex].deviceId;
   return static_cast<uint8_t>(m_singleton->GetState(b2sId) * 255.f);
}

float MSGPIAPI B2SServer::GetFloatState(const unsigned int deviceIndex)
{
   if (B2SServer::m_singleton == nullptr || deviceIndex >= m_singleton->m_devSrc.nDevices)
      return 0.f;
   int b2sId = m_singleton->m_devSrc.deviceDefs[deviceIndex].deviceId;
   return m_singleton->GetState(b2sId);
}

void MSGPIAPI B2SServer::RegisterStateChangeCallback(unsigned int deviceIndex, int isRegister, ctlpi_chg_callback cb, void* ctx)
{
   if (B2SServer::m_singleton == nullptr || deviceIndex >= m_singleton->m_devSrc.nDevices)
   {
      LOGE("Invalid state listener registration requested");
      assert(false);
      return;
   }
   const int b2sId = m_singleton->m_devSrc.deviceDefs[deviceIndex].deviceId;
   if (auto mapIt = m_singleton->m_stateChgCallbacks.find(b2sId); mapIt == m_singleton->m_stateChgCallbacks.end())
      m_singleton->m_stateChgCallbacks[b2sId] = vector<ChgCallback>();
   auto& callbacks = m_singleton->m_stateChgCallbacks[b2sId];
   auto it = std::ranges::find_if(callbacks, [&cb](const ChgCallback& a) { return a.m_callback == cb; });
   if (isRegister)
   {
      if (it != callbacks.end())
      {
         LOGE("Requested to register an already registered state change listener");
         assert(false);
      }
      else
      {
         callbacks.emplace_back(cb, deviceIndex, ctx);
      }
   }
   else
   {
      if (it != callbacks.end())
      {
         callbacks.erase(it);
      }
      else
      {
         LOGE("Requested to unregister an unknown state change listener");
         assert(false);
      }
   }
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

void B2SServer::ForwardPinMAMECall(int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet) { m_pinmameClassDef->members[memberIndex].Call(m_pinmame, memberIndex, pArgs, pRet); }


void B2SServer::B2SSetData(int b2sId, int value)
{
   const auto it = m_states.find(b2sId);
   if (it == m_states.end())
   {
      m_states[b2sId] = static_cast<float>(value);
      UpdateDevSrc();
   }
   else
   {
      it->second = static_cast<float>(value);
      const auto chgIt = m_stateChgCallbacks.find(b2sId);
      if (chgIt != m_stateChgCallbacks.end())
      {
         for (const auto& cb : chgIt->second)
            cb.m_callback(cb.m_index, cb.m_context);
      }
   }
}

void B2SServer::B2SSetData(const std::string& group, int value)
{

}

float B2SServer::GetState(int b2sId) const
{
   const auto it = m_states.find(b2sId);
   return it == m_states.end() ? 0.f : it->second;
}

// Scores

void B2SServer::B2SSetScorePlayer(int playerno, int score)
{
   m_playerScores[playerno] = score;
}

int B2SServer::GetPlayerScore(int player) const
{
   const auto it = m_playerScores.find(player);
   return it == m_playerScores.end() ? 0 : it->second;
}

void B2SServer::B2SSetScoreDigit(int digit, int value)
{
   m_scoreDigits[digit] = static_cast<float>(value);
}

float B2SServer::GetScoreDigit(int b2sId) const
{
   const auto it = m_scoreDigits.find(b2sId);
   return it == m_scoreDigits.end() ? 0.f : it->second;
}

}