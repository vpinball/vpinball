// license:GPLv3+

#include "common.h"

#include "B2SRenderer.h"
#include "B2SServer.h"

#include "pinmame/PinMAMEPlugin.h"

#include <vector>
#include <algorithm>
#include <cmath>

namespace B2S {

// sRGB to linear conversion (duplicate from the main VPX codebase, see utils/color.h)
static inline float InvsRGB(const float x) { return (x <= 0.04045f) ? (x * (float)(1.0 / 12.92)) : (powf(x * (float)(1.0 / 1.055) + (float)(0.055 / 1.055), 2.4f)); }

MSGPI_BOOL_VAL_SETTING(showGrillProp, "ShowGrill", "Show Grill", "Show Grill", true, false);
MSGPI_BOOL_VAL_SETTING(upgradedSegRenderingProp, "UpgradedSegmentRendering", "Upgraded segment rendering",
   "Render LED score displays with the upgraded segment renderer (turn off for the original B2S segment rendering)", true, true);

static const char* dualModeValues[] = { "Authentic", "Fantasy" };
MSGPI_ENUM_VAL_SETTING(dualModeProp, "DualMode", "Dual Mode", "Active mode for dual backglasses (authentic or fantasy)", true, 1, 2, dualModeValues, 1);

B2SRenderer::B2SRenderer(const MsgPluginAPI* const msgApi, const VPXPluginAPI* const vpxApi, const unsigned int endpointId, std::shared_ptr<B2STable> b2s)
   : m_b2s(b2s)
   , m_msgApi(msgApi)
   , m_vpxApi(vpxApi)
   , m_endpointId(endpointId)
   , m_resURIResolver(*msgApi, endpointId, true, false, false)
   , m_pinmameControllers(
        msgApi, endpointId, CTLPI_CONTROLLERS_GET_MSG, CTLPI_CONTROLLERS_ON_CHG_MSG,
        [](std::vector<ControllerDef>& items)
        {
           const string pinmamePrefix(PMPI_GAMEID_PREFIX);
           std::erase_if(items, [&pinmamePrefix](const ControllerDef& src) { return !string(src.gameId).starts_with(pinmamePrefix); });
        },
        nullptr,
        [this]()
        {
           m_stateSources.Refresh();
           m_segSources.Refresh();
        })
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
        [this]() { m_stateSources.With([this](const std::vector<StateSrcId>& items) { OnStateSrcChanged({}); }); },
        [this]() { m_stateSources.With([this](const std::vector<StateSrcId>& items) { OnStateSrcChanged(items); }); })
{
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
   msgApi->RegisterSetting(endpointId, &upgradedSegRenderingProp);
}

namespace
{
   int MSGPIAPI ShowGrillGet(void*) { return showGrillProp_Get(); }
   void MSGPIAPI ShowGrillSet(void*, int v) { showGrillProp_Set(v); }
   int MSGPIAPI DualModeGet(void*) { return dualModeProp_Get(); }
   void MSGPIAPI DualModeSet(void*, int v) { dualModeProp_Set(v); }
   int MSGPIAPI UpgradedSegGet(void*) { return upgradedSegRenderingProp_Get(); }
   void MSGPIAPI UpgradedSegSet(void*, int v) { upgradedSegRenderingProp_Set(v); }
}

int B2SRenderer::FillSettingsItems(PagedUIItemDef* items)
{
   items[0] = {};
   items[0].type = PAGEDUI_ITEM_BOOL;
   items[0].id = "ShowGrill";
   items[0].label = "Show Grill";
   items[0].tooltip = "Show Grill";
   items[0].storage = PAGEDUI_STORAGE_HOST;
   items[0].boolProp = { 0, &ShowGrillGet, &ShowGrillSet };
   items[1] = {};
   items[1].type = PAGEDUI_ITEM_ENUM;
   items[1].id = "DualMode";
   items[1].label = "Dual Mode";
   items[1].tooltip = "Active mode for dual backglasses (authentic or fantasy)";
   items[1].storage = PAGEDUI_STORAGE_HOST;
   items[1].enumProp = { 1, 2, dualModeValues, 1, &DualModeGet, &DualModeSet };
   items[2] = {};
   items[2].type = PAGEDUI_ITEM_BOOL;
   items[2].id = "UpgradedSegmentRendering";
   items[2].label = "Upgraded segment rendering";
   items[2].tooltip = "Render LED score displays with the upgraded segment renderer (turn off for the original B2S segment rendering)";
   items[2].storage = PAGEDUI_STORAGE_HOST;
   items[2].boolProp = { 1, &UpgradedSegGet, &UpgradedSegSet };
   return 3;
}

