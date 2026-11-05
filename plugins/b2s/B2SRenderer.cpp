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
   DeleteTexture(m_whiteTexture);
   for (auto& [key, texture] : m_reel3DCache)
      DeleteTexture(texture);
   for (auto& [reel, entry] : m_reel3DOverlays)
      DeleteTexture(entry.texture);
   for (auto& [pair, overlay] : m_proLightOverlays)
      DeleteTexture(overlay.texture);
}

// Reference DrawDynamicFrontLight: for every visible image snippit drawn below the light (lower ZOrder, or
// the one DynamicFrontTargetSnippetID names), the overlap of the light and the snippit is lit: the snippit's
// current artwork is cropped there, the light field is scaled from the light's rectangle onto it, and the
// kernel's result is drawn over the artwork
void B2SRenderer::RenderProDynamicLight(VPXRenderContext2D* ctx, const B2SBulb& light, const vector<std::unique_ptr<B2SBulb>>& bulbs, const B2SDualMode dualMode)
{
   const VPXTextureInfo* const fieldInfo = GetTextureInfo(light.m_pro.dynamicFrontLightField);
   if (fieldInfo == nullptr || fieldInfo->data == nullptr || fieldInfo->width == 0 || fieldInfo->height == 0 || light.m_width <= 0 || light.m_height <= 0)
      return;
   const int fieldBpp = fieldInfo->format == VPXTEXFMT_sRGBA8 ? 4 : (fieldInfo->format == VPXTEXFMT_sRGB8 ? 3 : 0);
   if (fieldBpp == 0)
      return;
   const float lx = static_cast<float>(light.m_locationX), ly = static_cast<float>(light.m_locationY);
   const float lw = static_cast<float>(light.m_width), lh = static_cast<float>(light.m_height);
   for (const auto& snippitPtr : bulbs)
   {
      const B2SBulb& snippit = *snippitPtr;
      if (&snippit == &light || !snippit.m_isImageSnippit || snippit.m_brightness <= 0.f || snippit.m_image == nullptr || snippit.m_zOrder >= light.m_zOrder)
         continue;
      if (light.m_pro.dynamicFrontTargetSnippetId > 0 && snippit.m_pro.bulbId != light.m_pro.dynamicFrontTargetSnippetId)
         continue;
      if (m_b2s->m_dualBackglass && snippit.m_dualMode != B2SDualMode::Both && dualMode != B2SDualMode::Both && snippit.m_dualMode != dualMode)
         continue;
      const float sw = static_cast<float>(snippit.m_width), sh = static_cast<float>(snippit.m_height);
      if (sw <= 0.f || sh <= 0.f)
         continue;
      const float sx = snippit.m_proRt.posValid ? snippit.m_proRt.posX - sw * 0.5f : static_cast<float>(snippit.m_locationX);
      const float sy = snippit.m_proRt.posValid ? snippit.m_proRt.posY - sh * 0.5f : static_cast<float>(snippit.m_locationY);
      // The lit crop is of the snippit as currently shown (the reference's CreateCurrentSnippetFrame): a rolling
      // ball turned by its roll angle about its centre, a pivoting or rotating image by its angle about its pivot
      const bool nativeRotation = snippit.HasProPivot() || snippit.HasProRotation() || snippit.m_proRt.externalPivot;
      const float angle = nativeRotation ? snippit.m_proRt.angle : (snippit.m_proRt.posValid ? snippit.m_proRt.rollAngle : 0.f);
      const float pivotPx = sx + sw * (nativeRotation ? snippit.m_pro.pivotX : 0.5f), pivotPy = sy + sh * (nativeRotation ? snippit.m_pro.pivotY : 0.5f);
      const int ox = static_cast<int>(std::lround(std::max(lx, sx))), oy = static_cast<int>(std::lround(std::max(ly, sy)));
      const int ox2 = static_cast<int>(std::lround(std::min(lx + lw, sx + sw))), oy2 = static_cast<int>(std::lround(std::min(ly + lh, sy + sh)));
      const int ow = ox2 - ox, oh = oy2 - oy;
      if (ow <= 0 || oh <= 0)
         continue;
      ProLightOverlay& overlay = m_proLightOverlays[{ &light, &snippit }];
      if (!overlay.valid || overlay.snippitX != sx || overlay.snippitY != sy || overlay.width != ow || overlay.height != oh || overlay.angle != angle)
      {
         overlay.valid = false;
         const VPXTextureInfo* const art = GetTextureInfo(snippit.m_image);
         if (art == nullptr || art->data == nullptr || art->width == 0 || art->height == 0 || (art->format != VPXTEXFMT_sRGBA8 && art->format != VPXTEXFMT_sRGB8))
            continue;
         const int artBpp = art->format == VPXTEXFMT_sRGBA8 ? 4 : 3;
         const uint8_t* const artPixels = static_cast<const uint8_t*>(art->data);
         const uint8_t* const fieldPixels = static_cast<const uint8_t*>(fieldInfo->data);
         // Crop of the snippit artwork (nearest sample of the artwork scaled into its rectangle) and of the
         // field (scaled from the light rectangle), both at the overlap, in backglass pixels
         vector<uint8_t> source(static_cast<size_t>(ow) * oh * 4), field(static_cast<size_t>(ow) * oh);
         // Rotation as DrawImage applies it (a positive angle turns clockwise on screen): a crop pixel maps back to
         // the unrotated artwork by the inverse turn about the pivot, and lands transparent outside it
         const float rad = angle * static_cast<float>(M_PI / 180.0), cosA = std::cos(rad), sinA = std::sin(rad);
         for (int y = 0; y < oh; y++)
            for (int x = 0; x < ow; x++)
            {
               float cx = static_cast<float>(ox + x) + 0.5f, cy = static_cast<float>(oy + y) + 0.5f;
               uint8_t* const d = source.data() + (static_cast<size_t>(y) * ow + x) * 4;
               if (angle != 0.f)
               {
                  const float dx = cx - pivotPx, dy = cy - pivotPy;
                  const float ux = dx * cosA + dy * sinA, uy = -dx * sinA + dy * cosA;
                  cx = pivotPx + ux;
                  cy = pivotPy + uy;
                  if (cx < sx || cy < sy || cx >= sx + sw || cy >= sy + sh)
                  {
                     d[0] = d[1] = d[2] = d[3] = 0;
                     const int fx0 = std::clamp(static_cast<int>((static_cast<float>(ox + x) + 0.5f - lx) / lw * static_cast<float>(fieldInfo->width)), 0, static_cast<int>(fieldInfo->width) - 1);
                     const int fy0 = std::clamp(static_cast<int>((static_cast<float>(oy + y) + 0.5f - ly) / lh * static_cast<float>(fieldInfo->height)), 0, static_cast<int>(fieldInfo->height) - 1);
                     field[static_cast<size_t>(y) * ow + x] = fieldBpp == 4 ? fieldPixels[(static_cast<size_t>(fy0) * fieldInfo->width + fx0) * 4 + 3] : 255;
                     continue;
                  }
               }
               const int ax = std::clamp(static_cast<int>((cx - sx) / sw * static_cast<float>(art->width)), 0, static_cast<int>(art->width) - 1);
               const int ay = std::clamp(static_cast<int>((cy - sy) / sh * static_cast<float>(art->height)), 0, static_cast<int>(art->height) - 1);
               const uint8_t* const p = artPixels + (static_cast<size_t>(ay) * art->width + ax) * artBpp;
               d[0] = p[0];
               d[1] = p[1];
               d[2] = p[2];
               d[3] = artBpp == 4 ? p[3] : 255;
               cx = static_cast<float>(ox + x) + 0.5f; // The field is sampled at the crop pixel itself
               cy = static_cast<float>(oy + y) + 0.5f;
               const int fx = std::clamp(static_cast<int>((cx - lx) / lw * static_cast<float>(fieldInfo->width)), 0, static_cast<int>(fieldInfo->width) - 1);
               const int fy = std::clamp(static_cast<int>((cy - ly) / lh * static_cast<float>(fieldInfo->height)), 0, static_cast<int>(fieldInfo->height) - 1);
               field[static_cast<size_t>(y) * ow + x] = fieldBpp == 4 ? fieldPixels[(static_cast<size_t>(fy) * fieldInfo->width + fx) * 4 + 3] : 255;
            }
         ProEffects::DynamicLightParams params;
         params.exposure = light.m_pro.dynamicLightExposure;
         params.colorR = light.m_pro.dynamicLightColor.x;
         params.colorG = light.m_pro.dynamicLightColor.y;
         params.colorB = light.m_pro.dynamicLightColor.z;
         params.flasherStyle = light.m_pro.dynamicFlasherStyle;
         params.saturation = light.m_pro.dynamicSaturation;
         params.highlightProtection = light.m_pro.dynamicHighlightProtection;
         params.darkAreaLift = light.m_pro.dynamicDarkAreaLift;
         params.transmissionContrast = light.m_pro.dynamicTransmissionContrast;
         params.temperature = light.m_pro.dynamicLightTemperature;
         const vector<uint8_t> lit = ProEffects::RenderDynamicLight(source.data(), 4, field.data(), ow, oh, params);
         if (lit.empty())
            continue;
         UpdateTexture(&overlay.texture, ow, oh, VPXTEXFMT_sRGBA8, lit.data());
         overlay.x = static_cast<float>(ox);
         overlay.y = static_cast<float>(oy);
         overlay.width = ow;
         overlay.height = oh;
         overlay.snippitX = sx;
         overlay.snippitY = sy;
         overlay.angle = angle;
         overlay.valid = overlay.texture != nullptr;
      }
      if (overlay.valid)
         ctx->DrawImage(ctx, overlay.texture, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, static_cast<float>(ow), static_cast<float>(oh), 0.f, 0.f, 0.f, overlay.x, overlay.y, static_cast<float>(ow), static_cast<float>(oh));
   }
}

