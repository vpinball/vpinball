// license:GPLv3+

#include "common.h"

#include "B2SRenderer.h"
#include "B2SServer.h"

#include "pinmame/PinMAMEPlugin.h"

#include <vector>
#include <algorithm>

namespace B2S {

MSGPI_BOOL_VAL_SETTING(showGrillProp, "ShowGrill", "Show Grill", "Show Grill", true, false);

static const char* dualModeValues[] = { "Authentic", "Fantasy" };
MSGPI_ENUM_VAL_SETTING(dualModeProp, "DualMode", "Dual Mode", "Active mode for dual backglasses (authentic or fantasy)", true, 1, 2, dualModeValues, 1);

B2SRenderer::B2SRenderer(const MsgPluginAPI* const msgApi, const VPXPluginAPI* const vpxApi, const unsigned int endpointId, std::shared_ptr<B2STable> b2s)
   : m_b2s(b2s)
   , m_msgApi(msgApi)
   , m_endpointId(endpointId)
   , m_resURIResolver(*msgApi, endpointId, true, false, false)
   , m_scoreViewDmdOverlay(vpxApi, m_resURIResolver, m_dmdTex, m_b2s->m_dmdImage.m_image)
   , m_backglassDmdOverlay(vpxApi, m_resURIResolver, m_dmdTex,
        m_b2s->m_backglassImage.m_image         ? m_b2s->m_backglassImage.m_image
           : m_b2s->m_backglassOffImage.m_image ? m_b2s->m_backglassOffImage.m_image
                                                : m_b2s->m_backglassOnImage.m_image)
   , m_pinmameControllers(
        msgApi, endpointId, CTLPI_CONTROLLERS_GET_MSG, CTLPI_CONTROLLERS_ON_CHG_MSG,
        [](std::vector<ControllerDef>& items)
        {
           const string pinmamePrefix(PMPI_GAMEID_PREFIX);
           std::erase_if(items, [&pinmamePrefix](const ControllerDef& src) { return !string(src.gameId).starts_with(pinmamePrefix); });
        },
        nullptr, [this]() { m_stateSources.Refresh(); m_segSources.Refresh(); })
   , m_segSources(
        msgApi, endpointId, CTLPI_SEG_GET_SRC_MSG, CTLPI_SEG_ON_SRC_CHG_MSG,
        [this](std::vector<SegSrcId>& items)
        {
           m_pinmameControllers.With(
              [&items](const std::vector<ControllerDef>& controllers)
              {
                 std::erase_if(items,
                    [&controllers](const SegSrcId& source)
                    {
                       return std::find_if(controllers.begin(), controllers.end(), [source](const ControllerDef& ctrl) { return ctrl.endpointId == source.id.endpointId; })
                          == controllers.end();
                    });
              });
        },
        nullptr, nullptr)
   , m_stateSources(
        msgApi, endpointId, CTLPI_STATE_GET_SRC_MSG, CTLPI_STATE_ON_SRC_CHG_MSG,
        [this](std::vector<StateSrcId>& items)
        {
           m_pinmameControllers.With(
              [&items](const std::vector<ControllerDef>& controllers)
              {
                 std::erase_if(items,
                    [&controllers](const StateSrcId& source)
                    {
                       return std::find_if(controllers.begin(), controllers.end(), [source](const ControllerDef& ctrl) { return ctrl.endpointId == source.id.endpointId; })
                          == controllers.end();
                    });
              });
        },
        [this]() { m_stateSources.With([this](const std::vector<StateSrcId>& items) { OnStateSrcChanged({ }); }); },
        [this]() { m_stateSources.With([this](const std::vector<StateSrcId>& items) { OnStateSrcChanged(items); }); })
{
   m_backglassDmdOverlay.LoadSettings(false);
   m_scoreViewDmdOverlay.LoadSettings(true);

   const VPXTextureInfo* dmdTexInfo = nullptr;
   if (m_b2s->m_dmdImage.m_image)
      dmdTexInfo = GetTextureInfo(m_b2s->m_dmdImage.m_image);
   m_dmdWidth = dmdTexInfo ? static_cast<float>(dmdTexInfo->width) : 1024.f;
   m_dmdHeight = dmdTexInfo ? static_cast<float>(dmdTexInfo->height) : 768.f;

   m_segSources.Subscribe();
   m_stateSources.Subscribe();
   m_pinmameControllers.Subscribe();
}

B2SRenderer::~B2SRenderer()
{
   m_segSources.Unsubscribe();
   m_stateSources.Unsubscribe();
   m_pinmameControllers.Unsubscribe();
}

void B2SRenderer::RegisterSettings(const MsgPluginAPI* const msgApi, unsigned int endpointId)
{
   msgApi->RegisterSetting(endpointId, &showGrillProp);
   msgApi->RegisterSetting(endpointId, &dualModeProp);
}

void B2SRenderer::OnStateSrcChanged(const std::vector<StateSrcId>& items)
{
   if (m_b2s->m_backglassOnImage.m_image)
      m_b2s->m_backglassOnImage.m_romUpdater = []() { /* No ROM source */ };
   for (auto& bulb : m_b2s->m_backglassIlluminations)
      bulb->m_romUpdater = []() { /* No ROM source */ };


   if (m_b2s->m_backglassOnImage.m_image)
      m_b2s->m_backglassOnImage.m_romUpdater
         = ResolveRomPropUpdater(items, &m_b2s->m_backglassOnImage.m_brightness, m_b2s->m_backglassOnImage.m_romIdType, m_b2s->m_backglassOnImage.m_romId, m_b2s->m_backglassOnImage.m_romInverted);

   for (auto& bulb : m_b2s->m_backglassIlluminations)
      switch (bulb->m_snippitType)
      {
      case B2SSnippitType::StandardImage: bulb->m_romUpdater = ResolveRomPropUpdater(items, &bulb->m_brightness, bulb->m_romIdType, bulb->m_romId, bulb->m_romInverted, bulb->m_b2sValue); break;
      case B2SSnippitType::MechRotatingImage: bulb->m_romUpdater = ResolveRomPropUpdater(items, &bulb->m_mechRot, bulb->m_romIdType, bulb->m_romId); break;
      case B2SSnippitType::SelfRotatingImage:
         bulb->m_romUpdater = ResolveRomPropUpdater(items, &bulb->m_romOn, bulb->m_romIdType, bulb->m_romId, bulb->m_romInverted, bulb->m_b2sValue);
         break;
      }

   // Reel illumination lamps (ReelIlluB2SID is always a lamp in the reference implementation)
   for (auto* scores : { &m_b2s->m_backglassScores, &m_b2s->m_dmdScores })
      for (auto& score : scores->m_scores)
         score.m_reelIlluUpdater = score.m_reelIlluB2SID > 0 ? ResolveRomPropUpdater(items, &score.m_reelIllu, B2SRomIDType::Lamp, score.m_reelIlluB2SID) : []() { };

   // Rebind the ROM event triggers of all animations (IDJoin)
   const auto resolver = [this, &items](B2SRomIDType romIdType, int romId, bool inverted, float* target) { return ResolveRomPropUpdater(items, target, romIdType, romId, inverted); };
   for (auto& animation : m_b2s->m_backglassAnimations)
      animation.BindRomTriggers(resolver);
   for (auto& animation : m_b2s->m_dmdAnimations)
      animation.BindRomTriggers(resolver);
}

std::function<void()> B2SRenderer::ResolveRomPropUpdater(const std::vector<StateSrcId>& items, float* value, const B2SRomIDType romIdType, const int romId, const bool romInverted, const int romValue) const
{
   int groupId;
   switch (romIdType)
   {
   case B2SRomIDType::Solenoid: groupId = PMPI_GROUP_SOLENOID; break;
   case B2SRomIDType::GIString: groupId = PMPI_GROUP_GI; break;
   case B2SRomIDType::Lamp: groupId = PMPI_GROUP_LAMP; break;
   case B2SRomIDType::Mech: groupId = PMPI_GROUP_MECH; break;
   default: return []() { /* No ROM source */ };
   }
   for (const StateSrcId& src : items)
   {
      if (src.id.resId != groupId)
         continue;
      for (unsigned int i = 0; i < src.nStates; i++)
      {
         const auto& def = src.stateDefs[i];
         if (def.mappingId == romId && def.dataFormat == CTLPI_STATE_FORMAT_FLOAT && def.GetState)
         {
            return [this, value, def, romInverted, romValue]()
            {
               m_stateSources.With([this, value, &def](const std::vector<StateSrcId>&) { def.GetState(def.callContext, value); });
               if (romValue > 0)
                  *value = (static_cast<int>(*value) == romValue) ? 1.f : 0.f;
               else if (romInverted)
                  *value = 1.f - *value;
            };
         }
      }
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

B2SDualMode B2SRenderer::ActiveDualMode() const { return m_b2s->m_dualBackglass ? static_cast<B2SDualMode>(dualModeProp_Get()) : B2SDualMode::Both; }

void B2SRenderer::UpdateAnimations(vector<B2SAnimation>& animations, float elapsed, B2SServer* server, B2SDualMode dualMode)
{
   B2SAnimationEffects fx;
   if (server)
      fx = server->GetAnimationEffects();
   fx.dualMode = dualMode;
   fx.randomTrigger = [this, &animations](B2SRomIDType romIdType, int romId, bool start, B2SAnimation*) { OnRandomAnimationTrigger(animations, romIdType, romId, start); };
   for (auto& animation : animations)
   {
      // Animation slow downs from the plugin settings (AllAnimationSlowDown takes precedence over per-animation)
      float scaledElapsed = elapsed;
      if (server)
      {
         int slowDown = server->GetAllAnimationSlowDown();
         if (slowDown <= 1)
            slowDown = server->GetAnimationSlowDown(animation.m_name);
         if (slowDown > 1)
            scaledElapsed /= static_cast<float>(slowDown);
      }
      animation.Update(scaledElapsed, fx);
   }
}

// Script B2SSetData writes in the lamp-id space trigger animations bound to that lamp through IDJoin
// (mirrors the reference's MyB2SSetData which treats the B2SID space as lamp space)
void B2SRenderer::DispatchScriptTriggers(B2SServer* server)
{
   for (const auto& [lampId, value] : server->DrainAnimationTriggers())
      for (auto* animations : { &m_b2s->m_backglassAnimations, &m_b2s->m_dmdAnimations })
         for (auto& animation : *animations)
            for (const auto& trigger : animation.GetRomTriggers())
               if (trigger.romIdType == B2SRomIDType::Lamp && trigger.romId == lampId)
               {
                  const bool start = (value != 0) != trigger.inverted;
                  if (animation.m_randomStart)
                     OnRandomAnimationTrigger(*animations, B2SRomIDType::Lamp, lampId, start);
                  else if (start)
                     animation.Start();
                  else
                     animation.Stop();
                  break;
               }
}

void B2SRenderer::OnRandomAnimationTrigger(vector<B2SAnimation>& animations, B2SRomIDType romIdType, int romId, bool start)
{
   if (start)
   {
      // Pick one animation at random among all the random-start animations sharing this ROM trigger,
      // weighted by RandomQuality (the reference adds the animation once per quality point to the pool)
      vector<B2SAnimation*> pool;
      int totalWeight = 0;
      for (B2SAnimation& animation : animations)
         if (animation.m_randomStart
            && std::ranges::any_of(
               animation.GetRomTriggers(), [romIdType, romId](const B2SAnimation::RomTrigger& trigger) { return trigger.romIdType == romIdType && trigger.romId == romId; }))
         {
            pool.push_back(&animation);
            totalWeight += std::max(1, animation.m_randomQuality);
         }
      const bool anyRunning = std::ranges::any_of(pool, [](const B2SAnimation* animation) { return animation->IsRunning(); });
      if (!anyRunning && !pool.empty())
      {
         int pick = std::rand() % totalWeight;
         for (B2SAnimation* animation : pool)
         {
            pick -= std::max(1, animation->m_randomQuality);
            if (pick < 0)
            {
               m_lastRandomAnimation = animation;
               break;
            }
         }
         m_lastRandomAnimation->Start(false);
      }
   }
   else if (m_lastRandomAnimation != nullptr)
   {
      m_lastRandomAnimation->Stop();
      m_lastRandomAnimation = nullptr;
   }
}

void B2SRenderer::RenderBulbs(VPXRenderContext2D* ctx, const B2SServer* server, const vector<std::unique_ptr<B2SBulb>>& bulbs, float elapsed, B2SDualMode dualMode)
{
   for (const auto& bulb : bulbs)
   {
      if (dualMode != B2SDualMode::Both && bulb->m_dualMode != B2SDualMode::Both && bulb->m_dualMode != dualMode)
         continue; // Dual backglass: only render bulbs matching the active mode
      if (bulb->m_bakedIntoBackground)
         continue; // Drawn as part of the lit background image
      const bool locked = server && !bulb->m_name.empty() && server->IsIlluminationLocked(bulb->m_name);
      float state = 0.f;
      const bool scripted = server && server->GetBulbState(*bulb, state);
      if (scripted)
         bulb->m_brightness = (bulb->m_b2sValue > 0) ? //
            ((static_cast<int>(state) == bulb->m_b2sValue) ? 1.f : 0.f)
                                                     : state;
      else if (!locked) // While the illumination group is locked by an animation, ROM updates are suspended
         bulb->m_romUpdater();
      float rotation = 0.f;
      if (bulb->m_snippitType == B2SSnippitType::MechRotatingImage)
         rotation = 360.f * (bulb->m_mechRot / static_cast<float>(bulb->m_snippitRotatingSteps));
      else if (bulb->m_snippitType == B2SSnippitType::SelfRotatingImage)
      {
         // Self rotating images spin while their driving state is on; scripted state wins over the ROM
         // lamp state (same as illumination). Rotation only toggles on state transitions so that explicit
         // B2SStartRotation calls keep spinning until B2SStopRotation or a state write (reference behaviour).
         const float spin = scripted ? bulb->m_brightness : bulb->m_romOn;
         const bool spinOn = spin >= 0.5f;
         if (spinOn != bulb->m_spinDriverOn)
         {
            bulb->m_spinDriverOn = spinOn;
            if (spinOn)
               bulb->StartRotation();
            else
               bulb->StopRotation();
         }
         if (!scripted)
            bulb->m_brightness = std::max(bulb->m_brightness, bulb->m_romOn);
         bulb->UpdateRotation(elapsed);
         rotation = bulb->GetRotationAngle();
      }
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

void B2SRenderer::RenderScores(VPXRenderContext2D* ctx, B2SServer* server, const B2SScores& scores, float elapsed)
{
   if (server == nullptr)
      return;
   // B2SHideScoreDisplays/animations only hide LED score displays, reel displays keep rendering
   const bool ledDisplaysHidden = server->AreScoreDisplaysHidden();

   vector<SegElementType> segTypes;
   vector<float> luminances;
   vector<VPXSegDisplayRenderStyle> styles;
   vector<VPXSegDisplayHint> hints;
   m_segSources.With([&segTypes, &styles, &hints, &luminances, server](const std::vector<SegSrcId>& segDisplays){
      int digitIndex = 1;
      for (const auto& display : segDisplays)
      {
         SegDisplayFrame state = display.GetState(display.callContext);
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
   });

   // Build the per-player score strings: right aligned, space padded on the left, then split
   // across the player displays in file order (each display consuming its digit count)
   ankerl::unordered_dense::map<int, string> playerScoreText;
   ankerl::unordered_dense::map<int, size_t> playerScoreOffset;
   for (const auto& score : scores.m_scores)
      if (score.m_b2sPlayerNo > 0)
         playerScoreText[score.m_b2sPlayerNo].append(static_cast<size_t>(std::max(0, score.m_digits)), ' ');
   for (auto& [player, text] : playerScoreText)
   {
      const string scoreText = std::to_string(abs(server->GetPlayerScore(player)));
      if (scoreText.length() >= text.length())
         text = scoreText.substr(scoreText.length() - text.length());
      else
         text.replace(text.length() - scoreText.length(), scoreText.length(), scoreText);
   }

   for (const auto& reel : scores.m_scores)
   {
      // Skip hidden displays (DisplayState=1), and LED displays when score displays are hidden
      if (reel.IsHidden() || (ledDisplaysHidden && (reel.m_scoreType == B2SScoreRenderer::Dream7 || reel.m_scoreType == B2SScoreRenderer::RenderedLED)))
         continue;

      // Skip digits located on the grill when the grill is hidden
      if (static_cast<float>(reel.m_locY) > ctx->srcHeight)
      {
         if (reel.m_b2sPlayerNo != 0)
            playerScoreOffset[reel.m_b2sPlayerNo] += static_cast<size_t>(std::max(0, reel.m_digits));
         continue;
      }

      const float width = (static_cast<float>(reel.m_width) - 0.5f * static_cast<float>((reel.m_digits - 1) * reel.m_spacing)) / static_cast<float>(reel.m_digits);

      // Reel rolling: resolve the rolling direction, the illuminated image selection and the per-digit
      // hold flags. A digit waits while a less significant digit has a pending 9->0 / 0->9 wrap (carry).
      const bool rollUp = scores.m_reelRollingDirection != B2SReelRollingDirection::Down;
      // ReelIlluB2SID is in lamp-id space: script B2SSetData writes take precedence, else the ROM lamp state applies
      float illuState = reel.m_reelIllu;
      if (server == nullptr || !server->GetScriptedLampState(reel.m_reelIlluB2SID, illuState))
      {
         reel.m_reelIlluUpdater();
         illuState = reel.m_reelIllu;
      }
      const bool illuminated = reel.m_reelIlluB2SID > 0 && (reel.m_reelIlluB2SValue > 0 ? (static_cast<int>(illuState) == reel.m_reelIlluB2SValue) : (illuState != 0.f));
      vector<char> hold(static_cast<size_t>(std::max(0, reel.m_digits)), 0);
      bool lowerWrapPending = false;
      for (int i = reel.m_digits - 1; i >= 0; i--)
      {
         hold[static_cast<size_t>(i)] = lowerWrapPending ? 1 : 0;
         if (const auto it = m_reelDigits.find(reel.m_resolvedStartDigit + i); it != m_reelDigits.end() && it->second.HasPendingWrap(rollUp))
            lowerWrapPending = true;
      }

      for (int i = 0; i < reel.m_digits; i++)
      {
         const float x = static_cast<float>(reel.m_locX) + static_cast<float>(i) * (width + 0.5f * static_cast<float>(reel.m_spacing));
         int digit = 0;
         const int index = reel.m_resolvedStartDigit + i;
         if (reel.m_b2sPlayerNo != 0)
         {
            const string& text = playerScoreText[reel.m_b2sPlayerNo];
            const size_t offset = playerScoreOffset[reel.m_b2sPlayerNo] + static_cast<size_t>(i);
            const char c = offset < text.length() ? text[offset] : ' ';
            const int raw = (c >= '0' && c <= '9') ? c - '0' : -1;
            digit = (reel.m_scoreType == B2SScoreRenderer::Dream7 || reel.m_scoreType == B2SScoreRenderer::RenderedLED) ? raw : (raw < 0 ? 0 : raw);
            if (i == reel.m_digits - 1)
               playerScoreOffset[reel.m_b2sPlayerNo] += static_cast<size_t>(reel.m_digits);
         }
         else
            digit = server->GetScoreDigit(index);

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
               int shownDigit = digit;
               int intermediate = 0;
               B2SReelDigit& st = m_reelDigits[index];
               const B2SReelImage* cur = m_b2s->m_reels.GetImage(reel.m_reelType, st.Current(), illuminated, reel.m_reelIlluImageSet);
               const int intermediates = cur ? cur->m_countOfIntermediate : 0;
               st.SetIlluminated(illuminated, intermediates);
               // Player displays are updated from decoded pinmame frames so they always roll;
               // script driven displays roll when B2SSetScore/B2SSetReel raised the roll flag.
               // LED image displays never roll (B2SReelBox::isLED).
               st.SetTarget(digit, reel.m_scoreType == B2SScoreRenderer::Reel && (reel.m_b2sPlayerNo != 0 || server->ConsumeScoreDigitRoll(index)));
               if (!hold[static_cast<size_t>(i)] && st.Update(elapsed, scores.m_reelRollingInterval, intermediates, rollUp))
               {
                  // Reel roll sound: per-digit Sound1..N attribute, or the display wide Sound attribute
                  const string& soundName
                     = (i < static_cast<int>(reel.m_soundNames.size()) && !reel.m_soundNames[static_cast<size_t>(i)].empty()) ? reel.m_soundNames[static_cast<size_t>(i)] : reel.m_soundName;
                  if (!soundName.empty() && soundName != "stille"sv)
                     server->B2SPlaySound(soundName);
               }
               shownDigit = st.Current();
               intermediate = st.Intermediate();
               const B2SReelImage* reelImage = m_b2s->m_reels.GetImage(reel.m_reelType, shownDigit, illuminated, reel.m_reelIlluImageSet);
               const VPXTexture texture = (reelImage && intermediate > 0 && intermediate <= static_cast<int>(reelImage->m_intermediates.size()))
                  ? reelImage->m_intermediates[static_cast<size_t>(intermediate - 1)]
                  : nullptr;
               if (reelImage && (texture != nullptr || reelImage->m_image != nullptr))
               {
                  const VPXTexture img = texture != nullptr ? texture : reelImage->m_image;
                  const VPXTextureInfo* texInfo = GetTextureInfo(img);
                  ctx->DrawImage(ctx, img, 1.f, 1.f, 1.f, 1.f, //
                     0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height), //
                     0.f, 0.f, 0.f, // No rotation
                     x, static_cast<float>(reel.m_locY), width, static_cast<float>(reel.m_height));
               }
            }
            break;

         case B2SScoreRenderer::Dream7:
            // Asteroid Annie (Gottlieb 1980)
         case B2SScoreRenderer::RenderedLED:
         {
            std::array<float, 16> brightness;
            // Segment layout declared by the ReelType suffix (Dream7LEDx/RenderedLEDx)
            SegElementType segType = reel.m_ledSegments == 7 ? SegElementType::CTLPI_SEG_LAYOUT_7
               : reel.m_ledSegments == 10                    ? SegElementType::CTLPI_SEG_LAYOUT_9
               : reel.m_ledSegments == 14                    ? SegElementType::CTLPI_SEG_LAYOUT_14
                                                             : SegElementType::CTLPI_SEG_LAYOUT_14;
            VPXSegDisplayRenderStyle style = VPXSegDisplayRenderStyle::VPXSegStyle_Plasma;
            VPXSegDisplayHint hint = VPXSegDisplayHint::Generic;
            if (index > 0 && index * 16 <= (int)luminances.size())
            {
               segType = segTypes[index - 1];
               style = styles[index - 1];
               hint = hints[index - 1];
               memcpy(brightness.data(), luminances.data() + (index - 1) * 16, 16 * sizeof(float));
            }
            else
            {
               // Script driven digit: an explicit B2SSetLED segment mask takes precedence,
               // otherwise the value is mapped to its standard 7 segment pattern
               const int segMask = server->GetScoreDigitSegments(index);
               static constexpr uint16_t digitSegments[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };
               const uint16_t bits = segMask >= 0 ? static_cast<uint16_t>(segMask) : (digit >= 0 && digit < 10) ? digitSegments[digit] : 0;
               for (int j = 0; j < 16; j++)
                  brightness[j] = ((bits >> j) & 1) ? 1.f : 0.f;
               if (reel.m_scoreType == B2SScoreRenderer::RenderedLED)
                  style = VPXSegDisplayRenderStyle::VPXSegStyle_GenLED;
            }
            // UsedLEDType plugin setting forces Dream7 (2) or rendered LEDs (1) whatever the table declares
            const int usedLEDType = server->GetUsedLEDType();
            if (usedLEDType == 1)
               style = VPXSegDisplayRenderStyle::VPXSegStyle_GenLED;
            else if (usedLEDType == 2)
               style = VPXSegDisplayRenderStyle::VPXSegStyle_Plasma;
            ctx->DrawSegDisplay(ctx, style, hint,
               // First layer: glass tinted with the unlit segment color
               nullptr, reel.m_reelDarkColor.x, reel.m_reelDarkColor.y, reel.m_reelDarkColor.z, 0.15f, // Glass texture, tint and roughness
               0.f, 0.f, 0.f, 0.f, // Glass texture coordinates (inside overall glass texture, cut for each element)
               0.f, 0.f, 0.f, // Glass lighting from room
               // Second layer: emitter tinted with the lit segment color
               segType, brightness.data(), // Segment emitter type and brightness array
               reel.m_reelLitColor.x, reel.m_reelLitColor.y, reel.m_reelLitColor.z, 1.f, 1.f, // Emitter tint, emitter brightness, emitter alpha
               0.f, 0.f, 0.f, 0.f, // Emitter padding (from glass border)
               // Render quad
               x, ctx->srcHeight - static_cast<float>(reel.m_locY + reel.m_height), width, static_cast<float>(reel.m_height));
         }
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

   m_backglassDmdOverlay.SetEnableOverride(server->GetHideDMD());
   m_backglassDmdOverlay.LoadSettings(false);

   ctx->srcWidth = m_b2sWidth;
   ctx->srcHeight = m_b2sHeight;

   // Update animations
   auto now = std::chrono::steady_clock::now();
   float elapsed = static_cast<float>(static_cast<double>((now - m_lastBackglassRenderTick).count()) / 1000000000.0);
   m_lastBackglassRenderTick = now;
   const B2SDualMode dualMode = ActiveDualMode();
   DispatchScriptTriggers(server);
   UpdateAnimations(m_b2s->m_backglassAnimations, elapsed, server, dualMode);

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
   RenderBulbs(ctx, server, m_b2s->m_backglassIlluminations, elapsed, dualMode);
   RenderScores(ctx, server, m_b2s->m_backglassScores, elapsed);
   m_backglassDmdOverlay.Render(ctx);

   return true;
}

bool B2SRenderer::RenderScoreView(VPXRenderContext2D* ctx, B2SServer* server)
{
   if (m_b2s->m_dmdImage.m_image == nullptr && m_b2s->m_dmdIlluminations.empty())
      return false;
   if (server->GetHideB2SDMD() == 1) // HideB2SDMD plugin setting
      return false;

   // Update to latest settings state
   m_scoreViewDmdOverlay.SetEnableOverride(server->GetHideDMD());
   m_scoreViewDmdOverlay.LoadSettings(true);

   ctx->srcWidth = m_dmdWidth;
   ctx->srcHeight = m_dmdHeight;

   // Update animations
   auto now = std::chrono::steady_clock::now();
   float elapsed = static_cast<float>(static_cast<double>((now - m_lastDmdRenderTick).count()) / 1000000000.0);
   m_lastDmdRenderTick = now;
   DispatchScriptTriggers(server);
   UpdateAnimations(m_b2s->m_dmdAnimations, elapsed, server, ActiveDualMode());
   // Draw background
   if (m_b2s->m_dmdImage.m_image)
      ctx->DrawImage(ctx, m_b2s->m_dmdImage.m_image, 1.f, 1.f, 1.f, 1.f,
         0.f, 0.f, m_dmdWidth, m_dmdHeight,
         0.f, 0.f, 0.f, // No rotation
         0.f, 0.f, m_dmdWidth, m_dmdHeight);

   // Draw illuminations, scores and DMD overlay
   RenderBulbs(ctx, server, m_b2s->m_dmdIlluminations, elapsed, B2SDualMode::Both); // Reference does not dual-filter score view bulbs
   RenderScores(ctx, server, m_b2s->m_dmdScores, elapsed);
   m_scoreViewDmdOverlay.Render(ctx);

   return true;
}

}
