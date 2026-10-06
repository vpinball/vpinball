// license:GPLv3+

#include "common.h"

#include "B2SRenderer.h"
#include "B2SServer.h"

#include "pinmame/PinMAMEPlugin.h"

#include <vector>
#include <algorithm>

namespace B2S {

MSGPI_BOOL_VAL_SETTING(showGrillProp, "ShowGrill", "Show Grill", "Show Grill", true, false);

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
      case B2SSnippitType::SelfRotatingImage: bulb->m_romUpdater = ResolveRomPropUpdater(items, &bulb->m_romOn, bulb->m_romIdType, bulb->m_romId, bulb->m_romInverted); break;
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

void B2SRenderer::UpdateAnimations(vector<B2SAnimation>& animations, float elapsed, B2SServer* server)
{
   B2SAnimationEffects fx;
   if (server)
      fx = server->GetAnimationEffects();
   fx.randomTrigger = [this, &animations](B2SRomIDType romIdType, int romId, bool start, B2SAnimation*) { OnRandomAnimationTrigger(animations, romIdType, romId, start); };
   for (auto& animation : animations)
      animation.Update(elapsed, fx); // TODO implement slowdown settings/props (scale elapsed)
}

void B2SRenderer::OnRandomAnimationTrigger(vector<B2SAnimation>& animations, B2SRomIDType romIdType, int romId, bool start)
{
   if (start)
   {
      // Pick one animation at random among all the random-start animations sharing this ROM trigger
      vector<B2SAnimation*> pool;
      for (B2SAnimation& animation : animations)
         if (animation.m_randomStart
            && std::ranges::any_of(
               animation.GetRomTriggers(), [romIdType, romId](const B2SAnimation::RomTrigger& trigger) { return trigger.romIdType == romIdType && trigger.romId == romId; }))
            pool.push_back(&animation);
      const bool anyRunning = std::ranges::any_of(pool, [](const B2SAnimation* animation) { return animation->IsRunning(); });
      if (!anyRunning && !pool.empty())
      {
         int pick = pool.size() == 1 ? 0 : std::rand() % static_cast<int>(pool.size());
         if (pool.size() > 1 && pick == m_lastRandomPick)
            pick = (pick + 1) % static_cast<int>(pool.size()); // avoid restarting the same animation twice in a row
         m_lastRandomPick = pick;
         m_lastRandomAnimation = pool[pick];
         m_lastRandomAnimation->Start(false);
      }
   }
   else if (m_lastRandomAnimation != nullptr)
   {
      m_lastRandomAnimation->Stop();
      m_lastRandomAnimation = nullptr;
   }
}

void B2SRenderer::RenderBulbs(VPXRenderContext2D* ctx, const B2SServer* server, const vector<std::unique_ptr<B2SBulb>>& bulbs, float elapsed)
{
   for (const auto& bulb : bulbs)
   {
      const bool locked = server && !bulb->m_name.empty() && server->IsIlluminationLocked(bulb->m_name);
      float state = 0.f;
      if (server && server->GetBulbState(*bulb, state))
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
         // ROM-driven self rotating images rotate while their lamp is on (reference behaviour)
         if (bulb->m_romId >= 0 && bulb->m_romIdType != B2SRomIDType::NotDefined)
         {
            if (bulb->m_romOn >= 0.5f)
               bulb->StartRotation();
            else
               bulb->StopRotation();
            bulb->m_brightness = std::max(bulb->m_brightness, bulb->m_romOn);
         }
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
   if (server == nullptr || server->AreScoreDisplaysHidden())
      return;

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
      reel.m_reelIlluUpdater();
      const bool illuminated = reel.m_reelIlluB2SID > 0 && (reel.m_reelIlluB2SValue > 0 ? (static_cast<int>(reel.m_reelIllu) == reel.m_reelIlluB2SValue) : (reel.m_reelIllu != 0.f));
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
               else
               {
                  // Script driven digit: map value to its standard 7 segment pattern
                  static constexpr uint16_t digitSegments[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };
                  const uint16_t bits = (digit >= 0 && digit < 10) ? digitSegments[digit] : 0;
                  for (int j = 0; j < 16; j++)
                     brightness[j] = ((bits >> j) & 1) ? 1.f : 0.f;
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
   float elapsed = static_cast<float>(static_cast<double>((now - m_lastBackglassRenderTick).count()) / 1000000000.0);
   m_lastBackglassRenderTick = now;
   UpdateAnimations(m_b2s->m_backglassAnimations, elapsed, server);

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
   RenderBulbs(ctx, server, m_b2s->m_backglassIlluminations, elapsed);
   RenderScores(ctx, server, m_b2s->m_backglassScores, elapsed);
   m_backglassDmdOverlay.Render(ctx);

   return true;
}

bool B2SRenderer::RenderScoreView(VPXRenderContext2D* ctx, B2SServer* server)
{
   if (m_b2s->m_dmdImage.m_image == nullptr && m_b2s->m_dmdIlluminations.empty())
      return false;

   // Update to latest settings state
   m_scoreViewDmdOverlay.LoadSettings(true);

   ctx->srcWidth = m_dmdWidth;
   ctx->srcHeight = m_dmdHeight;

   // Update animations
   auto now = std::chrono::steady_clock::now();
   float elapsed = static_cast<float>(static_cast<double>((now - m_lastDmdRenderTick).count()) / 1000000000.0);
   m_lastDmdRenderTick = now;
   UpdateAnimations(m_b2s->m_dmdAnimations, elapsed, server);

   // Draw background
   if (m_b2s->m_dmdImage.m_image)
      ctx->DrawImage(ctx, m_b2s->m_dmdImage.m_image, 1.f, 1.f, 1.f, 1.f,
         0.f, 0.f, m_dmdWidth, m_dmdHeight,
         0.f, 0.f, 0.f, // No rotation
         0.f, 0.f, m_dmdWidth, m_dmdHeight);

   // Draw illuminations, scores and DMD overlay
   RenderBulbs(ctx, server, m_b2s->m_dmdIlluminations, elapsed);
   RenderScores(ctx, server, m_b2s->m_dmdScores, elapsed);
   m_scoreViewDmdOverlay.Render(ctx);

   return true;
}

}