VPXTexture B2SRenderer::Reel3DTexture(const VPXTexture image, const int width, const int height, const int brightness, const int temperature, const int depth)
{
   const Reel3DKey key { image, width, height, brightness, temperature, depth };
   if (const auto it = m_reel3DCache.find(key); it != m_reel3DCache.end())
      return it->second;
   VPXTexture texture = nullptr;
   const VPXTextureInfo* const info = GetTextureInfo(image);
   if (info != nullptr && info->data != nullptr && info->width > 0 && info->height > 0 && (info->format == VPXTEXFMT_sRGBA8 || info->format == VPXTEXFMT_sRGB8) && width > 0 && height > 0)
   {
      const std::vector<uint8_t> pixels = ProEffects::RenderReel(static_cast<const uint8_t*>(info->data), static_cast<int>(info->width), static_cast<int>(info->height),
         info->format == VPXTEXFMT_sRGBA8 ? 4 : 3, width, height, brightness, temperature, depth);
      if (!pixels.empty())
         UpdateTexture(&texture, width, height, VPXTEXFMT_sRGBA8, pixels.data());
   }
   m_reel3DCache[key] = texture; // A failed render is cached as nullptr: the plain image is drawn
   return texture;
}

VPXTexture B2SRenderer::Reel3DOverlay(const B2SScore& reel, const int width, const int height)
{
   Reel3DOverlayEntry& entry = m_reel3DOverlays[&reel];
   if (entry.texture == nullptr || entry.width != width || entry.height != height || entry.depth != reel.m_pro.reel3DDepth || entry.glass != reel.m_pro.reel3DGlass)
   {
      DeleteTexture(entry.texture);
      entry = { width, height, reel.m_pro.reel3DDepth, reel.m_pro.reel3DGlass, nullptr };
      const std::vector<uint8_t> pixels = ProEffects::ReelWindowOverlay(width, height, reel.m_pro.reel3DDepth, reel.m_pro.reel3DGlass);
      if (!pixels.empty())
         UpdateTexture(&entry.texture, width, height, VPXTEXFMT_sRGBA8, pixels.data());
   }
   return entry.texture;
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

static B2SRomIDType ProTriggerRomIdType(B2SProTriggerType type);

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
         // B2S Pro triggers of pivots and automatic rotations (solenoid / lamp / switch from the bus; B2S ids
         // and startup are handled when polling). An automatic rotation replaces the bulb's own ROM binding
         // by its trigger in the reference (the picture stays visible, the trigger only spins it)
         bulb->m_proRt.triggerUpdater = []() { };
         if (bulb->HasProRotation())
         {
            bulb->m_romUpdater = []() { };
            if (!bulb->m_pro.automaticRotationContinuous && bulb->m_pro.automaticRotationTriggerId > 0)
               bulb->m_proRt.triggerUpdater = ResolveRomPropUpdater(items, &bulb->m_proRt.triggerProbe, ProTriggerRomIdType(bulb->m_pro.automaticRotationTriggerType), bulb->m_pro.automaticRotationTriggerId);
         }
         else if (bulb->HasProPivot())
            bulb->m_proRt.triggerUpdater = ResolveRomPropUpdater(items, &bulb->m_proRt.triggerProbe, ProTriggerRomIdType(static_cast<B2SProTriggerType>(bulb->m_pro.pivotTriggerType)), bulb->m_pro.pivotTriggerId);
         else if (bulb->m_pro.physicsBall && bulb->m_pro.physicsLauncher.enabled)
            bulb->m_proRt.triggerUpdater = ResolveRomPropUpdater(items, &bulb->m_proRt.triggerProbe, ProTriggerRomIdType(static_cast<B2SProTriggerType>(bulb->m_pro.physicsLauncher.triggerType)), bulb->m_pro.physicsLauncher.triggerId);
         for (B2SBulb::ProRuntime::Trigger& trigger : bulb->m_proRt.motionTriggers)
            trigger.updater = ResolveRomPropUpdater(items, &trigger.probe, ProTriggerRomIdType(trigger.type), trigger.id);
      }

   // B2S Pro: say which trigger sources were found, authors need to know why a trigger stays silent
   if (!items.empty() && m_b2s->IsB2SPro())
   {
      const auto describe = [&items](const B2SRomIDType type, const int id) -> string
      {
         int groupId;
         switch (type)
         {
         case B2SRomIDType::Solenoid: groupId = PMPI_GROUP_SOLENOID; break;
         case B2SRomIDType::Lamp: groupId = PMPI_GROUP_LAMP; break;
         case B2SRomIDType::Switch: groupId = PMPI_GROUP_SWITCH; break;
         default: return "script"s;
         }
         int groupStates = 0;
         for (const StateSrcId& src : items)
         {
            if (src.id.resId != groupId)
               continue;
            groupStates += static_cast<int>(src.nStates);
            for (unsigned int i = 0; i < src.nStates; i++)
               if (src.stateDefs[i].mappingId == id)
                  return std::format("bound (format {})", static_cast<int>(src.stateDefs[i].dataFormat));
         }
         return std::format("NOT FOUND ({} states in the group)", groupStates);
      };
      for (const auto* bulbs : { &m_b2s->m_backglassIlluminations, &m_b2s->m_dmdIlluminations })
         for (const auto& bulb : *bulbs)
         {
            if (bulb->HasProPivot() && bulb->m_pro.pivotTriggerType != static_cast<int>(B2SProTriggerType::Startup))
               LOGI(std::format("B2S Pro pivot trigger of '{}': type {} id {}: {}", bulb->m_name, bulb->m_pro.pivotTriggerType, bulb->m_pro.pivotTriggerId,
                  describe(ProTriggerRomIdType(static_cast<B2SProTriggerType>(bulb->m_pro.pivotTriggerType)), bulb->m_pro.pivotTriggerId)));
            for (const B2SBulb::ProRuntime::Trigger& trigger : bulb->m_proRt.motionTriggers)
               LOGI(std::format("B2S Pro motion trigger of '{}': action {} type {} id {}: {}", bulb->m_name, static_cast<int>(trigger.action), static_cast<int>(trigger.type), trigger.id,
                  describe(ProTriggerRomIdType(trigger.type), trigger.id)));
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

// B2S Pro trigger kind to the state group it is polled from (B2S ids and startup have no bus source)
static B2SRomIDType ProTriggerRomIdType(const B2SProTriggerType type)
{
   switch (type)
   {
   case B2SProTriggerType::Solenoid: return B2SRomIDType::Solenoid;
   case B2SProTriggerType::Lamp: return B2SRomIDType::Lamp;
   case B2SProTriggerType::Switch: return B2SRomIDType::Switch;
   default: return B2SRomIDType::NotDefined;
   }
}

// Reads a B2S Pro trigger: the bus probe and, for lamp and B2S id triggers, script writes on that lamp
// channel, the most recent one winning (as bulbs do). Returns false while nothing was observed yet
static bool PollProTriggerState(float& probe, uint64_t& stampRef, const std::function<void()>& updater, B2SServer* server, const B2SProTriggerType type, const int id, float& value)
{
   if (id <= 0)
      return false;
   bool known = false;
   const float prevProbe = probe;
   updater();
   if (probe >= 0.f)
   {
      known = true;
      value = probe;
      if (server != nullptr && prevProbe >= 0.f && probe != prevProbe)
         stampRef = server->NextStamp();
   }
   if (server != nullptr && (type == B2SProTriggerType::Lamp || type == B2SProTriggerType::B2SID))
   {
      float scripted;
      uint64_t stamp = 0;
      if (server->GetScriptedLampState(id, scripted, &stamp) && stamp > stampRef)
      {
         known = true;
         value = scripted;
         stampRef = stamp;
      }
   }
   return known;
}

bool B2SRenderer::PollProTrigger(B2SBulb::ProRuntime& rt, B2SServer* server, const B2SProTriggerType type, const int id, float& value)
{
   return PollProTriggerState(rt.triggerProbe, rt.triggerStamp, rt.triggerUpdater, server, type, id, value);
}

// Polls the trigger of a pivot or automatic rotation and applies its edges, then advances the animation
// (unless the pivot is advanced by the physics clock)
void B2SRenderer::UpdateProTrigger(B2SBulb& bulb, B2SServer* server, const B2SProTriggerType type, const int id, const bool inverted, const float elapsed, const bool advance)
{
   B2SBulb::ProRuntime& rt = bulb.m_proRt;
   const bool pivot = bulb.HasProPivot();
   if (type == B2SProTriggerType::Startup)
   {
      if (!rt.startupDone)
      {
         rt.startupDone = true;
         if (pivot)
            bulb.ProSetPivotTriggerState(true);
      }
   }
   else if (bulb.HasProRotation() && bulb.m_pro.automaticRotationContinuous)
   {
      if (!rt.startupDone)
      {
         rt.startupDone = true;
         bulb.ProStartRotation();
      }
   }
   else if (float value = 0.f; PollProTrigger(rt, server, type, id, value))
   {
      const bool on = (value != 0.f) != inverted;
      if (on != rt.triggerPrev)
      {
         rt.triggerPrev = on;
         if (pivot)
            bulb.ProSetPivotTriggerState(on);
         else
            bulb.ProSetRotationState(on);
      }
   }
   if (!advance)
      return;
   if (pivot)
      bulb.ProAdvancePivot(elapsed);
   else
      bulb.ProAdvanceRotation(elapsed);
}

// Physics balls of an illumination list, with their flipper pivots (RegisterPhysicsBall / FindPivotPicture)
void B2SRenderer::BuildProPhysics(const vector<std::unique_ptr<B2SBulb>>& bulbs, B2SServer* server)
{
   ProPhysicsWorld& world = m_proPhysics[&bulbs];
   for (const auto& bulb : bulbs)
   {
      if (!bulb->m_pro.physicsBall || bulb->m_pro.physicsBounds.IsEmpty() || bulb->m_snippitType != B2SSnippitType::StandardImage)
         continue;
      B2SBulb* flipper = nullptr;
      if (!bulb->m_pro.physicsFlipperName.empty())
         for (const auto& candidate : bulbs)
            if (candidate->HasProPivot() && candidate->m_name == bulb->m_pro.physicsFlipperName)
            {
               flipper = candidate.get();
               break;
            }
      auto ball = std::make_unique<B2SProPhysicsBall>(*bulb, flipper);
      if (ball->Flipper() != nullptr && !ball->Flipper()->m_proRt.physicsDriven)
      {
         ball->Flipper()->m_proRt.physicsDriven = true;
         world.pivots.push_back(ball->Flipper());
      }
      ball->pulseSwitch = [server](int switchId)
      {
         if (server != nullptr)
            server->PulseSwitch(switchId);
      };
      // Gravity trough balls wait for their motion path (not implemented yet): they stay at their authored place
      if (!bulb->m_pro.motionPathGravityDrop)
         ball->Start();
      bulb->m_proRt.physicsBall = ball.get();
      world.balls.push_back(std::move(ball));
   }
   // Motion path sequence groups (RegisterMotionPathSequence): members named by group with a positive order
   for (const auto& bulb : bulbs)
   {
      if (!bulb->m_isImageSnippit || bulb->m_pro.motionPathSequenceGroup.empty() || bulb->m_pro.motionPathSequenceOrder <= 0)
         continue;
      const string groupName = string_to_lower(bulb->m_pro.motionPathSequenceGroup);
      B2SProSequenceGroup* group = nullptr;
      for (auto& candidate : world.sequences)
         if (candidate->Name() == groupName)
         {
            group = candidate.get();
            break;
         }
      if (group == nullptr)
      {
         world.sequences.push_back(std::make_unique<B2SProSequenceGroup>(groupName));
         group = world.sequences.back().get();
      }
      group->Register(bulb.get());
      world.sequenceOf[bulb.get()] = group;
   }
   // External position grids, external pivot groups and single image pivots (reference registration order)
   for (const auto& bulb : bulbs)
   {
      const B2SProBulb& pro = bulb->m_pro;
      if (!pro.externalGridName.empty())
      {
         auto& grid = world.positionGrids[string_to_lower(pro.externalGridName)];
         if (!grid)
            grid = std::make_unique<B2SProPositionGrid>();
         grid->Register(bulb.get(), bulb->m_name, pro.externalGridRepresentative, pro.externalGridDuration);
      }
      if (!pro.externalPivotGroup.empty())
      {
         auto& group = world.pivotGroups[string_to_lower(pro.externalPivotGroup)];
         if (!group)
            group = std::make_unique<B2SProExternalPivotGroup>();
         group->Register(bulb.get(), bulb->m_name, pro.externalPivotRepresentative, pro.externalPivotAngle, pro.pivotDuration);
      }
      if (pro.pivotAnimation && pro.pivotTriggerType == 0 && !pro.pivotDownTrigger.empty() && !pro.pivotUpTrigger.empty())
      {
         auto group = std::make_unique<B2SProExternalPivotGroup>();
         group->Register(bulb.get(), pro.pivotDownTrigger, true, pro.pivotDownAngle, pro.pivotDuration);
         group->Register(bulb.get(), pro.pivotUpTrigger, false, pro.pivotUpAngle, pro.pivotDuration);
         world.pivotGroups["pivot:"s + string_to_lower(bulb->m_name)] = std::move(group);
      }
   }
}

void B2SRenderer::UpdateProRuntime(const vector<std::unique_ptr<B2SBulb>>& bulbs, B2SServer* server, const float elapsed)
{
   if (m_proPhysics.find(&bulbs) == m_proPhysics.end())
      BuildProPhysics(bulbs, server);
   ProPhysicsWorld& world = m_proPhysics[&bulbs];
   std::set<B2SProSequenceGroup*> groupStarts, groupRemoves;

   // Triggers, and the per frame animations
   for (const auto& bulb : bulbs)
   {
      if (bulb->m_snippitType != B2SSnippitType::StandardImage)
         continue;
      if (bulb->HasProRotation())
         UpdateProTrigger(*bulb, server, bulb->m_pro.automaticRotationTriggerType, bulb->m_pro.automaticRotationTriggerId, bulb->m_romInverted, elapsed, true);
      else if (bulb->HasProPivot())
         UpdateProTrigger(*bulb, server, static_cast<B2SProTriggerType>(bulb->m_pro.pivotTriggerType), bulb->m_pro.pivotTriggerId, false, elapsed, !bulb->m_proRt.physicsDriven);
      else if (bulb->m_proRt.externalPivot && !bulb->m_proRt.physicsDriven)
         bulb->ProAdvancePivot(elapsed);
      // Motion path triggers: every action fires on a change to a non zero value (the reference's lamp / solenoid /
      // B2S id / switch routes). Not for physics balls: their travel is the simulation's. Sequence group members
      // route their start and remove to the group (once per group and frame), stop and resume stay per snippit
      if (!bulb->m_proRt.motionTriggers.empty() && (bulb->m_proRt.physicsBall == nullptr || bulb->m_pro.motionPathGravityDrop))
      {
         const auto groupIt = world.sequenceOf.find(bulb.get());
         B2SProSequenceGroup* const group = groupIt != world.sequenceOf.end() ? groupIt->second : nullptr;
         for (B2SBulb::ProRuntime::Trigger& trigger : bulb->m_proRt.motionTriggers)
         {
            float value = 0.f;
            if (!PollProTriggerState(trigger.probe, trigger.stamp, trigger.updater, server, trigger.type, trigger.id, value))
               continue;
            const bool on = value != 0.f;
            if (on == trigger.prevOn)
               continue;
            trigger.prevOn = on;
            if (!on)
               continue;
            LOGI(std::format("B2S Pro motion trigger of '{}' fired: action {} type {} id {}", bulb->m_name, static_cast<int>(trigger.action), static_cast<int>(trigger.type), trigger.id));
            switch (trigger.action)
            {
            case B2SBulb::ProRuntime::MotionAction::Start:
               if (group != nullptr)
                  groupStarts.insert(group);
               else
                  bulb->ProStartMotionPath(false, false);
               break;
            case B2SBulb::ProRuntime::MotionAction::Stop: bulb->ProStopMotionPath(); break;
            case B2SBulb::ProRuntime::MotionAction::Resume: bulb->ProResumeMotionPath(); break;
            case B2SBulb::ProRuntime::MotionAction::Remove:
               if (group != nullptr)
                  groupRemoves.insert(group);
               else if (bulb->HasProExitPath())
                  bulb->ProStartMotionPath(true, false);
               break;
            }
         }
      }
      if (bulb->m_proRt.physicsBall == nullptr || bulb->m_pro.motionPathGravityDrop)
      {
         bulb->ProAdvanceMotionPath(elapsed);
         bulb->ProAdvanceTroughImpulse(elapsed);
         bulb->ProAdvanceSequenceSource(elapsed);
      }
   }
   for (B2SProSequenceGroup* group : groupStarts)
      group->RequestStart();
   for (B2SProSequenceGroup* group : groupRemoves)
      group->RequestRemove();
   for (auto& group : world.sequences)
      group->Update();
   for (auto& [name, group] : world.pivotGroups)
      group->Update(server);
   for (auto& [name, grid] : world.positionGrids)
      grid->Update(server, elapsed);

   // Launcher triggers (FirePhysicsLaunchers: any change to a non zero value launches)
   for (auto& ball : world.balls)
   {
      if (!ball->HasLauncher())
         continue;
      B2SBulb::ProRuntime& rt = ball->Ball().m_proRt;
      float value = 0.f;
      if (PollProTrigger(rt, server, static_cast<B2SProTriggerType>(ball->Ball().m_pro.physicsLauncher.triggerType), ball->Ball().m_pro.physicsLauncher.triggerId, value))
      {
         const bool on = value != 0.f;
         if (on != rt.triggerPrev)
         {
            rt.triggerPrev = on;
            if (on)
               ball->Launch();
         }
      }
   }

   // Physics clock: fixed 1 ms steps, flipper pivots and balls advanced together, work capped per frame
   if (!world.balls.empty())
   {
      constexpr double stepSeconds = 0.001;
      world.pendingSeconds += static_cast<double>(elapsed);
      const int steps = static_cast<int>(std::min(250.0, std::floor((world.pendingSeconds + 0.000000001) / stepSeconds)));
      for (int step = 0; step < steps; step++)
      {
         for (B2SBulb* pivot : world.pivots)
            pivot->ProAdvancePivot(static_cast<float>(stepSeconds));
         for (auto& ball : world.balls)
            ball->Step(static_cast<float>(stepSeconds));
      }
      world.pendingSeconds = std::max(0.0, world.pendingSeconds - steps * stepSeconds);
      for (auto& ball : world.balls)
         ball->Present();
   }
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
   case B2SRomIDType::Switch: groupId = PMPI_GROUP_SWITCH; break;
   default: return []() { /* No ROM source */ };
   }
   for (const StateSrcId& src : items)
   {
      if (src.id.resId != groupId)
         continue;
      for (unsigned int i = 0; i < src.nStates; i++)
      {
         const auto& def = src.stateDefs[i];
         if (def.mappingId != romId || def.GetState == nullptr)
            continue;
         // Lamps, solenoids and GI are published as floats; PinMAME publishes switches as bytes and mechs as
         // 32 bit integers (B2SPluginEventStream reads them the same way), read into the float probe
         if (def.dataFormat == CTLPI_STATE_FORMAT_FLOAT)
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
         if (def.dataFormat == CTLPI_STATE_FORMAT_UINT8 || def.dataFormat == CTLPI_STATE_FORMAT_INT32)
         {
            const bool isSwitch = groupId == PMPI_GROUP_SWITCH;
            const bool isByte = def.dataFormat == CTLPI_STATE_FORMAT_UINT8;
            return [this, value, def, romInverted, romValue, isSwitch, isByte]()
            {
               m_stateSources.With(
                  [this, value, &def, isSwitch, isByte](const std::vector<StateSrcId>&)
                  {
                     if (isByte)
                     {
                        uint8_t byteState = 0;
                        def.GetState(def.callContext, &byteState);
                        *value = isSwitch ? (byteState != 0 ? 1.f : 0.f) : static_cast<float>(byteState);
                     }
                     else
                     {
                        int32_t intState = 0;
                        def.GetState(def.callContext, &intState);
                        *value = static_cast<float>(intState);
                     }
                  });
               if (romValue > 0)
                  *value = (static_cast<int>(*value) == romValue) ? 1.f : 0.f;
               else if (romInverted)
                  *value = (*value != 0.f) ? 0.f : 1.f;
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

// B2S Pro ball artwork keeps its aspect when the backglass is stretched differently in X and Y (the reference's
// VisualArtworkBounds: PreservePhysicsArtworkAspect for physics balls scales by the smaller of the two axis
// scales, a gravity trough ball by the area preserving geometric mean), about the authored centre. Rolling
// motion path balls (a trough's balls) get the same treatment here where the reference stretches them with the
// canvas: a round ball stays round on a 16:9 panel showing 5:4 art. The rectangle is in source units.
static void PreserveBallAspect(const VPXRenderContext2D* ctx, const bool areaRule, float& x, float& y, float& w, float& h)
{
   const float sx = ctx->outWidth / ctx->srcWidth, sy = ctx->outHeight / ctx->srcHeight;
   if (sx <= 0.f || sy <= 0.f || std::abs(sx - sy) <= 1e-4f * std::max(sx, sy))
      return;
   const float k = areaRule ? std::sqrt(sx * sy) : std::min(sx, sy);
   const float nw = w * k / sx, nh = h * k / sy;
   x += (w - nw) * 0.5f;
   y += (h - nh) * 0.5f;
   w = nw;
   h = nh;
}

void B2SRenderer::RenderBulbs(VPXRenderContext2D* ctx, const B2SServer* server, const vector<std::unique_ptr<B2SBulb>>& bulbs, float elapsed, B2SDualMode dualMode, const int pass)
{
   if (pass != 1)
      UpdateProRuntime(bulbs, const_cast<B2SServer*>(server), elapsed);
   // B2S Pro: a visible dynamic front light whose base is not on a separate layer sits on the backglass below
   // every snippit: its artwork is drawn first (its lit overlay is drawn at its own place in the order)
   if (pass != 0)
      for (const auto& bulb : bulbs)
         if (IsProDynamicLight(*bulb) && bulb->m_pro.dynamicFrontShowLight && !bulb->m_pro.separateLightLayer && bulb->m_brightness > 0.f && bulb->m_image != nullptr
            && !(dualMode != B2SDualMode::Both && bulb->m_dualMode != B2SDualMode::Both && bulb->m_dualMode != dualMode))
         {
            const VPXTextureInfo* const tex = GetTextureInfo(bulb->m_image);
            ctx->DrawImage(ctx, bulb->m_image, bulb->m_lightColor.x, bulb->m_lightColor.y, bulb->m_lightColor.z, bulb->m_brightness, 0.f, 0.f, static_cast<float>(tex->width), static_cast<float>(tex->height),
               0.f, 0.f, 0.f, static_cast<float>(bulb->m_locationX), static_cast<float>(bulb->m_locationY), static_cast<float>(bulb->m_width), static_cast<float>(bulb->m_height));
         }
   for (const auto& bulb : bulbs)
   {
      if (dualMode != B2SDualMode::Both && bulb->m_dualMode != B2SDualMode::Both && bulb->m_dualMode != dualMode)
         continue; // Dual backglass: only render bulbs matching the active mode
      if (bulb->m_bakedIntoBackground)
         continue; // Drawn as part of the lit background image
      // B2S Pro: snippits behind the canvas are drawn before it so its alpha masks them (pass 0), the rest after (pass 1)
      if (pass != 2 && bulb->m_pro.snippitBehindCanvas != (pass == 0))
         continue;
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

      // B2S Pro: pivot animation or automatic rotation of a standard image around the authored pivot point,
      // and physics balls drawn where the simulation put them, turned by their rolling angle
      float pivotX = 0.5f, pivotY = 0.5f;
      float drawX = static_cast<float>(bulb->m_locationX), drawY = static_cast<float>(bulb->m_locationY);
      float drawW = static_cast<float>(bulb->m_width), drawH = static_cast<float>(bulb->m_height);
      const bool keepBallAspect = bulb->m_pro.physicsBall || bulb->m_pro.motionPathRollEnabled;
      float drawBrightness = bulb->m_brightness;
      // B2S Pro flasher pulse: each off to on edge shows the artwork for the authored duration, whatever the
      // state does meanwhile (a momentary pulse returns off at once, a held flasher still ends after it)
      if (bulb->m_pro.flasherPulseDuration > 0 && bulb->m_snippitType == B2SSnippitType::StandardImage)
      {
         B2SBulb::ProRuntime& rt = bulb->m_proRt;
         const bool on = bulb->m_brightness > 0.f;
         if (on && !rt.pulsePrevOn)
         {
            rt.pulseActive = true;
            rt.pulseRemainingS = static_cast<float>(bulb->m_pro.flasherPulseDuration) / 1000.f;
         }
         rt.pulsePrevOn = on;
         if (rt.pulseActive)
         {
            rt.pulseRemainingS -= elapsed;
            if (rt.pulseRemainingS <= 0.f)
               rt.pulseActive = false;
         }
         drawBrightness = rt.pulseActive ? 1.f : 0.f;
      }
      if (bulb->m_snippitType == B2SSnippitType::StandardImage)
      {
         // B2S Pro dynamic front light: never drawn as a plain image. A separate light layer draws its artwork
         // here (above the snippits), then the lit overlays of the snippits below it
         if (IsProDynamicLight(*bulb))
         {
            if (drawBrightness > 0.f)
            {
               if (bulb->m_pro.separateLightLayer && bulb->m_image != nullptr)
               {
                  const VPXTextureInfo* const tex = GetTextureInfo(bulb->m_image);
                  ctx->DrawImage(ctx, bulb->m_image, bulb->m_lightColor.x, bulb->m_lightColor.y, bulb->m_lightColor.z, drawBrightness, 0.f, 0.f, static_cast<float>(tex->width),
                     static_cast<float>(tex->height), 0.f, 0.f, 0.f, drawX, drawY, static_cast<float>(bulb->m_width), static_cast<float>(bulb->m_height));
               }
               RenderProDynamicLight(ctx, *bulb, bulbs, dualMode);
            }
            continue;
         }
         if (bulb->HasProPivot() || bulb->HasProRotation() || bulb->m_proRt.externalPivot)
         {
            rotation = bulb->m_proRt.angle;
            pivotX = bulb->m_pro.pivotX;
            pivotY = bulb->m_pro.pivotY;
            if (bulb->m_proRt.posValid) // A grid representative that also pivots
            {
               drawX = bulb->m_proRt.posX - static_cast<float>(bulb->m_width) * 0.5f;
               drawY = bulb->m_proRt.posY - static_cast<float>(bulb->m_height) * 0.5f;
            }
         }
         else if (bulb->m_proRt.posValid)
         {
            rotation = bulb->m_proRt.rollAngle;
            drawX = bulb->m_proRt.posX + bulb->m_proRt.impulseOffX - static_cast<float>(bulb->m_width) * 0.5f;
            drawY = bulb->m_proRt.posY + bulb->m_proRt.impulseOffY - static_cast<float>(bulb->m_height) * 0.5f;
            // A masked ball is drawn from its composed frame (artwork rolled and cut by the selection mask)
            if (bulb->m_proRt.physicsBall != nullptr && bulb->m_proRt.physicsBall->HasMask())
            {
               float fx, fy, fw, fh;
               if (const VPXTexture frame = bulb->m_proRt.physicsBall->MaskedFrame(fx, fy, fw, fh); frame != nullptr)
               {
                  float dx = fx, dy = fy, dw = fw, dh = fh;
                  PreserveBallAspect(ctx, bulb->m_pro.motionPathGravityDrop, dx, dy, dw, dh);
                  if (drawBrightness > 0.f)
                     ctx->DrawImage(ctx, frame, bulb->m_lightColor.x, bulb->m_lightColor.y, bulb->m_lightColor.z, drawBrightness, 0.f, 0.f, fw, fh, fw * 0.5f, fh * 0.5f, 0.f, dx, dy, dw, dh);
                  continue;
               }
            }
            if (keepBallAspect)
               PreserveBallAspect(ctx, bulb->m_pro.motionPathGravityDrop, drawX, drawY, drawW, drawH);
         }
      }

      // B2S Pro sequence feeder: the first member of a trough shows the next artwork at the path start
      if (bulb->m_proRt.seqSourceAnchor && bulb->m_proRt.seqSourceVisible && bulb->m_proRt.seqSourceImage != nullptr)
      {
         B2SProRect src = bulb->ProSequenceSourceRect();
         if (!src.IsEmpty())
         {
            if (keepBallAspect)
               PreserveBallAspect(ctx, bulb->m_pro.motionPathGravityDrop, src.x, src.y, src.w, src.h);
            const VPXTextureInfo* const srcTex = GetTextureInfo(bulb->m_proRt.seqSourceImage);
            ctx->DrawImage(ctx, bulb->m_proRt.seqSourceImage, bulb->m_lightColor.x, bulb->m_lightColor.y, bulb->m_lightColor.z, 1.f, 0.f, 0.f, static_cast<float>(srcTex->width),
               static_cast<float>(srcTex->height), static_cast<float>(srcTex->width) * 0.5f, static_cast<float>(srcTex->height) * 0.5f,
               bulb->m_pro.motionPathRollEnabled ? bulb->m_proRt.seqSourceRollAngle : 0.f, src.x, src.y, src.w, src.h);
         }
      }

      if (bulb->m_offImage && drawBrightness < 1.f)
      {
         const VPXTextureInfo* const bulbTex = GetTextureInfo(bulb->m_offImage);
         ctx->DrawImage(ctx, bulb->m_offImage, bulb->m_lightColor.x, bulb->m_lightColor.y, bulb->m_lightColor.z, 1.f,
            0.f, 0.f, static_cast<float>(bulbTex->width), static_cast<float>(bulbTex->height),
            static_cast<float>(bulbTex->width) * pivotX, static_cast<float>(bulbTex->height) * pivotY, rotation,
            drawX, drawY, drawW, drawH);
      }
      const VPXTexture artwork = bulb->m_proRt.displayImage != nullptr ? bulb->m_proRt.displayImage : bulb->m_image;
      if (artwork)
      {
         const VPXTextureInfo* const bulbTex = GetTextureInfo(artwork);
         ctx->DrawImage(ctx, artwork, bulb->m_lightColor.x, bulb->m_lightColor.y, bulb->m_lightColor.z, drawBrightness,
            0.f, 0.f, static_cast<float>(bulbTex->width), static_cast<float>(bulbTex->height),
            static_cast<float>(bulbTex->width) * pivotX, static_cast<float>(bulbTex->height) * pivotY, rotation,
            drawX, drawY, drawW, drawH);
      }
   }
}

void B2SRenderer::RenderScores(VPXRenderContext2D* ctx, B2SServer* server, const B2SScores& scores, float elapsed, const int pass)
{
   if (server == nullptr)
      return;
   // B2SHideScoreDisplays/animations only hide LED score displays, reel displays keep rendering
   const bool ledDisplaysHidden = server->AreScoreDisplaysHidden();
   // Two pass drawing for B2S Pro displays behind the canvas: pass 0 (before the canvas) updates the
   // states and draws those, pass 1 draws the others from the states already advanced; pass 2 does both
   const bool update = pass != 1;
   const auto drawsNow = [pass](const B2SScore& reel) { return pass == 2 || reel.m_pro.behindCanvas == (pass == 0); };

   vector<SegElementType> segTypes;
   vector<float> luminances;
   vector<VPXSegDisplayRenderStyle> styles;
   vector<VPXSegDisplayHint> hints;
   m_segSources.With([&segTypes, &styles, &hints, &luminances, server, update](const std::vector<SegSrcId>& segDisplays){
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
            if (update)
            {
               server->B2SSetScoreDigit(digitIndex, ret);
               // Keep the raw segment mask: script digits decode to value-only, but frame-fed displays
               // need the full mask (alphanumeric glyphs do not decode to a digit). Must be applied
               // after B2SSetScoreDigit which clears the mask.
               server->ApplyScoreSegments(digitIndex, bitState);
            }
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
         if (!drawsNow(reel))
         {
            if (reel.m_b2sPlayerNo != 0)
               playerScoreOffset[reel.m_b2sPlayerNo] += static_cast<size_t>(reel.m_digits);
            continue;
         }
         const int usedLEDType = server->GetUsedLEDType();
         const bool dream7 = (usedLEDType == 2) || (usedLEDType != 1 && reel.m_scoreType == B2SScoreRenderer::Dream7);
         RenderLegacyScoreDisplay(ctx, server, reel, dream7, segTypes, playerScoreText, playerScoreOffset);
         continue;
      }
      const bool draw = drawsNow(reel);

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
               if (update)
               {
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
               }
               shownDigit = st.Current();
               intermediate = st.Intermediate();
               const B2SReelImage* reelImage = m_b2s->m_reels.GetImage(reel.m_reelType, shownDigit, illuminated, reel.m_reelIlluImageSet);
               const VPXTexture texture = (reelImage && intermediate > 0 && intermediate <= static_cast<int>(reelImage->m_intermediates.size()))
                  ? reelImage->m_intermediates[static_cast<size_t>(intermediate - 1)]
                  : nullptr;
               if (draw && reelImage && (texture != nullptr || reelImage->m_image != nullptr))
               {
                  VPXTexture img = texture != nullptr ? texture : reelImage->m_image;
                  const VPXTextureInfo* texInfo = GetTextureInfo(img);
                  // B2S Pro: digit box of the artwork. KeepDigitAspect fits the image (or stretches it to
                  // ReelDigitWidth) inside the digit, centred, instead of filling it
                  float dx = x, dy = static_cast<float>(reel.m_locY), dw = width, dh = static_cast<float>(reel.m_height);
                  if (reel.m_pro.keepDigitAspect && texInfo != nullptr && texInfo->width > 0 && texInfo->height > 0)
                  {
                     const float aspect = static_cast<float>(texInfo->width) / static_cast<float>(texInfo->height);
                     if (reel.m_pro.reelDigitWidth > 0)
                        dw = std::min(width, static_cast<float>(reel.m_pro.reelDigitWidth));
                     else
                        dw = std::min(width, dh * aspect);
                     dh = std::min(static_cast<float>(reel.m_height), dw / aspect);
                     dw = dh * aspect;
                     dx = x + (width - dw) * 0.5f;
                     dy = static_cast<float>(reel.m_locY) + (static_cast<float>(reel.m_height) - dh) * 0.5f;
                  }
                  // B2S Pro 3D reel: the backlight follows the reel illumination lamp when there is one
                  if (reel.m_pro.reel3DEnabled)
                  {
                     const bool backlightOn = reel.m_reelIlluB2SID <= 0 || illuminated;
                     if (const VPXTexture drum = Reel3DTexture(img, std::max(1, static_cast<int>(std::lround(dw))), std::max(1, static_cast<int>(std::lround(dh))),
                            backlightOn ? reel.m_pro.reel3DBrightness : 0, backlightOn ? reel.m_pro.reel3DTemperature : 4000, reel.m_pro.reel3DDepth);
                        drum != nullptr)
                     {
                        img = drum;
                        texInfo = GetTextureInfo(img);
                     }
                  }
                  ctx->DrawImage(ctx, img, 1.f, 1.f, 1.f, 1.f, //
                     0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height), //
                     0.f, 0.f, 0.f, // No rotation
                     dx, dy, dw, dh);
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
            if (!draw)
               break;
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

      // B2S Pro 3D reel: one glass window overlay (inset shadow and reflection) across the whole display
      if (draw && reel.m_pro.reel3DEnabled && reel.m_scoreType == B2SScoreRenderer::Reel && reel.m_width > 2 && reel.m_height > 2)
         if (const VPXTexture overlay = Reel3DOverlay(reel, reel.m_width, reel.m_height); overlay != nullptr)
            ctx->DrawImage(ctx, overlay, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, static_cast<float>(reel.m_width), static_cast<float>(reel.m_height), 0.f, 0.f, 0.f,
               static_cast<float>(reel.m_locX), static_cast<float>(reel.m_locY), static_cast<float>(reel.m_width), static_cast<float>(reel.m_height));
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

   // B2S Pro: the canvas backing shows through transparent canvas pixels (legacy files show the black clear colour)
   if (m_b2s->m_hasCanvasBacking)
   {
      if (m_whiteTexture == nullptr)
      {
         const uint8_t white[4] = { 255, 255, 255, 255 };
         UpdateTexture(&m_whiteTexture, 1, 1, VPXTEXFMT_sRGBA8, white);
      }
      if (m_whiteTexture != nullptr)
         ctx->DrawImage(ctx, m_whiteTexture, m_b2s->m_canvasBacking.x, m_b2s->m_canvasBacking.y, m_b2s->m_canvasBacking.z, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f,
            0.f, // No rotation
            0.f, 0.f, m_b2sWidth, m_b2sHeight);
   }
   // B2S Pro: snippits and score displays behind the canvas, which then masks them through its transparent openings
   RenderBulbs(ctx, server, m_b2s->m_backglassIlluminations, elapsed, dualMode, 0);
   RenderScores(ctx, server, m_b2s->m_backglassScores, elapsed, 0);

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
   RenderBulbs(ctx, server, m_b2s->m_backglassIlluminations, elapsed, dualMode, 1);
   RenderScores(ctx, server, m_b2s->m_backglassScores, elapsed, 1);
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
   RenderBulbs(ctx, server, m_b2s->m_dmdIlluminations, elapsed, B2SDualMode::Both, 2); // Reference does not dual-filter score view bulbs
   RenderScores(ctx, server, m_b2s->m_dmdScores, elapsed, 2);
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
