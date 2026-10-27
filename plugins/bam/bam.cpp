// license:GPLv3+

#include "plugins/MsgPlugin.h"
#include "plugins/VPXPlugin.h"
#include "plugins/LoggingPlugin.h"

#include <cfloat>
#include <cmath>

#include "BAM_Tracker.h"

///////////////////////////////////////////////////////////////////////////////
// BAM Headtracking plugin
//
// This plugin implements the headtracking feature that was part of VPX up to
// 10.8, adapted to the current view system. The legacy implementation used to
// replace the whole view and projection matrices with its own 'screen as a
// window' math (asymetric frustum fitted to the physical display). This is not
// needed anymore since the 'window' (cabinet) view layout mode already
// performs the same off center projection, fitting the virtual table into the
// physical screen. Therefore this plugin only updates the view position each
// frame from the head position reported by the BAM Tracker application
// (BAM-Tracker.exe from Future Pinball BAM, which shares head tracking data
// through a shared memory block).
//
// Coordinate systems:
// - BAM Tracker reports the head position in millimeters, right handed, with
//   its origin at the screen center, +X to the right of the screen, +Y toward
//   the top of the screen surface and +Z toward the viewer.
// - VPX expects the player position in centimeters, expressed in the screen
//   frame, with its origin at the bottom center of the screen, +X to the
//   right, +Y along the screen surface toward its top edge and +Z toward the
//   viewer. The screen dimensions and inclination are the ones defined in the
//   cabinet settings of the application.
// - The view setup 'viewX/viewY/viewZ' fields are the player position rotated
//   into the playfield frame (origin at the bottom center of the playfield),
//   following the same math as ViewSetup::SetViewPosFromPlayerPosition.
//
// Since the view layout is handled by VPX (table fitting, screen size,
// inclination, viewport rotation,...), the only setting this plugin needs is
// the orientation of the tracker coordinate system against the cabinet screen
// which depends on the user setup of BAM-Tracker.