void B2SRenderer::OnStateSrcChanged(const std::vector<StateSrcId>& items)
{
   if (m_b2s->m_backglassOnImage.m_image)
      m_b2s->m_backglassOnImage.m_romUpdater = []() { /* No ROM source */ };
   for (auto* bulbs : { &m_b2s->m_backglassIlluminations, &m_b2s->m_dmdIlluminations })
      for (auto& bulb : *bulbs)
         bulb->m_romUpdater = []() { /* No ROM source */ };


   // The reference forces TopRomInverted to False for the BackglassOnImage channel
   if (m_b2s->m_backglassOnImage.m_image)
      m_b2s->m_backglassOnImage.m_romUpdater
         = ResolveRomPropUpdater(items, &m_b2s->m_backglassOnImage.m_romProbe, m_b2s->m_backglassOnImage.m_romIdType, m_b2s->m_backglassOnImage.m_romId, false);

   // The reference only defines a matching value (B2SValue as RomIDValue) on B2SID bulbs; ROM events and
   // script writes share the same lamp channel there, so the value filter applies to both paths
   for (auto* bulbs : { &m_b2s->m_backglassIlluminations, &m_b2s->m_dmdIlluminations })
      for (auto& bulb : *bulbs)
      {
         const int romValue = bulb->m_b2sId >= 0 ? bulb->m_b2sValue : 0;
         switch (bulb->m_snippitType)
         {
         case B2SSnippitType::StandardImage: bulb->m_romUpdater = ResolveRomPropUpdater(items, &bulb->m_romProbe, bulb->m_romIdType, bulb->m_romId, bulb->m_romInverted, romValue); break;
         case B2SSnippitType::MechRotatingImage: bulb->m_romUpdater = ResolveRomPropUpdater(items, &bulb->m_mechProbe, bulb->m_romIdType, bulb->m_romId); break;
         case B2SSnippitType::SelfRotatingImage: bulb->m_romUpdater = ResolveRomPropUpdater(items, &bulb->m_romOn, bulb->m_romIdType, bulb->m_romId, bulb->m_romInverted, romValue); break;
         }
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
      // ROM polling: a ROM state change counts as a write into the same channel as script writes
      // (the reference routes both through the same shared state, the most recent one wins), so the
      // probe keeps updating even while a script write is in control. Probes are suspended while the
      // illumination group is locked by an animation, and the first observation does not count as a
      // change. Mech values are rotation steps, not visibility writes, and never take the channel.
      if (!locked)
      {
         float* const probe = bulb->m_snippitType == B2SSnippitType::MechRotatingImage ? &bulb->m_mechProbe
            : bulb->m_snippitType == B2SSnippitType::SelfRotatingImage                 ? &bulb->m_romOn
                                                                                       : &bulb->m_romProbe;
         const float prev = *probe;
         bulb->m_romUpdater();
         if (server != nullptr && prev >= 0.f && *probe != prev && bulb->m_romIdType != B2SRomIDType::Mech)
            bulb->m_romStamp = server->NextStamp();
      }
      float state = 0.f;
      bool fromLamp = false;
      const bool scripted = server && server->GetBulbState(*bulb, state, &fromLamp);
      // Script writes on the lamp channel of a rotating image drive its rotation instead of its
      // visibility (reference): for mech images the value selects the rotation step, for self
      // rotating images it toggles the spinning, most recent write between script and ROM wins
      const int lampId = (bulb->m_romIdType == B2SRomIDType::Lamp && bulb->m_romId > 0) ? bulb->m_romId : -1;
      uint64_t lampWriteStamp = 0;
      float lampWriteState = 0.f;
      const bool lampWinsRotation = server != nullptr && !locked && lampId > 0 && server->GetScriptedLampState(lampId, lampWriteState, &lampWriteStamp) && lampWriteStamp > bulb->m_romStamp;
      float rotation = 0.f;
      if (bulb->m_snippitType == B2SSnippitType::MechRotatingImage)
      {
         // Mech rotating images stay visible (the channel value only selects the rotation step);
         // illumination group writes still drive their visibility
         if (scripted && !fromLamp)
            bulb->m_brightness = state;
         if (lampWinsRotation)
            bulb->m_mechRot = lampWriteState;
         else if (bulb->m_mechProbe >= 0.f)
            bulb->m_mechRot = bulb->m_mechProbe;
         rotation = 360.f * (bulb->m_mechRot / static_cast<float>(bulb->m_snippitRotatingSteps));
      }
      else if (bulb->m_snippitType == B2SSnippitType::SelfRotatingImage)
      {
         if (scripted && !fromLamp)
            bulb->m_brightness = state;
         float spin = std::max(0.f, bulb->m_romOn);
         if (lampWinsRotation)
            spin = (bulb->m_b2sId >= 0 && bulb->m_b2sValue > 0) ? (static_cast<int>(lampWriteState) == bulb->m_b2sValue ? 1.f : 0.f)
                                                                : ((lampWriteState != 0.f) != bulb->m_romInverted ? 1.f : 0.f);
         // Rotation only toggles on state transitions so that explicit B2SStartRotation calls keep
         // spinning until B2SStopRotation or a state write (reference behaviour)
         const bool spinOn = spin >= 0.5f;
         if (spinOn != bulb->m_spinDriverOn)
         {
            bulb->m_spinDriverOn = spinOn;
            if (spinOn)
               bulb->StartRotation();
            else
               bulb->StopRotation();
         }
         bulb->UpdateRotation(elapsed);
         rotation = bulb->GetRotationAngle();
      }
      else if (scripted)
         bulb->m_brightness = state;
      else if (bulb->m_romStamp > 0 || (server == nullptr && bulb->m_romProbe >= 0.f))
         // The probe only takes over once a ROM change was actually observed: the first observation
         // is the initial state read, not a write, so InitialState survives until the first event
         bulb->m_brightness = bulb->m_romProbe;
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
            // Keep the raw segment mask: script digits decode to value-only, but frame-fed displays
            // need the full mask (alphanumeric glyphs do not decode to a digit). Must be applied
            // after B2SSetScoreDigit which clears the mask.
            server->ApplyScoreSegments(digitIndex, bitState);
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

      // Nothing to draw, and the digit width below would divide by zero
      if (reel.m_digits == 0)
         continue;

      // Skip digits located on the grill when the grill is hidden
      if (static_cast<float>(reel.m_locY) > ctx->srcHeight)
      {
         if (reel.m_b2sPlayerNo != 0)
            playerScoreOffset[reel.m_b2sPlayerNo] += static_cast<size_t>(std::max(0, reel.m_digits));
         continue;
      }

      // Legacy segment rendering (opt-out of the upgraded renderer): rasterize like the original
      // B2S server — one Dream7Display per LED display, or one simple LED box per digit
      if (!upgradedSegRenderingProp_Get() && (reel.m_scoreType == B2SScoreRenderer::Dream7 || reel.m_scoreType == B2SScoreRenderer::RenderedLED))
      {
         const int usedLEDType = server->GetUsedLEDType();
         const bool dream7 = (usedLEDType == 2) || (usedLEDType != 1 && reel.m_scoreType == B2SScoreRenderer::Dream7);
         RenderLegacyScoreDisplay(ctx, server, reel, dream7, segTypes, playerScoreText, playerScoreOffset);
         continue;
      }

      const float width = (static_cast<float>(reel.m_width) - 0.5f * static_cast<float>((reel.m_digits - 1) * reel.m_spacing)) / static_cast<float>(reel.m_digits);

      // Reel rolling: resolve the rolling direction, the illuminated image selection and the per-digit
      // hold flags. A digit waits while a less significant digit has a pending 9->0 / 0->9 wrap (carry).
      const bool rollUp = scores.m_reelRollingDirection != B2SReelRollingDirection::Down;
      // ReelIlluB2SID is in lamp-id space: script B2SSetData writes and ROM lamp states share the
      // channel; the most recent change wins
      {
         const float prevIllu = reel.m_reelIllu;
         reel.m_reelIlluUpdater();
         if (server != nullptr && prevIllu >= 0.f && reel.m_reelIllu != prevIllu)
            reel.m_reelIlluStamp = server->NextStamp();
      }
      float illuState = std::max(0.f, reel.m_reelIllu);
      uint64_t illuLampStamp = 0;
      float scriptedIllu = 0.f;
      if (server != nullptr && server->GetScriptedLampState(reel.m_reelIlluB2SID, scriptedIllu, &illuLampStamp) && illuLampStamp > reel.m_reelIlluStamp)
         illuState = scriptedIllu;
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
            // The styled profiles embed their own display color so the emitter tint must stay neutral
            // for them (it is multiplied over the profile color, tinting twice); only the generic
            // styles take the authored lit color. Same for the glass: the B2S dark color is the unlit
            // segment color, not a glass tint, and multiplied over the output it darkens the display.
            const vec4 litColor = (style == VPXSegDisplayRenderStyle::VPXSegStyle_GenPlasma || style == VPXSegDisplayRenderStyle::VPXSegStyle_GenLED)
               ? vec4(InvsRGB(reel.m_reelLitColor.x), InvsRGB(reel.m_reelLitColor.y), InvsRGB(reel.m_reelLitColor.z), 1.f)
               : vec4(1.f, 1.f, 1.f, 1.f);
            ctx->DrawSegDisplay(ctx, style, hint,
               // First layer: neutral glass, there is no glass texture
               nullptr, 1.f, 1.f, 1.f, 0.15f, // Glass texture, tint and roughness
               0.f, 0.f, 0.f, 0.f, // Glass texture coordinates (inside overall glass texture, cut for each element)
               0.f, 0.f, 0.f, // Glass lighting from room
               // Second layer: emitter tinted with the lit segment color
               segType, brightness.data(), // Segment emitter type and brightness array
               litColor.x, litColor.y, litColor.z, 1.f, 1.f, // Emitter tint, emitter brightness, emitter alpha
               0.f, 0.f, 0.f, 0.f, // Emitter padding (from glass border)
               // Render quad
               x, ctx->srcHeight - static_cast<float>(reel.m_locY + reel.m_height), width, static_cast<float>(reel.m_height));
         }
            break;
         }
      }
   }
}


