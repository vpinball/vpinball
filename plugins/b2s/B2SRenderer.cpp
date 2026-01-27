// license:GPLv3+

#include "common.h"

#include "B2SRenderer.h"
#include "B2SServer.h"

#include <vector>
#include <algorithm>

namespace B2S {

MSGPI_BOOL_VAL_SETTING(showGrillProp, "ShowGrill", "Show Grill", "Show Grill", true, false);

B2SRenderer::B2SRenderer(const MsgPluginAPI* const msgApi, const unsigned int endpointId, std::shared_ptr<B2STable> b2s)
   : m_b2s(b2s)
   , m_msgApi(msgApi)
   , m_endpointId(endpointId)
   , m_resURIResolver(*msgApi, endpointId, true, false, false, false)
   , m_scoreViewDmdOverlay(m_resURIResolver, m_dmdTex, m_b2s->m_dmdImage.m_image)
   , m_backglassDmdOverlay(m_resURIResolver, m_dmdTex,
        m_b2s->m_backglassImage.m_image         ? m_b2s->m_backglassImage.m_image
           : m_b2s->m_backglassOffImage.m_image ? m_b2s->m_backglassOffImage.m_image
                                                : m_b2s->m_backglassOnImage.m_image)
{
   m_scoreViewDmdOverlay.LoadSettings(true);

   const VPXTextureInfo* dmdTexInfo = nullptr;
   if (m_b2s->m_dmdImage.m_image)
      dmdTexInfo = GetTextureInfo(m_b2s->m_dmdImage.m_image);
   m_dmdWidth = dmdTexInfo ? static_cast<float>(dmdTexInfo->width) : 1024.f;
   m_dmdHeight = dmdTexInfo ? static_cast<float>(dmdTexInfo->height) : 768.f;

   m_getDevSrcMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_GET_SRC_MSG);
   m_onDevChangedMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_ON_SRC_CHG_MSG);
   m_msgApi->SubscribeMsg(m_endpointId, m_onDevChangedMsgId, OnDevSrcChanged, this);
   OnDevSrcChanged(m_onDevChangedMsgId, this, nullptr);

   m_getSegSrcMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_GET_SRC_MSG);
   m_onSegChangedMsgId = m_msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_SEG_ON_SRC_CHG_MSG);
   m_msgApi->SubscribeMsg(m_endpointId, m_onSegChangedMsgId, OnSegSrcChanged, this);
   OnSegSrcChanged(m_onSegChangedMsgId, this, nullptr);
}

B2SRenderer::~B2SRenderer()
{
   m_msgApi->UnsubscribeMsg(m_onDevChangedMsgId, OnDevSrcChanged);
   m_msgApi->ReleaseMsgID(m_onDevChangedMsgId);
   m_msgApi->ReleaseMsgID(m_getDevSrcMsgId);
   delete[] m_deviceStateSrc.deviceDefs;

   m_msgApi->UnsubscribeMsg(m_onSegChangedMsgId, OnSegSrcChanged);
   m_msgApi->ReleaseMsgID(m_onSegChangedMsgId);
   m_msgApi->ReleaseMsgID(m_getSegSrcMsgId);
   m_segDisplays.clear();
}

void B2SRenderer::RegisterSettings(const MsgPluginAPI* const msgApi, unsigned int endpointId)
{
   msgApi->RegisterSetting(endpointId, &showGrillProp);
}

void B2SRenderer::OnSegSrcChanged(const unsigned int, void* userData, void*)
{
   auto me = static_cast<B2SRenderer*>(userData);
   me->m_segDisplays.clear();

   unsigned int pinmameEndpoint = me->m_msgApi->GetPluginEndpoint("PinMAME");
   if (pinmameEndpoint == 0)
      return;

   GetSegSrcMsg getSrcMsg = { 0, 0, nullptr };
   me->m_msgApi->SendMsg(me->m_endpointId, me->m_getSegSrcMsgId, pinmameEndpoint, &getSrcMsg);
   vector<SegSrcId> entries(getSrcMsg.count);
   getSrcMsg = { getSrcMsg.count, 0, entries.data() };
   me->m_msgApi->SendMsg(me->m_endpointId, me->m_getSegSrcMsgId, pinmameEndpoint, &getSrcMsg);
   for (unsigned int i = 0; i < getSrcMsg.count; i++)
   {
      if (getSrcMsg.entries[i].id.endpointId == pinmameEndpoint)
      {
         me->m_segDisplays.push_back(getSrcMsg.entries[i]);
      }
   }
}

