// license:GPLv3+

#include <cassert>
#include <cstdlib>
#include <chrono>
#include <cstring>
#include <mutex>
#include <thread>

#include "plugins/MsgPlugin.h"
#include "plugins/VPXPlugin.h"
#include "plugins/ControllerPlugin.h"
#include "common.h"
#include "serum-decode.h"

#include <filesystem>

#include "plugins/LoggingPlugin.h"

using namespace std::string_literals;

namespace Serum {

LPI_IMPLEMENT

///////////////////////////////////////////////////////////////////////////////
// Serum Colorization plugin
//
// This plugin only rely on the generic message plugin API, the generic controller
// plugin API and the following messages:
// - PinMame/OnGameStart: msgData is PinMame game identifier (rom name)
// - PinMame/OnGameEnd

static const MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;

static uint32_t endpointId;
static unsigned int onControllerGameStartId, onControllerGameEndId;
static unsigned int onDmdSrcChangedId, getDmdSrcId, onDmdTrigger;

static bool isRunning = false;
static std::mutex sourceMutex;
static std::mutex stateMutex;
static std::thread colorizeThread;
static DisplaySrcId dmdId = {};

static Serum_Frame_Struc* pSerum = nullptr;
static unsigned int lastRawFrameId = 0;

MSGPI_STRING_VAL_SETTING(serumPathProp, "SerumPath", "Serum Path", "Folder that cotains Serum colorization files (cRZ, cROMc)", true, "", 1024);

class ColorizationState final
{
public:
   ColorizationState(unsigned int width, unsigned int height)
      : m_colorFrame(pSerum->SerumVersion == SERUM_V1 ? new uint8_t[width * height * 3] : nullptr)
      , m_width(width), m_height(height)
      , m_colorizedFrameFormat(pSerum->SerumVersion == SERUM_V1 ? CTLPI_DISPLAY_FORMAT_SRGB888 : CTLPI_DISPLAY_FORMAT_SRGB565)
      , m_colorizedframeId(static_cast<unsigned int>(std::rand()))
   {
      assert(m_width > 0);
      assert(m_height > 0);
   }

   ~ColorizationState()
   {
      delete[] m_colorFrame;
   }

   void UpdateFrameV1()
   {
      assert(pSerum && (pSerum->SerumVersion == SERUM_V1));
      for (unsigned int i = 0; i < m_width * m_height; i++)
         memcpy(&(m_colorFrame[i * 3]), &pSerum->palette[pSerum->frame[i] * 3], 3);
      m_colorizedframeId++;
   }
   
   void UpdateFrame32V2()
   {
      assert(pSerum && (pSerum->SerumVersion == SERUM_V2));
      if (pSerum->width32 > 0)
      {
         m_width32 = pSerum->width32;
         m_colorFrame32 = reinterpret_cast<uint8_t*>(pSerum->frame32);
         m_colorizedframeId++;
      }
   }
   
   void UpdateFrame64V2()
   {
      assert(pSerum && (pSerum->SerumVersion == SERUM_V2));
      if (pSerum->width64 > 0)
      {
         m_width64 = pSerum->width64;
         m_colorFrame64 = reinterpret_cast<uint8_t*>(pSerum->frame64);
         m_colorizedframeId++;
      }
   }

   // Serum v1
   uint8_t* const m_colorFrame;

   // Serum v2
   unsigned int m_width32 = 0;
   uint8_t* m_colorFrame32 = nullptr;
   unsigned int m_width64 = 0;
   uint8_t* m_colorFrame64 = nullptr;