// Legacy segment rendering (UpgradedSegmentRendering opt-out), matching the original B2S server
// implementation: a Dream7Display spans the whole score display, rendered LED boxes are one box
// per digit. Both rasterize the segments into a texture which is redrawn only on changes
// Segment count of a CTLPI_SEG_LAYOUT_xxx element type, matching B2SSegmentUntranslateBitCode's 'ledSegments' values
static int SegLayoutSegments(SegElementType type)
{
   switch (type)
   {
   case CTLPI_SEG_LAYOUT_7:
   case CTLPI_SEG_LAYOUT_7C:
   case CTLPI_SEG_LAYOUT_7D: return 7;
   case CTLPI_SEG_LAYOUT_9:
   case CTLPI_SEG_LAYOUT_9C: return 10;
   case CTLPI_SEG_LAYOUT_16: return 16;
   default: return 14;
   }
}

void B2SRenderer::RenderLegacyScoreDisplay(VPXRenderContext2D* ctx, B2SServer* server, const B2SScore& reel, bool dream7, const std::vector<SegElementType>& segTypes,
   const ankerl::unordered_dense::map<int, string>& playerScoreText, ankerl::unordered_dense::map<int, size_t>& playerScoreOffset)
{
   LegacySegDisplay& state = m_legacySegDisplays[&reel];
   if (!state.built || state.dream7 != dream7)
   {
      // (Re)build the display, also on a UsedLEDType mode change
      state = LegacySegDisplay {};
      state.dream7 = dream7;
      const uint8_t litR = static_cast<uint8_t>(std::clamp(reel.m_reelLitColor.x * 255.f + 0.5f, 0.f, 255.f));
      const uint8_t litG = static_cast<uint8_t>(std::clamp(reel.m_reelLitColor.y * 255.f + 0.5f, 0.f, 255.f));
      const uint8_t litB = static_cast<uint8_t>(std::clamp(reel.m_reelLitColor.z * 255.f + 0.5f, 0.f, 255.f));
      const uint8_t darkR = static_cast<uint8_t>(std::clamp(reel.m_reelDarkColor.x * 255.f + 0.5f, 0.f, 255.f));
      const uint8_t darkG = static_cast<uint8_t>(std::clamp(reel.m_reelDarkColor.y * 255.f + 0.5f, 0.f, 255.f));
      const uint8_t darkB = static_cast<uint8_t>(std::clamp(reel.m_reelDarkColor.z * 255.f + 0.5f, 0.f, 255.f));
      const uint32_t litColor = seg::RGB(litR, litG, litB);
      const uint32_t darkColor = seg::RGB(darkR, darkG, darkB);
      if (dream7)
      {
         auto d7 = std::make_unique<seg::Dream7Display>(m_vpxApi);
         d7->SetRect({ reel.m_locX - 1, reel.m_locY, reel.m_width, reel.m_height });
         d7->SetType(reel.m_ledSegments == 10 ? seg::SegmentNumberType_TenSegment : reel.m_ledSegments == 14 ? seg::SegmentNumberType_FourteenSegment : seg::SegmentNumberType_SevenSegment);
         d7->SetScaleMode(seg::ScaleMode_Stretch);
         d7->SetDigits(reel.m_digits);
         d7->SetSpacing(static_cast<float>(reel.m_spacing) * 5.f);
         // Reference color wiring: the glass colors derive from the authored lit segment color
         d7->SetLightColor(litColor);
         d7->SetGlassColor(seg::RGB(static_cast<uint8_t>(std::min(255, litR + 50)), static_cast<uint8_t>(std::min(255, litG + 50)), static_cast<uint8_t>(std::min(255, litB + 50))));
         d7->SetGlassColorCenter(seg::RGB(static_cast<uint8_t>(std::min(255, litR + 70)), static_cast<uint8_t>(std::min(255, litG + 70)), static_cast<uint8_t>(std::min(255, litB + 70))));
         d7->SetOffColor(darkColor);
         d7->SetGlassAlpha(140);
         d7->SetGlassAlphaCenter(255);
         d7->SetThickness(static_cast<float>(reel.m_thickness) / 100.f * 1.2f);
         d7->SetShear(static_cast<float>(reel.m_shear) / 100.f);
         d7->SetGlow(static_cast<float>(std::lround(static_cast<float>(reel.m_glow) / 100.f)));
         // 'Taxi' patch to shear the third LED display (reference FormBackglass)
         if (reel.m_id == 3 && string_to_lower(m_b2s->m_name) == "taxi"s)
         {
            d7->SetAngle(4.f);
            d7->SetShear(d7->GetShear() / 2.f);
         }
         state.display = std::move(d7);
      }
      else
      {
         const int width = static_cast<int>(std::lround((reel.m_width - (reel.m_digits - 1) * reel.m_spacing / 2.f) / reel.m_digits));
         state.boxes.reserve(static_cast<size_t>(reel.m_digits));
         for (int i = 0; i < reel.m_digits; i++)
         {
            auto box = std::make_unique<seg::LEDBox>(m_vpxApi);
            const int x = static_cast<int>(std::lround((reel.m_locX - 1) + i * (width + reel.m_spacing / 2.f)));
            box->SetRect({ x, reel.m_locY, width, reel.m_height });
            box->SetLEDType(reel.m_ledSegments == 10 ? seg::eLEDType_LED10 : reel.m_ledSegments == 14 ? seg::eLEDType_LED14 : seg::eLEDType_LED8);
            box->SetLitLEDSegmentColor(litColor);
            box->SetDarkLEDSegmentColor(darkColor);
            state.boxes.push_back(std::move(box));
         }
      }
      state.built = true;
   }

   // Feed the digits: an explicit script segment mask (B2SSetLED/B2SSetLEDDisplay, untranslated
   // back to the Dream7 bit order these renderers expect) wins, else the digit glyph
   for (int i = 0; i < reel.m_digits; i++)
   {
      const int index = reel.m_resolvedStartDigit + i;
      string text;
      if (reel.m_b2sPlayerNo != 0)
      {
         const auto it = playerScoreText.find(reel.m_b2sPlayerNo);
         const size_t offset = playerScoreOffset[reel.m_b2sPlayerNo] + static_cast<size_t>(i);
         text = string(1, (it != playerScoreText.end() && offset < it->second.length()) ? it->second[offset] : ' ');
      }
      else
      {
         // TODO As the original B2SSetScoreDigit: decimal text, of which only the first character is drawn (10-15 show '1').
         // TODO Unlike the original, a digit the script never set shows '0' (the original left it blank)
         text = std::to_string(server->GetScoreDigit(index));
      }
      const int segMask = server->GetScoreDigitSegments(index);
      // Source segment layout for the mask untranslation: the PinMAME frame element's own layout
      // when the digit is frame-fed (it may differ from the display's declared type, e.g. a Williams
      // 16 segment alphanum source driving a Dream7LED14 display), else the declared segment count.
      const int srcSegs = (index > 0 && index <= static_cast<int>(segTypes.size())) ? SegLayoutSegments(segTypes[static_cast<size_t>(index - 1)]) : reel.m_ledSegments;
      if (dream7)
      {
         if (segMask >= 0)
            state.display->SetValue(i, B2SSegmentUntranslateBitCode(static_cast<uint32_t>(segMask), srcSegs));
         else
            state.display->SetValue(i, text);
      }
      else if (i < static_cast<int>(state.boxes.size()))
      {
         if (segMask >= 0)
            state.boxes[static_cast<size_t>(i)]->SetValue(B2SSegmentUntranslateBitCode(static_cast<uint32_t>(segMask), srcSegs));
         else
            state.boxes[static_cast<size_t>(i)]->SetText(text);
      }
   }
   if (reel.m_b2sPlayerNo != 0)
      playerScoreOffset[reel.m_b2sPlayerNo] += static_cast<size_t>(reel.m_digits);

   if (dream7)
      state.display->Render(ctx);
   else
      for (auto& box : state.boxes)
         box->Render(ctx);
}