void B2SRenderer::OnDevSrcChanged(const unsigned int, void* userData, void*)
{
   auto me = static_cast<B2SRenderer*>(userData);
   delete[] me->m_deviceStateSrc.deviceDefs;
   memset(&me->m_deviceStateSrc, 0, sizeof(me->m_deviceStateSrc));
   me->m_nSolenoids = 0;
   me->m_GIIndex = -1;
   me->m_nGIs = 0;
   me->m_lampIndex = -1;
   me->m_nLamps = 0;
   me->m_nMechs = 0;
   if (me->m_b2s->m_backglassOnImage.m_image)
      me->m_b2s->m_backglassOnImage.m_romUpdater = []() { /* No ROM source */ };
   for (auto& bulb : me->m_b2s->m_backglassIlluminations)
      bulb->m_romUpdater = []() { /* No ROM source */ };

   unsigned int pinmameEndpoint = me->m_msgApi->GetPluginEndpoint("PinMAME");
   if (pinmameEndpoint == 0)
      return;

   GetDevSrcMsg getSrcMsg = { 0, 0, nullptr };
   me->m_msgApi->SendMsg(me->m_endpointId, me->m_getDevSrcMsgId, pinmameEndpoint, &getSrcMsg);
   vector<DevSrcId> entries(getSrcMsg.count);
   getSrcMsg = { getSrcMsg.count, 0, entries.data() };
   me->m_msgApi->SendMsg(me->m_endpointId, me->m_getDevSrcMsgId, pinmameEndpoint, &getSrcMsg);
   for (unsigned int i = 0; i < getSrcMsg.count; i++)
   {
      if (getSrcMsg.entries[i].id.endpointId == pinmameEndpoint)
      {
         me->m_deviceStateSrc = getSrcMsg.entries[i];
         if (getSrcMsg.entries[i].deviceDefs)
         {
            me->m_deviceStateSrc.deviceDefs = new DeviceDef[getSrcMsg.entries[i].nDevices];
            memcpy(me->m_deviceStateSrc.deviceDefs, getSrcMsg.entries[i].deviceDefs, getSrcMsg.entries[i].nDevices * sizeof(DeviceDef));
         }
         break;
      }
   }

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

   if (me->m_b2s->m_backglassOnImage.m_image)
      me->m_b2s->m_backglassOnImage.m_romUpdater = me->ResolveRomPropUpdater(&me->m_b2s->m_backglassOnImage.m_brightness, me->m_b2s->m_backglassOnImage.m_romIdType, me->m_b2s->m_backglassOnImage.m_romId);

   for (auto& bulb : me->m_b2s->m_backglassIlluminations)
      switch (bulb->m_snippitType)
      {
      case B2SSnippitType::StandardImage: bulb->m_romUpdater = me->ResolveRomPropUpdater(&bulb->m_brightness, bulb->m_romIdType, bulb->m_romId); break;
      case B2SSnippitType::MechRotatingImage: bulb->m_romUpdater = me->ResolveRomPropUpdater(&bulb->m_mechRot, bulb->m_romIdType, bulb->m_romId); break;
      case B2SSnippitType::SelfRotatingImage: break;
      }
}

std::function<void()> B2SRenderer::ResolveRomPropUpdater(float* value, const B2SRomIDType romIdType, const int romId, const bool romInverted) const
{
   if (m_deviceStateSrc.deviceDefs == nullptr)
      return []() { /* No ROM source */ };
   switch (romIdType)
   {
   case B2SRomIDType::NotDefined:
      break;

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
      break;
   }
   return []() { /* No ROM source */ };
}

bool B2SRenderer::Render(VPXRenderContext2D* ctx, B2SServer* server)
{
   switch (ctx->window)
   {
   case VPXWindowId::VPXWINDOW_Backglass: return RenderBackglass(ctx, server);
   case VPXWindowId::VPXWINDOW_ScoreView: return RenderScoreView(ctx, server);
   default: return false;
   }
}

