#include "common.h"

#include "B2SRenderer.h"

#include <cmath>
#include <vector>
#include <stack>
#include <algorithm>

namespace B2S {

B2SRenderer::B2SRenderer(MsgPluginAPI* const msgApi, const unsigned int endpointId, std::shared_ptr<B2STable> b2s)
   : m_b2s(b2s)
   , m_msgApi(msgApi)
   , m_endpointId(endpointId)
   , m_resURIResolver(*msgApi, endpointId, true, false, false, false)
{
   bool m_showGrill = false;
   m_grillCut = m_showGrill ? 0.f : static_cast<float>(m_b2s->m_grillHeight);

   char buf[32];
   msgApi->GetSetting("B2S", "ScoreviewDMDOverlay", buf, sizeof(buf));
   m_scoreviewDmdOverlay = atoi(buf) != 0;
   msgApi->GetSetting("B2S", "ScoreviewDMDAutoPos", buf, sizeof(buf));
   m_scoreviewDetectDmdFrame = atoi(buf) != 0;
   msgApi->GetSetting("B2S", "ScoreviewDMDX", buf, sizeof(buf));
   m_scoreviewDmdSubFrame.x = atoi(buf);
   msgApi->GetSetting("B2S", "ScoreviewDMDY", buf, sizeof(buf));
   m_scoreviewDmdSubFrame.y = atoi(buf);
   msgApi->GetSetting("B2S", "ScoreviewDMDWidth", buf, sizeof(buf));
   m_scoreviewDmdSubFrame.z = atoi(buf);
   msgApi->GetSetting("B2S", "ScoreviewDMDHeight", buf, sizeof(buf));
   m_scoreviewDmdSubFrame.w = atoi(buf);
   
   msgApi->GetSetting("B2S", "BackglassDMDOverlay", buf, sizeof(buf));
   m_backglassDmdOverlay = atoi(buf) != 0;
   msgApi->GetSetting("B2S", "BackglassDMDAutoPos", buf, sizeof(buf));
   m_backglassDetectDmdFrame = atoi(buf) != 0;
   msgApi->GetSetting("B2S", "BackglassDMDX", buf, sizeof(buf));
   m_backglassDmdSubFrame.x = atoi(buf);
   msgApi->GetSetting("B2S", "BackglassDMDY", buf, sizeof(buf));
   m_backglassDmdSubFrame.y = atoi(buf);
   msgApi->GetSetting("B2S", "BackglassDMDWidth", buf, sizeof(buf));
   m_backglassDmdSubFrame.z = atoi(buf);
   msgApi->GetSetting("B2S", "BackglassDMDHeight", buf, sizeof(buf));
   m_backglassDmdSubFrame.w = atoi(buf);

   VPXTextureInfo* bgTexInfo = nullptr;
   if (m_b2s->m_backglassImage.m_image)
      bgTexInfo = GetTextureInfo(m_b2s->m_backglassImage.m_image);
   else if (m_b2s->m_backglassOffImage.m_image)
      bgTexInfo = GetTextureInfo(m_b2s->m_backglassOffImage.m_image);
   m_b2sWidth = bgTexInfo ? static_cast<float>(bgTexInfo->width) : 1024.f;
   m_b2sHeight = (bgTexInfo ? static_cast<float>(bgTexInfo->height) : 768.f) - m_grillCut;

   VPXTextureInfo* dmdTexInfo = nullptr;
   if (m_b2s->m_dmdImage.m_image)
      dmdTexInfo = GetTextureInfo(m_b2s->m_dmdImage.m_image);
   m_dmdWidth = dmdTexInfo ? static_cast<float>(dmdTexInfo->width) : 1024.f;
   m_dmdHeight = dmdTexInfo ? static_cast<float>(dmdTexInfo->height) : 768.f;

   if (IsPinMAMEDriven())
   {
      m_getDevSrcMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_GET_SRC_MSG);
      m_onDevChangedMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_ON_SRC_CHG_MSG);
      m_msgApi->SubscribeMsg(m_endpointId, m_onDevChangedMsgId, OnDevSrcChanged, this);
      OnDevSrcChanged(m_onDevChangedMsgId, this, nullptr);
   }
}