   // Common state information
   const unsigned int m_width, m_height; // Size of identify frame (which can differ from the size of the colorized frame)
   const unsigned int m_colorizedFrameFormat;
   unsigned int m_colorizedframeId = 0;
   bool m_hasAnimation = false;
   std::chrono::high_resolution_clock::time_point m_animationTick;
   std::chrono::high_resolution_clock::time_point m_animationNextTick;
};

static ColorizationState* state = nullptr;

static void ColorizeThread()
{
   SetThreadName("Serum.ColorizeThread"s);
   unsigned int lastFrameId = 0;
   while (isRunning)
   {
      // Original PinMAME code would evaluate DMD frames at a fixed 60 FPS and color rotation are also based on a 60FPS rate. So update at this pace.
      std::this_thread::sleep_for(std::chrono::microseconds(16666));

      std::lock_guard<std::mutex> lock1(sourceMutex);
      if (dmdId.id.id == 0)
         continue;

      const DisplayFrame frame = dmdId.GetIdentifyFrame(dmdId.id);
      if (frame.frame == nullptr)
         break;

      if (frame.frameId != lastFrameId)
      {
         // We received a new identify frame to match & colorize
         lastFrameId = frame.frameId;
         const uint32_t firstrot = Serum_Colorize(const_cast<uint8_t*>(static_cast<const uint8_t*>(frame.frame)));
         if (firstrot != IDENTIFY_NO_FRAME)
         {
            // New frame, eventually starting a new animation
            std::lock_guard<std::mutex> lock2(stateMutex);
            bool newState = false;
            if (state == nullptr)
            {
               state = new ColorizationState(dmdId.width, dmdId.height);
               newState = true;
            }
            else if (state->m_width != dmdId.width || state->m_height != dmdId.height)
            {
               delete state;
               state = new ColorizationState(dmdId.width, dmdId.height);
               newState = true;
            }
            
            state->m_hasAnimation = firstrot != 0;
            if (state->m_hasAnimation)
            {
               state->m_animationTick = std::chrono::high_resolution_clock::now();
               state->m_animationNextTick = state->m_animationTick + std::chrono::milliseconds(firstrot);
            }
            if (pSerum->SerumVersion == SERUM_V1)
               state->UpdateFrameV1();
            else if (pSerum->SerumVersion == SERUM_V2)
            {
               if (pSerum->flags & FLAG_RETURNED_32P_FRAME_OK)
                  state->UpdateFrame32V2();
               if (pSerum->flags & FLAG_RETURNED_64P_FRAME_OK)
                  state->UpdateFrame64V2();
            }
            
            // This assumes that we won't decode another frame with a pup trigger before the message will be processed on main thread (should be ok)
            if (pSerum->triggerID != 0xffffffff)
               msgApi->RunOnMainThread(endpointId, 0, [](void* userData) { msgApi->BroadcastMsg(endpointId, onDmdTrigger, &pSerum->triggerID); }, nullptr);

            if (newState)
               msgApi->RunOnMainThread(endpointId, 0, [](void* userData) { msgApi->BroadcastMsg(endpointId, onDmdSrcChangedId, nullptr); }, nullptr);
         }
      }
      else if (state && state->m_hasAnimation)
      {
         // Perform current animation (catching up to the current time point)
         const auto now = std::chrono::high_resolution_clock::now();
         while (state->m_animationNextTick < now)
         {
            const uint32_t nextrot = Serum_Rotate();
            const uint32_t delayMs = nextrot & 0x0000ffff;
            if (delayMs == 0)
            {
               state->m_hasAnimation = false;
               break;
            }
            state->m_animationTick = state->m_animationNextTick;
            state->m_animationNextTick = state->m_animationNextTick + std::chrono::milliseconds(delayMs);
            if ((pSerum->SerumVersion == SERUM_V1) && (nextrot & FLAG_RETURNED_V1_ROTATED))
               state->UpdateFrameV1();
            if ((pSerum->SerumVersion == SERUM_V2) && (nextrot & FLAG_RETURNED_V2_ROTATED32))
               state->UpdateFrame32V2();
            if ((pSerum->SerumVersion == SERUM_V2) && (nextrot & FLAG_RETURNED_V2_ROTATED64))
               state->UpdateFrame64V2();
         }
      }
   }
   isRunning = false;
}

static DisplayFrame GetRenderFrame(const CtlResId id) 
{
   // TODO To be fully clean we should do a copy of the render (since the direct data is updated asynchronously, so eventually while it is read by consumer)
   std::lock_guard<std::mutex> lock(stateMutex);
   if (state == nullptr)
      return { 0, nullptr };
   else if (id.resId == 1) // Serum V2 Height 64
      return { state->m_colorizedframeId, state->m_colorFrame64 };
   else if (state->m_colorFrame32) // Serum V2 Height 32
      return { state->m_colorizedframeId, state->m_colorFrame32 };
   else // Serum V1
      return { state->m_colorizedframeId, state->m_colorFrame };
}

static void OnGetRenderDMDSrc(const unsigned int eventId, void* userData, void* msgData)
{
   if (pSerum == nullptr || state == nullptr || dmdId.id.id == 0)
      return;
   GetDisplaySrcMsg& msg = *static_cast<GetDisplaySrcMsg*>(msgData);
   if (state->m_colorFrame32 && state->m_width32)
   {
      if (msg.count < msg.maxEntryCount)
      {
         msg.entries[msg.count] = {};
         msg.entries[msg.count].id = { { endpointId, 0 } };
         msg.entries[msg.count].overrideId = dmdId.id;
         msg.entries[msg.count].width = state->m_width32;
         msg.entries[msg.count].height = 32;
         msg.entries[msg.count].hardware = CTLPI_DISPLAY_HARDWARE_RGB_LED;
         msg.entries[msg.count].frameFormat = CTLPI_DISPLAY_FORMAT_SRGB565;
         msg.entries[msg.count].GetRenderFrame = &GetRenderFrame;
      }
      msg.count++;
   }
   else if (state->m_colorFrame && state->m_width && state->m_height)
   {
      if (msg.count < msg.maxEntryCount)
      {
         msg.entries[msg.count] = {};
         msg.entries[msg.count].id = { { endpointId, 0 } };
         msg.entries[msg.count].overrideId = dmdId.id;
         msg.entries[msg.count].width = state->m_width;
         msg.entries[msg.count].height = state->m_height;
         msg.entries[msg.count].hardware = CTLPI_DISPLAY_HARDWARE_RGB_LED;
         msg.entries[msg.count].frameFormat = CTLPI_DISPLAY_FORMAT_SRGB888;
         msg.entries[msg.count].GetRenderFrame = &GetRenderFrame;
      }
      msg.count++;
   }
   if (state->m_colorFrame64 && state->m_width64)
   {
      if (msg.count < msg.maxEntryCount)
      {
         msg.entries[msg.count] = {};
         msg.entries[msg.count].id = { { endpointId, 1 } };
         msg.entries[msg.count].overrideId = dmdId.id;
         msg.entries[msg.count].width = state->m_width64;
         msg.entries[msg.count].height = 64;
         msg.entries[msg.count].hardware = CTLPI_DISPLAY_HARDWARE_RGB_LED;
         msg.entries[msg.count].frameFormat = CTLPI_DISPLAY_FORMAT_SRGB565;
         msg.entries[msg.count].GetRenderFrame = &GetRenderFrame;
      }
      msg.count++;
   }
}

// Select the first DMD with a large enough size that supports frame identification
static void OnDmdSrcChanged(const unsigned int, void*, void*)
{
   if (pSerum == nullptr)
      return;
   std::lock_guard<std::mutex> lock(sourceMutex);
   dmdId.id.id = 0;
   GetDisplaySrcMsg getSrcMsg = { 0, 0, nullptr };
   msgApi->BroadcastMsg(endpointId, getDmdSrcId, &getSrcMsg);
   if (getSrcMsg.count == 0)
      return;
   getSrcMsg = { getSrcMsg.count, 0, new DisplaySrcId[getSrcMsg.count] };
   msgApi->BroadcastMsg(endpointId, getDmdSrcId, &getSrcMsg);
   for (unsigned int i = 0; i < getSrcMsg.count; i++)
   {
      if (getSrcMsg.entries[i].GetIdentifyFrame != nullptr && getSrcMsg.entries[i].width >= 128)
      {
         dmdId = getSrcMsg.entries[i];
         break;
      }
   }
   delete[] getSrcMsg.entries;
}

static void StopColorization()
{
   // TODO this is somewhat slow as this will block for up to 16ms => use a condition variable
   isRunning = false;
   if (colorizeThread.joinable())
      colorizeThread.join();
   if (pSerum)
   {
      delete state;
      state = nullptr;
      pSerum = nullptr;
      Serum_Dispose();
      msgApi->BroadcastMsg(endpointId, onDmdSrcChangedId, nullptr);
   }
   dmdId.id.id = 0;
}

static void OnControllerGameStart(const unsigned int eventId, void* userData, void* msgData)
{
   StopColorization();
   // Setup Serum on the selected DMD
   const CtlOnGameStartMsg* msg = static_cast<const CtlOnGameStartMsg*>(msgData);
   assert(msg != nullptr && msg->gameId != nullptr);

   VPXTableInfo tableInfo;
   vpxApi->GetTableInfo(&tableInfo);
   std::filesystem::path tablePath = tableInfo.path;

   std::filesystem::path serumPath = serumPathProp_Get();
   const std::filesystem::path crz = msg->gameId + ".cRZ"s;
   const std::filesystem::path cromc = msg->gameId + ".cROMc"s;

   // Priority 1: serum/rom/rom.crz
   if (auto path1 = find_case_insensitive_file_path(tablePath.parent_path() / "serum" / msg->gameId / crz); !path1.empty())
      serumPath = path1.parent_path().parent_path();
   else if (auto path2 = find_case_insensitive_file_path(tablePath.parent_path() / "serum" / msg->gameId / cromc); !path2.empty())
      serumPath = path2.parent_path().parent_path();

   // Priority 2: pinmame/altcolor/rom/rom.crz
   else if (auto path3 = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame" / "altcolor" / msg->gameId / crz); !path3.empty())
      serumPath = path3.parent_path().parent_path();
   else if (auto path4 = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame" / "altcolor" / msg->gameId / cromc); !path4.empty())
      serumPath = path4.parent_path().parent_path();

   // Default to global user setup folder if no table specific file is found
   pSerum = Serum_Load(serumPath.string().c_str(), msg->gameId, FLAG_REQUEST_32P_FRAMES | FLAG_REQUEST_64P_FRAMES);
   OnDmdSrcChanged(onDmdSrcChangedId, nullptr, nullptr);
   if (pSerum)
   {
      isRunning = true;
      colorizeThread = std::thread(ColorizeThread);
   }
}

static void OnControllerGameEnd(const unsigned int eventId, void* userData, void* msgData)
{
   StopColorization();
}

}

using namespace Serum;

MSGPI_EXPORT void MSGPIAPI SerumPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;

   // Request and setup shared login API
   LPISetup(endpointId, msgApi);

   msgApi->RegisterSetting(endpointId, &serumPathProp);

   unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
   msgApi->ReleaseMsgID(getVpxApiId);

   onDmdTrigger = msgApi->GetMsgID("Serum", "OnDmdTrigger");
   msgApi->SubscribeMsg(endpointId, onControllerGameStartId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_GAME_START), OnControllerGameStart, nullptr);
   msgApi->SubscribeMsg(endpointId, onControllerGameEndId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_GAME_END), OnControllerGameEnd, nullptr);
   msgApi->SubscribeMsg(endpointId, onDmdSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG), OnDmdSrcChanged, nullptr);
   msgApi->SubscribeMsg(endpointId, getDmdSrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG), OnGetRenderDMDSrc, nullptr);
}

MSGPI_EXPORT void MSGPIAPI SerumPluginUnload()
{
   StopColorization();
   msgApi->UnsubscribeMsg(getDmdSrcId, OnGetRenderDMDSrc);
   msgApi->UnsubscribeMsg(onDmdSrcChangedId, OnDmdSrcChanged);
   msgApi->UnsubscribeMsg(onControllerGameStartId, OnControllerGameStart);
   msgApi->UnsubscribeMsg(onControllerGameEndId, OnControllerGameEnd);
   msgApi->ReleaseMsgID(onControllerGameStartId);
   msgApi->ReleaseMsgID(onControllerGameEndId);
   msgApi->ReleaseMsgID(onDmdTrigger);
   msgApi->ReleaseMsgID(onDmdSrcChangedId);
   msgApi->ReleaseMsgID(getDmdSrcId);
   msgApi->FlushPendingCallbacks(endpointId);
   msgApi = nullptr;
}