void B2SRenderer::RenderBulbs(VPXRenderContext2D* ctx, B2SServer* server, const vector<std::unique_ptr<B2SBulb>>& bulbs)
{
   for (const auto& bulb : bulbs)
   {
      if (bulb->m_b2sId >= 0 && server)
      {
         bulb->m_brightness = server->GetState(bulb->m_b2sId);
      }
      else
      {
         bulb->m_romUpdater();
      }
      float rotation = 0.f;
      if (bulb->m_snippitType == B2SSnippitType::MechRotatingImage)
         rotation = 360.f * (bulb->m_mechRot / static_cast<float>(bulb->m_snippitRotatingSteps));
      if (bulb->m_offImage && bulb->m_brightness < 1.f)
      {
         const VPXTextureInfo* const bulbTex = GetTextureInfo(bulb->m_offImage);
         ctx->DrawImage(ctx, bulb->m_offImage, bulb->m_lightColor.x, bulb->m_lightColor.y, bulb->m_lightColor.z, 1.f,
            0.f, 0.f, static_cast<float>(bulbTex->width), static_cast<float>(bulbTex->height),
            static_cast<float>(bulbTex->width) * 0.5f, static_cast<float>(bulbTex->height) * 0.5f, rotation,
            static_cast<float>(bulb->m_locationX), static_cast<float>(bulb->m_locationY), static_cast<float>(bulb->m_width), static_cast<float>(bulb->m_height));
      }
      if (bulb->m_image)
      {
         const VPXTextureInfo* const bulbTex = GetTextureInfo(bulb->m_image);
         ctx->DrawImage(ctx, bulb->m_image, bulb->m_lightColor.x, bulb->m_lightColor.y, bulb->m_lightColor.z, bulb->m_brightness,
            0.f, 0.f, static_cast<float>(bulbTex->width), static_cast<float>(bulbTex->height),
            static_cast<float>(bulbTex->width) * 0.5f, static_cast<float>(bulbTex->height) * 0.5f, rotation,
            static_cast<float>(bulb->m_locationX), static_cast<float>(bulb->m_locationY), static_cast<float>(bulb->m_width), static_cast<float>(bulb->m_height));
      }
   }
}