B2SRenderer::~B2SRenderer()
{
   if (IsPinMAMEDriven())
   {
      m_msgApi->UnsubscribeMsg(m_onDevChangedMsgId, OnDevSrcChanged);
      m_msgApi->ReleaseMsgID(m_onDevChangedMsgId);
      m_msgApi->ReleaseMsgID(m_getDevSrcMsgId);
   }
   delete[] m_deviceStateSrc.deviceDefs;
}

bool B2SRenderer::IsPinMAMEDriven() const
{
   if (m_b2s->m_backglassOnImage.m_image && m_b2s->m_backglassOnImage.m_romIdType != B2SRomIDType::NotDefined)
      return true;
   for (const auto& bulb : m_b2s->m_backglassIlluminations)
      if (bulb.m_romIdType != B2SRomIDType::NotDefined)
         return true;
   return false;
}

void B2SRenderer::OnDevSrcChanged(const unsigned int msgId, void* userData, void* msgData)
{
   B2SRenderer* me = static_cast<B2SRenderer*>(userData);
   delete[] me->m_deviceStateSrc.deviceDefs;
   memset(&me->m_deviceStateSrc, 0, sizeof(me->m_deviceStateSrc));
   me->m_nSolenoids = 0;
   me->m_GIIndex = -1;
   me->m_nGIs = 0;
   me->m_lampIndex = -1;
   me->m_nLamps = 0;
   me->m_nMechs = 0;
   GetDevSrcMsg getSrcMsg = { 1024, 0, new DevSrcId[1024] };
   me->m_msgApi->BroadcastMsg(me->m_endpointId, me->m_getDevSrcMsgId, &getSrcMsg);
   for (unsigned int i = 0; i < getSrcMsg.count; i++)
   {
      // FIXME select PinMAME device source
      me->m_deviceStateSrc = getSrcMsg.entries[i];
      if (getSrcMsg.entries[i].deviceDefs)
      {
         me->m_deviceStateSrc.deviceDefs = new DeviceDef[getSrcMsg.entries[i].nDevices];
         memcpy(me->m_deviceStateSrc.deviceDefs, getSrcMsg.entries[i].deviceDefs, getSrcMsg.entries[i].nDevices * sizeof(DeviceDef));
      }
      break;
   }
   delete[] getSrcMsg.entries;

   if (me->m_deviceStateSrc.deviceDefs)
   {
      for (unsigned int i = 0; i < me->m_deviceStateSrc.nDevices; i++)
      {
         if (me->m_deviceStateSrc.deviceDefs[i].groupId == 0x0100)
         {
            if (me->m_GIIndex == -1)
               me->m_GIIndex = i;
            me->m_nGIs++;
         }
         else if (me->m_deviceStateSrc.deviceDefs[i].groupId == 0x0200)
         {
            if (me->m_lampIndex == -1)
               me->m_lampIndex = i;
            me->m_nLamps++;
         }
         else if (me->m_deviceStateSrc.deviceDefs[i].groupId == 0x0300)
         {
            if (me->m_mechIndex == -1)
               me->m_mechIndex = i;
            me->m_nMechs++;
         }
         else if ((me->m_GIIndex == -1) && (me->m_lampIndex == -1))
            me->m_nSolenoids++;
      }
   }

   if (me->m_b2s->m_backglassOnImage.m_image)
      me->m_b2s->m_backglassOnImage.m_romUpdater = me->ResolveRomPropUpdater(&me->m_b2s->m_backglassOnImage.m_brightness, me->m_b2s->m_backglassOnImage.m_romIdType, me->m_b2s->m_backglassOnImage.m_romId);

   for (auto& bulb : me->m_b2s->m_backglassIlluminations)
      switch (bulb.m_snippitType)
      {
      case B2SSnippitType::StandardImage:bulb.m_romUpdater = me->ResolveRomPropUpdater(&bulb.m_brightness, bulb.m_romIdType, bulb.m_romId); break;
      case B2SSnippitType::MechRotatingImage: bulb.m_romUpdater = me->ResolveRomPropUpdater(&bulb.m_mechRot, bulb.m_romIdType, bulb.m_romId); break;
      }
}