bool B2SRenderer::RenderBackglass(VPXRenderContext2D* ctx, B2SServer* server)
{
   // Update to latest settings state
   m_grillCut = showGrillProp_Get() ? 0.f : static_cast<float>(m_b2s->m_grillHeight);

   const VPXTextureInfo* bgTexInfo = nullptr;
   if (m_b2s->m_backglassOffImage.m_image)
      bgTexInfo = GetTextureInfo(m_b2s->m_backglassOffImage.m_image);
   else if (m_b2s->m_backglassImage.m_image)
      bgTexInfo = GetTextureInfo(m_b2s->m_backglassImage.m_image);
   m_b2sWidth = bgTexInfo ? static_cast<float>(bgTexInfo->width) : 1024.f;
   m_b2sHeight = (bgTexInfo ? static_cast<float>(bgTexInfo->height) : 768.f) - m_grillCut;

   ctx->srcWidth = m_b2sWidth;
   ctx->srcHeight = m_b2sHeight;

   // Update animations
   auto now = std::chrono::steady_clock::now();
   float elapsed = std::min(1.f, static_cast<float>(static_cast<double>((now - m_lastBackglassRenderTick).count()) / 1000000000.0));
   m_lastBackglassRenderTick = now;
   const B2SDualMode dualMode = ActiveDualMode();
   DispatchScriptTriggers(server);
   UpdateAnimations(m_b2s->m_backglassAnimations, elapsed, server, dualMode);

   // Draw background
   B2SImage& onImage = m_b2s->m_backglassOnImage;
   {
      const float prev = onImage.m_romProbe;
      onImage.m_romUpdater();
      if (server != nullptr && prev >= 0.f && onImage.m_romProbe != prev)
         onImage.m_romStamp = server->NextStamp();
   }
   // The lit background is also driven by script writes on its lamp channel (reference MyB2SSetData):
   // the most recent write between script and ROM wins, RomInverted is never applied to this channel
   uint64_t onLampStamp = 0;
   float onLampState = 0.f;
   if (server != nullptr && onImage.m_romIdType == B2SRomIDType::Lamp && onImage.m_romId > 0 && server->GetScriptedLampState(onImage.m_romId, onLampState, &onLampStamp)
      && onLampStamp > onImage.m_romStamp)
      onImage.m_brightness = (onLampState != 0.f) ? 1.f : 0.f;
   else if (onImage.m_romProbe >= 0.f)
      onImage.m_brightness = onImage.m_romProbe;

   if (onImage.m_image == nullptr || onImage.m_brightness < 1.f)
   {
      // The unlit variant is the OffImage when the file provides one (the reference's DarkImage)
      const B2SImage& darkImage = m_b2s->m_backglassOffImage.m_image ? m_b2s->m_backglassOffImage : m_b2s->m_backglassImage;
      if (darkImage.m_image)
      {
         const VPXTextureInfo* texInfo = GetTextureInfo(darkImage.m_image);
         ctx->DrawImage(ctx, darkImage.m_image, 1.f, 1.f, 1.f, 1.f, 0.f, m_grillCut, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height) - m_grillCut, 0.f, 0.f,
            0.f, // No rotation
            0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height) - m_grillCut);
      }
   }
   if (onImage.m_image)
   {
      const VPXTextureInfo* texInfo = GetTextureInfo(onImage.m_image);
      ctx->DrawImage(ctx, onImage.m_image, 1.f, 1.f, 1.f, onImage.m_brightness, 0.f, m_grillCut, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height) - m_grillCut, 0.f,
         0.f, 0.f, // No rotation
         0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height) - m_grillCut);
   }

   // Draw illuminations, scores and DMD overlays
   RenderBulbs(ctx, server, m_b2s->m_backglassIlluminations, elapsed, dualMode);
   RenderScores(ctx, server, m_b2s->m_backglassScores, elapsed);
   const VPXTexture backImage = m_b2s->m_backglassImage.m_image ? m_b2s->m_backglassImage.m_image
      : m_b2s->m_backglassOffImage.m_image                      ? m_b2s->m_backglassOffImage.m_image
                                                                : m_b2s->m_backglassOnImage.m_image;
   RenderDisplays(ctx, server, 0, backImage);

   return true;
}