void B2SRenderer::RenderScores(VPXRenderContext2D* ctx, B2SServer* server, const B2SScores& scores)
{
   if (server == nullptr)
      return;

   vector<SegElementType> segTypes;
   vector<float> luminances;
   vector<VPXSegDisplayRenderStyle> styles;
   vector<VPXSegDisplayHint> hints;
   int digitIndex = 1;
   for (const auto& display : m_segDisplays)
   {
      SegDisplayFrame state = display.GetState(display.id);
      for (unsigned int i = 0; i < display.nElements; i++)
      {
         VPXSegDisplayHint hint = VPXSegDisplayHint::Generic;
         VPXSegDisplayRenderStyle style = VPXSegDisplayRenderStyle::VPXSegStyle_Plasma;
         if ((display.hardware & CTLPI_SEG_HARDWARE_FAMILY_MASK) == CTLPI_SEG_HARDWARE_NEON_PLASMA)
            style = VPXSegDisplayRenderStyle::VPXSegStyle_Plasma;
         else if ((display.hardware & CTLPI_SEG_HARDWARE_FAMILY_MASK) == CTLPI_SEG_HARDWARE_VFD_GREEN)
            style = VPXSegDisplayRenderStyle::VPXSegStyle_GreenVFD;
         else if ((display.hardware & CTLPI_SEG_HARDWARE_FAMILY_MASK) == CTLPI_SEG_HARDWARE_VFD_BLUE)
         {
            style = VPXSegDisplayRenderStyle::VPXSegStyle_BlueVFD;
            if (display.hardware == CTLPI_SEG_HARDWARE_GTS1_4DIGIT //
               || display.hardware == CTLPI_SEG_HARDWARE_GTS1_6DIGIT //
               || display.hardware == CTLPI_SEG_HARDWARE_GTS80A_7DIGIT //
               || display.hardware == CTLPI_SEG_HARDWARE_GTS80B_20DIGIT //
            )
               hint = VPXSegDisplayHint::Gottlieb;
         }
         segTypes.push_back(display.elementType[i]);
         styles.push_back(style);
         hints.push_back(hint);

         int bitState = 0;
         for (int j = 0; j < 16; j++)
         {
            luminances.push_back(state.frame[i * 16 + j]);
            if (state.frame[i * 16 + j] > 0.5f)
               bitState |= 1 << j;
         }
         int ret;
         switch (bitState & ~0x80) // Remove the comma
         {
         // 7-segment stuff
         case 0x003F: ret = 0; break;
         case 0x0006: ret = 1; break;
         case 0x005B: ret = 2; break;
         case 0x004F: ret = 3; break;
         case 0x0066: ret = 4; break;
         case 0x006D: ret = 5; break;
         case 0x007D: ret = 6; break;
         case 0x0007: ret = 7; break;
         case 0x007F: ret = 8; break;
         case 0x006F: ret = 9; break;
         // Additional 10-segment stuff
         case 0x0300: ret = 1; break;
         case 0x007C: ret = 6; break;
         case 0x0067: ret = 9; break;
         // Default is empty
         default: ret = -1; break;
         }
         server->B2SSetScoreDigit(digitIndex, ret);
         digitIndex++;
      }
   }

   digitIndex = 1;
   for (const auto& reel : scores.m_scores)
   {
      // Skip digits located on the grill when the grill is hidden
      if (static_cast<float>(reel.m_locY) > ctx->srcHeight)
         continue;

      const float width = (static_cast<float>(reel.m_width) - 0.5f * static_cast<float>((reel.m_digits - 1) * reel.m_spacing)) / static_cast<float>(reel.m_digits);
      for (int i = 0; i < reel.m_digits; i++)
      {
         const float x = static_cast<float>(reel.m_locX) + static_cast<float>(i) * (width + 0.5f * static_cast<float>(reel.m_spacing));
         int digit = 0;
         int index = -1;
         if (reel.m_b2sPlayerNo != 0)
         {
            int score = abs(server->GetPlayerScore(reel.m_b2sPlayerNo));
            for (int j = 0; j < (reel.m_digits - 1 - i); ++j)
               score /= 10;
            digit = score % 10;
         }
         else
         {
            index = reel.m_b2sStartDigit > 0 ? (reel.m_b2sStartDigit + i) : digitIndex;
            digit = static_cast<int>(server->GetScoreDigit(index));
         }
         digitIndex++;

         switch (reel.m_scoreType)
         {
         case B2SScoreRenderer::LED:
            // Black Pyramid (Bally 1984)
         case B2SScoreRenderer::ImportedLED:
            // Apache! (Taito 1978)
            // LED & ImportedLED seem to be the same as reel but without animation
         case B2SScoreRenderer::Reel:
            // Volkan Steel and Metal (Original 2023), Hang Glider (Bally 1976) => Simple reels, no animations
            // ? => Simple reels with animations & sounds
            {
               const B2SReelImage* reelImage = m_b2s->m_reels.GetImage(reel.m_reelType, digit);
               if (reelImage && reelImage->m_image)
               {
                  const VPXTextureInfo* texInfo = GetTextureInfo(reelImage->m_image);
                  ctx->DrawImage(ctx, reelImage->m_image, 1.f, 1.f, 1.f, 1.f, //
                     0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height), //
                     0.f, 0.f, 0.f, // No rotation
                     x, static_cast<float>(reel.m_locY), width, static_cast<float>(reel.m_height));
               }
            }
            break;

         case B2SScoreRenderer::Dream7:
            // Asteroid Annie (Gottlieb 1980)
            {
               std::array<float, 16> brightness;
               SegElementType segType = SegElementType::CTLPI_SEG_LAYOUT_14;
               VPXSegDisplayRenderStyle style = VPXSegDisplayRenderStyle::VPXSegStyle_Plasma;
               VPXSegDisplayHint hint = VPXSegDisplayHint::Generic;
               if (index > 0 && index * 16 <= (int)luminances.size())
               {
                  segType = segTypes[index-1];
                  style = styles[index-1];
                  hint = hints[index-1];
                  memcpy(brightness.data(), luminances.data() + (index-1) * 16, 16 * sizeof(float));
               }
               ctx->DrawSegDisplay(ctx, style, hint,
                  // First layer: glass
                  nullptr, 1.f, 1.f, 1.f, 0.15f, // Glass texture, tint and roughness
                  0.f, 0.f, 0.f, 0.f, // Glass texture coordinates (inside overall glass texture, cut for each element)
                  0.f, 0.f, 0.f, // Glass lighting from room
                  // Second layer: emitter
                  segType, brightness.data(), // Segment emitter type and brightness array
                  1.f, 1.f, 1.f, 1.f, 1.f,  // Emitter tint, emitter brightness, emitter alpha
                  0.f, 0.f, 0.f, 0.f, // Emitter padding (from glass border)
                  // Render quad
                  x, ctx->srcHeight - static_cast<float>(reel.m_locY + reel.m_height), width, static_cast<float>(reel.m_height));
            }
            break;

         case B2SScoreRenderer::RenderedLED:
            // Did not find any backglass using this mode (very old mode superseeded by Dream7 ?)
            break;
         }
      }
   }
}