std::function<void()> B2SRenderer::ResolveRomPropUpdater(float* value, const B2SRomIDType romIdType, const int romId, const bool romInverted) const
{
   if (m_deviceStateSrc.deviceDefs == nullptr)
      return []() { };
   switch (romIdType)
   {
   case B2SRomIDType::NotDefined: break;
   case B2SRomIDType::Solenoid:
      if (0 < romId && (unsigned int)romId <= m_nSolenoids)
      {
         const int index = romId - 1;
         if (romInverted)
            return [this, value, index]() { *value = 1.f - m_deviceStateSrc.GetFloatState(index); };
         else
            return [this, value, index]() { *value = m_deviceStateSrc.GetFloatState(index); };
      }
      break;
   case B2SRomIDType::GIString:
      if (0 < romId && (unsigned int)romId <= m_nGIs)
      {
         const int index = m_GIIndex + romId - 1;
         if (romInverted)
            return [this, value, index]() { *value = 1.f - m_deviceStateSrc.GetFloatState(index); };
         else
            return [this, value, index]() { *value = m_deviceStateSrc.GetFloatState(index); };
      }
      break;
   case B2SRomIDType::Lamp:
      for (unsigned int i = 0; i < m_nLamps; i++)
      {
         if (m_deviceStateSrc.deviceDefs[m_lampIndex + i].groupId == 0x0200 && m_deviceStateSrc.deviceDefs[m_lampIndex + i].deviceId == romId)
         {
            const int index = m_lampIndex + i;
            if (romInverted)
               return [this, value, index]() { *value = 1.f - m_deviceStateSrc.GetFloatState(index); };
            else
               return [this, value, index]() { *value = m_deviceStateSrc.GetFloatState(index); };
         }
      }
      // value = (0 < romId && (unsigned int)romId <= m_nLamps) ? m_deviceStateSrc.GetFloatState(m_LampIndex + romId - 1) : 0.f;
      break;
   case B2SRomIDType::Mech:
      for (unsigned int i = 0; i < m_nMechs; i++)
      {
         if (m_deviceStateSrc.deviceDefs[m_mechIndex + i].groupId == 0x0300 && m_deviceStateSrc.deviceDefs[m_mechIndex + i].deviceId == romId)
         {
            const int index = m_mechIndex + i;
            return [this, value, index]() { *value = m_deviceStateSrc.GetFloatState(index); };
         }
      }
      // value = (0 < romId && (unsigned int)romId <= m_nLamps) ? m_deviceStateSrc.GetFloatState(m_LampIndex + romId - 1) : 0.f;
      break;
   }
   return []() { };
}

bool B2SRenderer::Render(VPXRenderContext2D* ctx)
{
   switch (ctx->window)
   {
   case VPXAnciliaryWindow::VPXWINDOW_Backglass: return RenderBackglass(ctx);
   case VPXAnciliaryWindow::VPXWINDOW_ScoreView: return RenderScoreview(ctx);
   }
   return false;
}

bool B2SRenderer::RenderBackglass(VPXRenderContext2D* ctx)
{
   ctx->srcWidth = m_b2sWidth;
   ctx->srcHeight = m_b2sHeight;

   // Update animations
   auto now = std::chrono::high_resolution_clock::now();
   float elapsed = static_cast<float>((now - m_lastBackglassRenderTick).count()) / 1000000000.0f;
   m_lastBackglassRenderTick = now;
   for (auto animation : m_b2s->m_backglassAnimations)
      animation.Update(elapsed); // TODO implement slowdown settings/props (scale elapsed)

   // Draw background
   m_b2s->m_backglassOnImage.m_romUpdater();
   if (m_b2s->m_backglassOnImage.m_image == nullptr || m_b2s->m_backglassOnImage.m_brightness < 1.f)
   {
      if (m_b2s->m_backglassImage.m_image)
      {
         VPXTextureInfo* texInfo = GetTextureInfo(m_b2s->m_backglassImage.m_image);
         ctx->DrawImage(ctx, m_b2s->m_backglassImage.m_image, 1.f, 1.f, 1.f, 1.f,
            0.f, m_grillCut, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height) - m_grillCut,
            0.f, 0.f, 0.f, // No rotation
            0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height) - m_grillCut);
      }
      else if (m_b2s->m_backglassOffImage.m_image)
      {
         VPXTextureInfo* texInfo = GetTextureInfo(m_b2s->m_backglassOffImage.m_image);
         ctx->DrawImage(ctx, m_b2s->m_backglassOffImage.m_image, 1.f, 1.f, 1.f, 1.f,
            0.f, m_grillCut, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height) - m_grillCut,
            0.f, 0.f, 0.f, // No rotation
            0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height) - m_grillCut);
      }
   }
   if (m_b2s->m_backglassOnImage.m_image)
   {
      VPXTextureInfo* texInfo = GetTextureInfo(m_b2s->m_backglassOnImage.m_image);
      ctx->DrawImage(ctx, m_b2s->m_backglassOnImage.m_image, 1.f, 1.f, 1.f, m_b2s->m_backglassOnImage.m_brightness,
         0.f, m_grillCut, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height) - m_grillCut,
         0.f, 0.f, 0.f, // No rotation
         0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height) - m_grillCut);
   }

   // Draw illuminations
   for (const auto& bulb : m_b2s->m_backglassIlluminations)
      bulb.Render(ctx);

   // Draw DMD overlay if enabled and available
   if (m_backglassDmdOverlay)
   {
      VPXTexture backTex = m_b2s->m_backglassImage.m_image ? m_b2s->m_backglassImage.m_image
         : m_b2s->m_backglassOffImage.m_image              ? m_b2s->m_backglassOffImage.m_image
                                                           : m_b2s->m_backglassOnImage.m_image;
      RenderDmdOverlay(ctx, m_backglassDetectDmdFrame ? backTex : nullptr, m_backglassDmdSrcId, m_backglassDmdSubFrame, m_backglassDmdSubFrameSearch);
   }

   return true;
}