namespace BAM
{

using namespace std::string_literals;

const MsgPluginAPI* msgApi = nullptr;
VPXPluginAPI* vpxApi = nullptr;
uint32_t endpointId;
unsigned int getVpxApiId, onGameStartId, onGameEndId, onPrepareFrameId;

static BAM_Tracker::BAM_Tracker_Client tracker;
static float tableHeight = 0.f;
static float lastPlayerPos[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
static bool staticPrepassDisabled = false;
static bool windowModeWarned = false;
static int trackerRetryDelay = 0;

// VPX window (cabinet) view layout mode, see ViewLayoutMode in src/renderer/ViewSetup.h
constexpr int VLM_WINDOW = 2;

static const char* rotationValues[] = { "0", "90", "180", "270" };
MSGPI_ENUM_VAL_SETTING(trackerRotationProp, "TrackerOrientation", "Tracker orientation",
   "Rotation between the coordinate system of the BAM Tracker screen and the cabinet screen. "
   "Adjust this setting if the head position is applied on wrong axis.",
   true, 0, 4, rotationValues, 0);

LPI_USE_CPP();

void onGameStart(const unsigned int eventId, void* userData, void* eventData)
{
   windowModeWarned = false;
   lastPlayerPos[0] = lastPlayerPos[1] = lastPlayerPos[2] = FLT_MAX;
   if (vpxApi == nullptr)
      return;
   VPXTableInfo tableInfo;
   vpxApi->GetTableInfo(&tableInfo);
   tableHeight = tableInfo.tableHeight;
   tracker.Connect();
   if (tracker.IsBAMTrackerPresent())
   {
      // The point of view is updated each frame, so the static part prerender must be disabled
      vpxApi->DisableStaticPrerendering(1);
      staticPrepassDisabled = true;
      vpxApi->PushNotification("BAM headtracking enabled", 2000);
   }
   else
   {
      LPI_LOGI_CPP("BAM tracker not found. Start BAM-Tracker.exe to enable headtracking."s);
   }
}

void onGameEnd(const unsigned int eventId, void* userData, void* eventData)
{
   tableHeight = 0.f;
   if (vpxApi == nullptr)
      return;
   if (staticPrepassDisabled)
   {
      vpxApi->DisableStaticPrerendering(0);
      staticPrepassDisabled = false;
   }
}

void onPrepareFrame(const unsigned int eventId, void* userData, void* eventData)
{
   if (vpxApi == nullptr)
      return;
   if (!tracker.IsBAMTrackerPresent())
   {
      // The tracker application may be started after the game, so retry to
      // connect from time to time (connection failure is fast to evaluate)
      if (trackerRetryDelay > 0)
      {
         trackerRetryDelay--;
         return;
      }
      trackerRetryDelay = 60;
      tracker.Connect();
      if (!tracker.IsBAMTrackerPresent())
         return;
      LPI_LOGI_CPP("BAM tracker connected"s);
      vpxApi->PushNotification("BAM headtracking enabled", 2000);
      if (!staticPrepassDisabled)
      {
         vpxApi->DisableStaticPrerendering(1);
         staticPrepassDisabled = true;
      }
   }

   double x, y, z;
   if (!tracker.GetPosition(x, y, z))
      return;

   VPXViewSetupDef view;
   vpxApi->GetActiveViewSetup(&view);
   if (view.viewMode != VLM_WINDOW)
   {
      if (!windowModeWarned)
      {
         vpxApi->PushNotification("BAM headtracking requires the 'window' (cabinet) view layout mode", 5000);
         windowModeWarned = true;
      }
      return;
   }

   // Rotate the tracked head position around the screen normal to match the
   // cabinet screen frame (same convention as legacy VPX implementation)
   constexpr float sinCos[4][2] = { { 0.f, 1.f }, { -1.f, 0.f }, { 0.f, -1.f }, { 1.f, 0.f } };
   const int rot = trackerRotationProp_Val & 3;
   const double px = x * sinCos[rot][1] - y * sinCos[rot][0];
   const double py = x * sinCos[rot][0] + y * sinCos[rot][1];

   // Express the head position as a VPX player position: cm instead of mm and
   // origin moved from the screen center to the bottom center of the screen.
   // The player Y axis runs along the screen fitted axis which is always the
   // longest display edge, that is to say 'ScreenWidth' (width > height).
   const float playerPos[3] = { (float)(px * 0.1), (float)(py * 0.1 + 0.5 * view.screenWidth), (float)(z * 0.1) };

   // Skip view update if the head position did not meaningfully change
   constexpr float minMove = 0.05f; // [cm]
   if (fabsf(playerPos[0] - lastPlayerPos[0]) < minMove && fabsf(playerPos[1] - lastPlayerPos[1]) < minMove && fabsf(playerPos[2] - lastPlayerPos[2]) < minMove)
      return;
   lastPlayerPos[0] = playerPos[0];
   lastPlayerPos[1] = playerPos[1];
   lastPlayerPos[2] = playerPos[2];

   // Plugin enabled during a game, after the game start event
   if (tableHeight <= 0.f)
   {
      VPXTableInfo tableInfo;
      vpxApi->GetTableInfo(&tableInfo);
      tableHeight = tableInfo.tableHeight;
   }

   // Convert the player position from the screen frame to the playfield frame,
   // using the same math as ViewSetup::SetViewPosFromPlayerPosition
   const float inc = atan2f(view.windowTopZOfs - view.windowBottomZOfs, tableHeight) - view.screenInclination * (float)(3.14159265358979323846 / 180.);
   const float incCos = cosf(inc), incSin = sinf(inc);
   const float vpuY = CMTOVPU(playerPos[1]), vpuZ = CMTOVPU(playerPos[2]);
   view.viewX = CMTOVPU(playerPos[0]);
   view.viewY = vpuY * incCos - vpuZ * incSin;
   view.viewZ = vpuY * incSin + vpuZ * incCos + view.windowBottomZOfs * view.sceneScaleY / view.realToVirtualScale;
   vpxApi->SetActiveViewSetup(&view);
}

LPI_IMPLEMENT_CPP // Implement shared log support

}

using namespace BAM;

MSGPI_EXPORT void MSGPIAPI BAMPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   LPISetup(endpointId, msgApi);
   msgApi->RegisterSetting(endpointId, &trackerRotationProp);
   msgApi->BroadcastMsg(endpointId, getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &vpxApi);
   msgApi->SubscribeMsg(endpointId, onGameStartId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_START), onGameStart, nullptr);
   msgApi->SubscribeMsg(endpointId, onGameEndId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), onGameEnd, nullptr);
   msgApi->SubscribeMsg(endpointId, onPrepareFrameId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_PREPARE_FRAME), onPrepareFrame, nullptr);
}

MSGPI_EXPORT void MSGPIAPI BAMPluginUnload()
{
   tracker.Disconnect();
   msgApi->UnsubscribeMsg(onGameStartId, onGameStart, nullptr);
   msgApi->UnsubscribeMsg(onGameEndId, onGameEnd, nullptr);
   msgApi->UnsubscribeMsg(onPrepareFrameId, onPrepareFrame, nullptr);
   msgApi->ReleaseMsgID(getVpxApiId);
   msgApi->ReleaseMsgID(onGameStartId);
   msgApi->ReleaseMsgID(onGameEndId);
   msgApi->ReleaseMsgID(onPrepareFrameId);
   tableHeight = 0.f;
   vpxApi = nullptr;
   msgApi = nullptr;
}
