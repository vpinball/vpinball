// license:GPLv3+

#pragma once

#include "common.h"
#include "B2SDataModel.h"
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
   void RenderBulbs(VPXRenderContext2D* ctx, const B2SServer* server, const vector<std::unique_ptr<B2SBulb>>& bulbs, float elapsed, B2SDualMode dualMode);
   void RenderScores(VPXRenderContext2D* ctx, B2SServer* server, const B2SScores& scores, float elapsed);
   void RenderLegacyScoreDisplay(VPXRenderContext2D* ctx, B2SServer* server, const B2SScore& reel, bool dream7, const std::vector<SegElementType>& segTypes,
      const ankerl::unordered_dense::map<int, string>& playerScoreText, ankerl::unordered_dense::map<int, size_t>& playerScoreOffset);
   void UpdateAnimations(vector<B2SAnimation>& animations, float elapsed, B2SServer* server, B2SDualMode dualMode);
   B2SDualMode ActiveDualMode() const;
   void OnRandomAnimationTrigger(vector<B2SAnimation>& animations, B2SRomIDType romIdType, int romId, bool start);
   void DispatchScriptTriggers(B2SServer* server);

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

   bool m_showGrill = false;
   float m_b2sWidth = 0.f;
   float m_b2sHeight = 0.f;
   float m_dmdWidth = 0.f;
   float m_dmdHeight = 0.f;
   float m_grillCut = 0.f;
};

}
