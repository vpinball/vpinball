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
   const B2SImage& bgImage = m_b2s->m_backglassImage.m_image ? m_b2s->m_backglassImage : m_b2s->m_backglassOffImage;
   int bgWidth, bgHeight;
   GetTextureInfo(bgImage.m_image, &bgWidth, &bgHeight);
   m_b2sWidth = static_cast<float>(bgWidth);
   m_b2sHeight = static_cast<float>(bgHeight) - m_grillCut;

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
   me->m_LampIndex = -1;
   me->m_nLamps = 0;
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

   if (me->m_deviceStateSrc.deviceDefs == nullptr)
      return;

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
         if (me->m_LampIndex == -1)
            me->m_LampIndex = i;
         me->m_nLamps++;
      }
      else if ((me->m_GIIndex == -1) && (me->m_LampIndex == -1))
         me->m_nSolenoids++;
   }
}

float B2SRenderer::GetBrightness(const float localState, const B2SRomIDType romIdType, const int romId, const bool romInverted) const
{
   float value = 0.f;
   switch (romIdType)
   {
   case B2SRomIDType::NotDefined: return localState;
   case B2SRomIDType::Solenoid: value = (0 < romId && (unsigned int)romId <= m_nSolenoids) ? m_deviceStateSrc.GetFloatState(romId - 1) : 0.f; break;
   case B2SRomIDType::GIString: value = (0 < romId && (unsigned int)romId <= m_nGIs) ? m_deviceStateSrc.GetFloatState(m_GIIndex + romId - 1) : 0.f; break;
   case B2SRomIDType::Lamp: value = (0 < romId && (unsigned int)romId <= m_nLamps) ? m_deviceStateSrc.GetFloatState(m_LampIndex + romId - 1) : 0.f; break;
   case B2SRomIDType::Mech: break; // TODO implement mech
   }
   return romInverted ? 1.f - value : value;
}

bool B2SRenderer::Render(VPXRenderContext2D* ctx)
{
   ctx->srcWidth = m_b2sWidth;
   ctx->srcHeight = m_b2sHeight;
   
   if (ctx->window != VPXAnciliaryWindow::VPXWINDOW_Backglass) // Not yet implemented
      return false;

   // Update animations
   auto now = std::chrono::high_resolution_clock::now();
   float elapsed = static_cast<float>((now - m_lastRenderTick).count()) / 1000000000.0f;
   m_lastRenderTick = now;
   for (auto animation : m_b2s->m_backglassAnimations)
      animation.Update(elapsed); // TODO implement slowdown settings/props (scale elapsed)

   // Draw background
   {
      float backgroundLight = 0.f;
      if (m_b2s->m_backglassOnImage.m_image)
         backgroundLight = GetBrightness(0.f, m_b2s->m_backglassOnImage.m_romIdType, m_b2s->m_backglassOnImage.m_romId);
      if (backgroundLight < 1.f)
      {
         if (m_b2s->m_backglassImage.m_image)
         {
            int bgW, bgH;
            GetTextureInfo(m_b2s->m_backglassImage.m_image, &bgW, &bgH);
            ctx->DrawImage(ctx, m_b2s->m_backglassImage.m_image,
               1.f, 1.f, 1.f, 1.f,
               0.f, m_grillCut, static_cast<float>(bgW), static_cast<float>(bgH) - m_grillCut,
               0.f, 0.f, static_cast<float>(bgW), static_cast<float>(bgH) - m_grillCut);
         }
         else if (m_b2s->m_backglassOffImage.m_image)
         {
            int bgW, bgH;
            GetTextureInfo(m_b2s->m_backglassOffImage.m_image, &bgW, &bgH);
            ctx->DrawImage(ctx, m_b2s->m_backglassOffImage.m_image,
               1.f, 1.f, 1.f, 1.f,
               0.f, m_grillCut, static_cast<float>(bgW), static_cast<float>(bgH) - m_grillCut,
               0.f, 0.f, static_cast<float>(bgW), static_cast<float>(bgH) - m_grillCut);
      
         }
      }
      if (backgroundLight > 0.f)
      {
         int bgW, bgH;
         GetTextureInfo(m_b2s->m_backglassOnImage.m_image, &bgW, &bgH);
         ctx->DrawImage(ctx, m_b2s->m_backglassOnImage.m_image, 
            1.f, 1.f, 1.f, backgroundLight,
            0.f, m_grillCut, static_cast<float>(bgW), static_cast<float>(bgH) - m_grillCut,
            0.f, 0.f, static_cast<float>(bgW), static_cast<float>(bgH) - m_grillCut);
      }
   }

   // Draw illuminations
   for (const auto& bulb : m_b2s->m_backglassIlluminations)
   {
      const float x = static_cast<float>(bulb.m_locationX);
      const float y = static_cast<float>(bulb.m_locationY);
      const float w = static_cast<float>(bulb.m_width);
      const float h = static_cast<float>(bulb.m_height);
      const float state = GetBrightness(bulb.m_isLit ? 1.f : 0.f, bulb.m_romIdType, bulb.m_romId, bulb.m_romInverted);
      if (state > 0.f)
      {
         int bulbW, bulbH;
         GetTextureInfo(bulb.m_image, &bulbW, &bulbH);
         ctx->DrawImage(ctx, bulb.m_image, 
            bulb.m_lightColor.x, bulb.m_lightColor.y, bulb.m_lightColor.z, state,
            0.f, 0.f, static_cast<float>(bulbW), static_cast<float>(bulbH),
            x, y, w, h);
      }
   }

   return true;
}

}