bool B2SRenderer::RenderScoreview(VPXRenderContext2D* ctx)
{
   ctx->srcWidth = m_dmdWidth;
   ctx->srcHeight = m_dmdHeight;

   // Update animations
   auto now = std::chrono::high_resolution_clock::now();
   float elapsed = static_cast<float>((now - m_lastDmdRenderTick).count()) / 1000000000.0f;
   m_lastDmdRenderTick = now;
   for (auto animation : m_b2s->m_dmdAnimations)
      animation.Update(elapsed); // TODO implement slowdown settings/props (scale elapsed)

   // Draw background
   if (m_b2s->m_dmdImage.m_image)
      ctx->DrawImage(ctx, m_b2s->m_dmdImage.m_image, 1.f, 1.f, 1.f, 1.f,
         0.f, 0.f, m_dmdWidth, m_dmdHeight,
         0.f, 0.f, 0.f, // No rotation
         0.f, 0.f, m_dmdWidth, m_dmdHeight);

   // Draw illuminations
   for (const auto& bulb : m_b2s->m_dmdIlluminations)
      bulb.Render(ctx);

   // Draw DMD overlay if enabled and available
   if (m_scoreviewDmdOverlay)
      RenderDmdOverlay(ctx, m_scoreviewDetectDmdFrame ? m_b2s->m_dmdImage.m_image : nullptr, m_scoreviewDmdSrcId, m_scoreviewDmdSubFrame, m_scoreviewDmdSubFrameSearch);

   return true;
}