bool B2SRenderer::RenderBackglass(VPXRenderContext2D* ctx, B2SServer* server)
{
   // Update to latest settings state
   m_grillCut = showGrillProp_Get() ? 0.f : static_cast<float>(m_b2s->m_grillHeight);

   const VPXTextureInfo* bgTexInfo = nullptr;
   if (m_b2s->m_backglassImage.m_image)
      bgTexInfo = GetTextureInfo(m_b2s->m_backglassImage.m_image);
   else if (m_b2s->m_backglassOffImage.m_image)
      bgTexInfo = GetTextureInfo(m_b2s->m_backglassOffImage.m_image);
   m_b2sWidth = bgTexInfo ? static_cast<float>(bgTexInfo->width) : 1024.f;
   m_b2sHeight = (bgTexInfo ? static_cast<float>(bgTexInfo->height) : 768.f) - m_grillCut;

   m_backglassDmdOverlay.LoadSettings(false);

   ctx->srcWidth = m_b2sWidth;
   ctx->srcHeight = m_b2sHeight;

   // Update animations
   auto now = std::chrono::steady_clock::now();
   float elapsed = static_cast<float>((now - m_lastBackglassRenderTick).count()) / 1000000000.0f;
   m_lastBackglassRenderTick = now;
   for (auto& animation : m_b2s->m_backglassAnimations)
      animation.Update(elapsed); // TODO implement slowdown settings/props (scale elapsed)

   // Draw background
   m_b2s->m_backglassOnImage.m_romUpdater();
   if (m_b2s->m_backglassOnImage.m_image == nullptr || m_b2s->m_backglassOnImage.m_brightness < 1.f)
   {
      if (m_b2s->m_backglassImage.m_image)
      {
         const VPXTextureInfo* texInfo = GetTextureInfo(m_b2s->m_backglassImage.m_image);
         ctx->DrawImage(ctx, m_b2s->m_backglassImage.m_image, 1.f, 1.f, 1.f, 1.f,
            0.f, m_grillCut, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height) - m_grillCut,
            0.f, 0.f, 0.f, // No rotation
            0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height) - m_grillCut);
      }
      else if (m_b2s->m_backglassOffImage.m_image)
      {
         const VPXTextureInfo* texInfo = GetTextureInfo(m_b2s->m_backglassOffImage.m_image);
         ctx->DrawImage(ctx, m_b2s->m_backglassOffImage.m_image, 1.f, 1.f, 1.f, 1.f,
            0.f, m_grillCut, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height) - m_grillCut,
            0.f, 0.f, 0.f, // No rotation
            0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height) - m_grillCut);
      }
   }
   if (m_b2s->m_backglassOnImage.m_image)
   {
      const VPXTextureInfo* texInfo = GetTextureInfo(m_b2s->m_backglassOnImage.m_image);
      ctx->DrawImage(ctx, m_b2s->m_backglassOnImage.m_image, 1.f, 1.f, 1.f, m_b2s->m_backglassOnImage.m_brightness,
         0.f, m_grillCut, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height) - m_grillCut,
         0.f, 0.f, 0.f, // No rotation
         0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height) - m_grillCut);
   }

   // Draw illuminations, scores and DMD overlay
   RenderBulbs(ctx, server, m_b2s->m_backglassIlluminations);
   RenderScores(ctx, server, m_b2s->m_backglassScores);
   m_backglassDmdOverlay.Render(ctx);

   return true;
}

bool B2SRenderer::RenderScoreView(VPXRenderContext2D* ctx, B2SServer* server)
{
   if (m_b2s->m_dmdImage.m_image == nullptr && m_b2s->m_dmdIlluminations.empty())
      return false;

   // Update to latest settings state
   m_scoreViewDmdOverlay.LoadSettings(false);

   ctx->srcWidth = m_dmdWidth;
   ctx->srcHeight = m_dmdHeight;

   // Update animations
   auto now = std::chrono::steady_clock::now();
   float elapsed = static_cast<float>((now - m_lastDmdRenderTick).count()) / 1000000000.0f;
   m_lastDmdRenderTick = now;
   for (auto& animation : m_b2s->m_dmdAnimations)
      animation.Update(elapsed); // TODO implement slowdown settings/props (scale elapsed)

   // Draw background
   if (m_b2s->m_dmdImage.m_image)
      ctx->DrawImage(ctx, m_b2s->m_dmdImage.m_image, 1.f, 1.f, 1.f, 1.f,
         0.f, 0.f, m_dmdWidth, m_dmdHeight,
         0.f, 0.f, 0.f, // No rotation
         0.f, 0.f, m_dmdWidth, m_dmdHeight);

   // Draw illuminations, scores and DMD overlay
   RenderBulbs(ctx, server, m_b2s->m_dmdIlluminations);
   RenderScores(ctx, server, m_b2s->m_dmdScores);
   m_scoreViewDmdOverlay.Render(ctx);

   return true;
}

}
