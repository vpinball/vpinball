// license:GPLv3+

#pragma once

#include "common.h"
#include "B2SDataModel.h"
#include "B2SProPhysics.h"
#include "B2SProEffects.h"
#include "B2SProSequence.h"
#include "DMDOverlay/DMDOverlay.h"
#include "seg/Dream7Display.h"
#include "seg/LEDBox.h"

#include "plugins/ControllerPlugin.h"
#include "plugins/ResURIResolver.h"
#include "plugins/VPXPlugin.h"

#include <future>
#include <chrono>
#include <map>
#include <unordered_dense.h>

namespace B2S
{

class B2SRenderer final
{
public:
   B2SRenderer(const MsgPluginAPI* const msgApi, const VPXPluginAPI* const vpxApi, const unsigned int endpointId, std::shared_ptr<B2STable> b2s);
   ~B2SRenderer();

   static void RegisterSettings(const MsgPluginAPI* const msgApi, unsigned int endpointId);
   // Fills the renderer owned entries (ShowGrill, DualMode, UpgradedSegmentRendering) of the plugin
   // declared "settings" sheet, returns the number of items filled
   static int FillSettingsItems(PagedUIItemDef* items);

   bool Render(VPXRenderContext2D* context, class B2SServer* server);

private:
   std::function<void()> ResolveRomPropUpdater(const std::vector<StateSrcId>& items, float* value, const B2SRomIDType romIdType, const int romId, const bool romInverted = false, const int romValue = 0) const;
   bool RenderBackglass(VPXRenderContext2D* context, class B2SServer* server);
   bool RenderScoreView(VPXRenderContext2D* context, class B2SServer* server);
   void RenderDisplays(VPXRenderContext2D* ctx, B2SServer* server, int target, VPXTexture backImage);
   // pass: 0 = B2S Pro elements behind the canvas (also runs the Pro runtime and state updates), 1 = the others, 2 = everything
   void RenderBulbs(VPXRenderContext2D* ctx, const B2SServer* server, const vector<std::unique_ptr<B2SBulb>>& bulbs, float elapsed, B2SDualMode dualMode, int pass);
   void RenderScores(VPXRenderContext2D* ctx, B2SServer* server, const B2SScores& scores, float elapsed, int pass);
   // B2S Pro 3D reel: the digit artwork rendered as a backlit drum, cached per image, size and backlight state
   VPXTexture Reel3DTexture(VPXTexture image, int width, int height, int brightness, int temperature, int depth);
   VPXTexture Reel3DOverlay(const B2SScore& reel, int width, int height);
   void RenderLegacyScoreDisplay(VPXRenderContext2D* ctx, B2SServer* server, const B2SScore& reel, bool dream7, const std::vector<SegElementType>& segTypes,
      const ankerl::unordered_dense::map<int, string>& playerScoreText, ankerl::unordered_dense::map<int, size_t>& playerScoreOffset);
   void UpdateAnimations(vector<B2SAnimation>& animations, float elapsed, B2SServer* server, B2SDualMode dualMode);
   B2SDualMode ActiveDualMode() const;
   void OnRandomAnimationTrigger(vector<B2SAnimation>& animations, B2SRomIDType romIdType, int romId, bool start);
   void DispatchScriptTriggers(B2SServer* server);
   // B2S Pro runtime, once per frame before the bulbs are drawn: trigger polling, pivot / rotation advance and the
   // physics clock (fixed 1 ms steps advancing the flipper pivots and the balls together)
   void UpdateProRuntime(const vector<std::unique_ptr<B2SBulb>>& bulbs, B2SServer* server, float elapsed);
   // Returns the trigger value of a Pro trigger when known (bus probe or script lamp write, most recent wins)
   bool PollProTrigger(B2SBulb::ProRuntime& rt, B2SServer* server, B2SProTriggerType type, int id, float& value);
   void UpdateProTrigger(B2SBulb& bulb, B2SServer* server, B2SProTriggerType type, int id, bool inverted, float elapsed, bool advance);