void B2SRenderer::RenderDmdOverlay(VPXRenderContext2D* ctx, VPXTexture image, CtlResId& dmdSrcId, ivec4& dmdSubFrame, std::future<ivec4>& dmdSubFrameSearch)
{
   ResURIResolver::DisplayState dmd = m_resURIResolver.GetDisplayState("ctrl://default/display");
   if (dmd.state.frame == nullptr)
      return;

   // When DMD source change, search for the DMD sub frame as it depends on the DMD source aspect ratio
   if (image && dmdSrcId.id != dmd.source->id.id)
   {
      dmdSubFrame = ivec4();
      dmdSrcId.id = dmd.source->id.id;
      const float ar = static_cast<float>(dmd.source->width) / static_cast<float>(dmd.source->height);
      dmdSubFrameSearch = std::async(std::launch::async, [this, image, ar]() { return SearchDmdSubFrame(image, ar); });
   }

   if (dmdSubFrameSearch.valid() && dmdSubFrameSearch.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
      dmdSubFrame = dmdSubFrameSearch.get();

   if (dmdSubFrame.z == 0.f || dmdSubFrame.w == 0.f)
      return;

   UpdateTexture(&m_dmdTex, dmd.source->width, dmd.source->height,
      dmd.source->frameFormat == CTLPI_DISPLAY_FORMAT_LUM8         ? VPXTextureFormat::VPXTEXFMT_BW
         : dmd.source->frameFormat == CTLPI_DISPLAY_FORMAT_SRGB565 ? VPXTextureFormat::VPXTEXFMT_sRGB565
                                                                   : VPXTextureFormat::VPXTEXFMT_sRGB8,
      dmd.state.frame);

   vec4 glassArea, glassAmbient(1.f, 1.f, 1.f, 1.f), glassTint(1.f, 1.f, 1.f, 1.f), glassPad;
   vec4 dmdTint(1.f, 1.f, 1.f, 1.f);
   ctx->DrawDisplay(ctx, VPXDisplayRenderStyle::VPXDMDStyle_Plasma,
      // First layer: glass
      nullptr, glassTint.x, glassTint.y, glassTint.z, 0.f, // Glass texture, tint and roughness
      glassArea.x, glassArea.y, glassArea.z, glassArea.w, // Glass texture coordinates (inside overall glass texture)
      glassAmbient.x, glassAmbient.y, glassAmbient.z, // Glass lighting from room
      // Second layer: emitter
      m_dmdTex, dmdTint.x, dmdTint.y, dmdTint.z, 1.f, 1.f, // DMD emitter, emitter tint, emitter brightness, emitter alpha
      glassPad.x, glassPad.y, glassPad.z, glassPad.w, // Emitter padding (from glass border)
      // Render quad
      static_cast<float>(dmdSubFrame.x), static_cast<float>(dmdSubFrame.y), static_cast<float>(dmdSubFrame.z), static_cast<float>(dmdSubFrame.w));
}

ivec4 B2SRenderer::SearchDmdSubFrame(VPXTexture image, float dmdAspectRatio)
{
   ivec4 subFrame;

   VPXTextureInfo* texInfo = GetTextureInfo(image);
   if (texInfo == nullptr)
      return subFrame;

   // Find the largest dark rectangle in the background image
   float maxHeuristic = 0.f;
   float selectedAspectRatio = 1.f;
   std::stack<int> st;
   vector<int> heights(texInfo->width, 0); // height of empty columns for each pixels in the row
   for (unsigned int y = 0; y < texInfo->height; ++y)
   {
      for (unsigned int x = 0; x < texInfo->width; ++x)
      {
         uint8_t lum = 0;
         const int pos = y * texInfo->width + x;
         switch (texInfo->format)
         {
         case VPXTEXFMT_BW: lum = texInfo->data[pos]; break;
         case VPXTEXFMT_sRGB8: lum = static_cast<uint8_t>(0.299f * texInfo->data[pos * 3] + 0.587f * texInfo->data[pos * 3 + 1] + 0.114f * texInfo->data[pos * 3 + 2]); break;
         case VPXTEXFMT_sRGBA8: lum = static_cast<uint8_t>(0.299f * texInfo->data[pos * 4] + 0.587f * texInfo->data[pos * 4 + 1] + 0.114f * texInfo->data[pos * 4 + 2]); break;
         default: return subFrame;
         }
         if (lum < 8)
            heights[x] += 1;
         else
            heights[x] = 0; // end a dark rectanlge
      }

      // Evaluate each rectangle that can be formed with the heights of the columns
      for (unsigned int x = 0; x <= texInfo->width; ++x)
      {
         while (!st.empty() && (x == texInfo->width || heights[st.top()] > heights[x]))
         {
            const int height = heights[st.top()];
            st.pop();
            const int width = st.empty() ? x : x - st.top() - 1;
            const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
            const float arMatch = aspectRatio < dmdAspectRatio ? aspectRatio / dmdAspectRatio : dmdAspectRatio / aspectRatio;
            const float heuristic = height * width * powf(arMatch, 0.5f);
            if (heuristic > maxHeuristic)
            {
               maxHeuristic = heuristic;
               subFrame.x = st.empty() ? 0 : st.top() + 1;
               subFrame.y = texInfo->height - y - 1;
               subFrame.z = width;
               subFrame.w = height;
            }
         }
         if (x < texInfo->width)
            st.push(x);
      }
   }

   // Do not select a too small area
   if (static_cast<unsigned int>(subFrame.z * subFrame.w) < texInfo->width * texInfo->height / 16)
      subFrame = ivec4();

   return subFrame;
}

}