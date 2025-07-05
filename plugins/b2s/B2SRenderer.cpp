#include "common.h"

#include "B2SRenderer.h"

namespace B2S {

B2SRenderer::B2SRenderer(MsgPluginAPI* const msgApi, const unsigned int endpointId, std::shared_ptr<B2STable> b2s)
   : m_b2s(b2s)
   , m_msgApi(msgApi)
   , m_endpointId(endpointId)
{
   bool m_showGrill = false;
   m_grillCut = m_showGrill ? 0.f : static_cast<float>(m_b2s->m_grillHeight);

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

   return true;
}

}