   struct ProPhysicsWorld
   {
      vector<std::unique_ptr<B2SProPhysicsBall>> balls;
      vector<B2SBulb*> pivots; // Physics driven pivots (flippers)
      double pendingSeconds = 0.0;
      vector<std::unique_ptr<B2SProSequenceGroup>> sequences; // Motion path sequence groups (troughs)
      std::map<const B2SBulb*, B2SProSequenceGroup*> sequenceOf;
   };
   std::map<const vector<std::unique_ptr<B2SBulb>>*, ProPhysicsWorld> m_proPhysics; // One world per illumination list
   void BuildProPhysics(const vector<std::unique_ptr<B2SBulb>>& bulbs, B2SServer* server);

   struct Reel3DKey
   {
      VPXTexture image;
      int width, height, brightness, temperature, depth;
      bool operator<(const Reel3DKey& o) const { return std::tie(image, width, height, brightness, temperature, depth) < std::tie(o.image, o.width, o.height, o.brightness, o.temperature, o.depth); }
   };
   std::map<Reel3DKey, VPXTexture> m_reel3DCache;
   struct Reel3DOverlayEntry
   {
      int width = 0, height = 0, depth = 0, glass = 0;
      VPXTexture texture = nullptr;
   };
   std::map<const B2SScore*, Reel3DOverlayEntry> m_reel3DOverlays;

   // B2S Pro dynamic front lighting: a light's authored field applied to each image snippit below it, one
   // composed overlay per (light, snippit) pair, recomposed when the snippit moves or the light switches
   struct ProLightOverlay
   {
      VPXTexture texture = nullptr;
      float x = 0.f, y = 0.f;
      int width = 0, height = 0;
      float snippitX = -1.f, snippitY = -1.f;
      bool valid = false;
   };
   std::map<std::pair<const B2SBulb*, const B2SBulb*>, ProLightOverlay> m_proLightOverlays;
   bool IsProDynamicLight(const B2SBulb& bulb) const { return bulb.m_pro.dynamicFrontLighting && bulb.m_pro.dynamicFrontLightField != nullptr && !bulb.m_isImageSnippit; }
   void RenderProDynamicLight(VPXRenderContext2D* ctx, const B2SBulb& light, const vector<std::unique_ptr<B2SBulb>>& bulbs, B2SDualMode dualMode);

   std::shared_ptr<B2STable> m_b2s;

   const MsgPluginAPI* const m_msgApi;
   const VPXPluginAPI* const m_vpxApi;
   const unsigned int m_endpointId;
   PinballPlugin::Controller::CtrlItemConsumer<ControllerDef> m_pinmameControllers;
   mutable PinballPlugin::Controller::CtrlItemConsumer<StateSrcId> m_stateSources;
   void OnStateSrcChanged(const std::vector<StateSrcId>& items);
   mutable PinballPlugin::Controller::CtrlItemConsumer<SegSrcId> m_segSources;

   PinballPlugin::ResURIResolver m_resURIResolver;
   vector<std::unique_ptr<DMDOverlay::Renderer>> m_dmdOverlays; // One per <DMD> display of the edited document, by document index

   std::chrono::time_point<std::chrono::steady_clock> m_lastBackglassRenderTick = std::chrono::steady_clock::now();
   std::chrono::time_point<std::chrono::steady_clock> m_lastDmdRenderTick = std::chrono::steady_clock::now();

   B2SAnimation* m_lastRandomAnimation = nullptr; // Last animation started through a random trigger

   std::map<int, B2SReelDigit> m_reelDigits; // Rolling reel digit states, keyed by resolved digit index

   // Legacy segment renderer (opt-out of the upgraded segment rendering): one rasterized display per score
   // display, either a single multi-digit Dream7Display or one LEDBox per digit, keyed by the score element
   struct LegacySegDisplay
   {
      bool dream7 = false;
      bool built = false;
      std::unique_ptr<seg::Dream7Display> display;
      vector<std::unique_ptr<seg::LEDBox>> boxes;
   };
   std::map<const B2SScore*, LegacySegDisplay> m_legacySegDisplays;

   VPXTexture m_whiteTexture = nullptr; // 1x1 white, tinted to draw flat colour areas (B2S Pro canvas backing)

   bool m_showGrill = false;
   float m_b2sWidth = 0.f;
   float m_b2sHeight = 0.f;
   float m_dmdWidth = 0.f;
   float m_dmdHeight = 0.f;
   float m_grillCut = 0.f;
};

}