bool B2SRenderer::RenderScoreView(VPXRenderContext2D* ctx, B2SServer* server)
{
   if (m_b2s->m_dmdImage.m_image == nullptr && m_b2s->m_dmdIlluminations.empty())
      return false;

   // Update to latest settings state
   ctx->srcWidth = m_dmdWidth;
   ctx->srcHeight = m_dmdHeight;

   // Update animations
   auto now = std::chrono::steady_clock::now();
   float elapsed = std::min(1.f, static_cast<float>(static_cast<double>((now - m_lastDmdRenderTick).count()) / 1000000000.0));
   m_lastDmdRenderTick = now;
   DispatchScriptTriggers(server);
   UpdateAnimations(m_b2s->m_dmdAnimations, elapsed, server, ActiveDualMode());
   // Draw background
   if (m_b2s->m_dmdImage.m_image)
      ctx->DrawImage(ctx, m_b2s->m_dmdImage.m_image, 1.f, 1.f, 1.f, 1.f,
         0.f, 0.f, m_dmdWidth, m_dmdHeight,
         0.f, 0.f, 0.f, // No rotation
         0.f, 0.f, m_dmdWidth, m_dmdHeight);

   // Draw illuminations, scores and DMD overlays
   RenderBulbs(ctx, server, m_b2s->m_dmdIlluminations, elapsed, B2SDualMode::Both); // Reference does not dual-filter score view bulbs
   RenderScores(ctx, server, m_b2s->m_dmdScores, elapsed);
   RenderDisplays(ctx, server, 1, m_b2s->m_dmdImage.m_image);

   return true;
}

// Renders the <DMD> displays of the document targeted at this view, growing the overlay pool on
// demand (a display presence in the document means enabled). Each display keeps the overlay of its
// document index whatever its target, so that the overlay state (glass texture, uploaded frame,
// bounds search) is not shared between the backglass and score view displays
void B2SRenderer::RenderDisplays(VPXRenderContext2D* ctx, B2SServer* server, int target, VPXTexture backImage)
{
   vector<B2SDMDDisplay>& displays = server->GetOverlayDoc().displays;
   for (size_t i = 0; i < displays.size(); i++)
   {
      if (displays[i].target != target)
         continue;
      while (i >= m_dmdOverlays.size())
         m_dmdOverlays.emplace_back(std::make_unique<DMDOverlay::Renderer>(m_vpxApi, m_resURIResolver));
      m_dmdOverlays[i]->Render(ctx, displays[i], backImage);
   }
}
}
