// license:GPLv3+

#pragma once

#include <future>
#include <chrono>

#include "common.h"
#include "B2SDataModel.h"
#include "B2SDMDOverlay.h"

#include "plugins/ControllerPlugin.h"

#include "plugins/ResURIResolver.h"

namespace B2S {
   
class B2SRenderer final
{
public:
   B2SRenderer(const MsgPluginAPI* const msgApi, const unsigned int endpointId, std::shared_ptr<B2STable> b2s);
   ~B2SRenderer();

   static void RegisterSettings(const MsgPluginAPI* const msgApi, unsigned int endpointId);

   bool Render(VPXRenderContext2D* context, class B2SServer* server);

private:
   std::function<void()> ResolveRomPropUpdater(float* value, const B2SRomIDType romIdType, const int romId, const bool romInverted = false) const;
   bool RenderBackglass(VPXRenderContext2D* context, class B2SServer* server);
   bool RenderScoreView(VPXRenderContext2D* context, class B2SServer* server);
   void RenderBulbs(VPXRenderContext2D* ctx, B2SServer* server, const vector<std::unique_ptr<B2SBulb>>& bulbs);
   void RenderScores(VPXRenderContext2D* ctx, B2SServer* server, const B2SScores& scores);

   std::shared_ptr<B2STable> m_b2s;

   const MsgPluginAPI* const m_msgApi;
   const unsigned int m_endpointId;
   unsigned int m_getDevSrcMsgId = 0;
   unsigned int m_onDevChangedMsgId = 0;
   static void OnDevSrcChanged(const unsigned int msgId, void* userData, void* msgData);
   DevSrcId m_deviceStateSrc {};
   unsigned int m_nSolenoids = 0;
   int m_GIIndex = -1;
   unsigned int m_nGIs = 0;
   int m_lampIndex = -1;
   unsigned int m_nLamps = 0;
   int m_mechIndex = -1;
   unsigned int m_nMechs = 0;

   unsigned int m_getSegSrcMsgId = 0;
   unsigned int m_onSegChangedMsgId = 0;
   static void OnSegSrcChanged(const unsigned int msgId, void* userData, void* msgData);
   vector<SegSrcId> m_segDisplays;

   ResURIResolver m_resURIResolver;
   VPXTexture m_dmdTex = nullptr;
   B2SDMDOverlay m_scoreViewDmdOverlay;
   B2SDMDOverlay m_backglassDmdOverlay;

   std::chrono::time_point<std::chrono::steady_clock> m_lastBackglassRenderTick;
   std::chrono::time_point<std::chrono::steady_clock> m_lastDmdRenderTick;

   bool m_showGrill = false;
   float m_b2sWidth = 0.f;
   float m_b2sHeight = 0.f;
   float m_dmdWidth = 0.f;
   float m_dmdHeight = 0.f;
   float m_grillCut = 0.f;
};

}
