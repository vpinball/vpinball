// license:GPLv3+

#include "core/stdafx.h"
#include "core/TableAudit.h"

#include "audio/AudioPlayer.h"
#include "core/editablereg.h"
#include "core/pinundo.h"
#include "parts/light.h"
#include "parts/Material.h"
#include "parts/pintable.h"
#include "parts/primitive.h"
#include "parts/Sound.h"
#include "parts/textbox.h"
#include "parts/timer.h"
#include "renderer/Texture.h"

#include <sstream>

namespace
{

// Undoable change of a set of fields of 'table', capturing the pre-fix values in the undo
// restorer and the post-fix values in the redo restorer (see PinUndo::MarkForAction)
template <typename Set> std::function<IEditable *(PinTable &table, PinUndo &undo)> TableFieldsFix(Set &&set, const std::function<std::vector<std::function<void()>>()> &snapshot)
{
   return [set = std::move(set), snapshot](PinTable &table, PinUndo &undo) -> IEditable *
   {
      undo.MarkForAction(
         [restore = snapshot()]()
         {
            for (const auto &restoreField : restore)
               restoreField();
         }, //
         [snapshot]() -> std::function<void()>
         {
            return [restore = snapshot()]()
            {
               for (const auto &restoreField : restore)
                  restoreField();
            };
         });
      set(table);
      return nullptr;
   };
}

// Creates a Timer part with the given name (the audit only offers this fix when the name
// is available). The returned part is registered to the player by the fix caller.
std::function<IEditable *(PinTable &table, PinUndo &undo)> CreateTimerFix(const string &timerName)
{
   return [timerName](PinTable &table, PinUndo &undo) -> IEditable *
   {
      IEditable *const timer = EditableRegistry::CreateAndInit(eItemTimer, &table, 0.f, 0.f);
      if (timer == nullptr)
         return nullptr;
      timer->GetIScriptable()->m_name = timerName; // Set before AddPart which registers it (and asserts it is set and unique)
      timer->m_timerEnabled = true;
      table.AddPart(timer);
      undo.MarkForCreate(timer);
      return timer;
   };
}

// Fix of a single field of a part's data (the part is serialized by MarkForUndo)
template <typename TPart, typename TData, typename TField> std::function<IEditable *(PinTable &table, PinUndo &undo)> PartFieldFix(TPart *part, TField TData::*field, TField value)
{
   return [part, field, value](PinTable &table, PinUndo &undo) -> IEditable *
   {
      undo.MarkForUndo(part);
      part->m_d.*field = value;
      return nullptr;
   };
}

}

TableAudit TableAudit::Run(PinTable &table)
{
   // Perform a simple table audit (disable lighting vs static, script reference of static parts, png vs webp, hdr vs exr,...)
   TableAudit audit;
   auto &items = audit.items;
   const auto addItem = [&items](TableAuditItem::Severity severity, const string &summary, const string &details = {}) -> TableAuditItem &
   {
      items.push_back({ .severity = severity, .summary = summary, .details = details.empty() ? summary : details });
      return items.back();
   };

   vector<string> functions;
   vector<string> identifiers;
   table.ParseScript(table.m_script_text, functions, identifiers,
      [&addItem](const string &functionName, int line)
      {
         TableAuditItem &item = addItem(TableAuditItem::Severity::Error, "Duplicate declaration of '" + functionName + "' in script at line " + std::to_string(line));
         item.scriptLine = line + 1; // 'line' counts lines from 0, the script editor is 1-based
      });

   if (FindIndexOf(identifiers, "execute"s) != -1)
      addItem(TableAuditItem::Severity::Warning, "Scripts seems to use the 'Execute' command. This command triggers computer security checks and will likely cause stutters during play.");

   // Glass height fixes: restore the default height of a new table (see PinTable::m_glassBottomHeight)
   const auto fixGlassHeight = TableFieldsFix([](PinTable &table) { table.m_glassBottomHeight = table.m_glassTopHeight = 210.f; }, [&table]() -> std::vector<std::function<void()>>
      { return { [&table, value = table.m_glassBottomHeight]() { table.m_glassBottomHeight = value; }, [&table, value = table.m_glassTopHeight]() { table.m_glassTopHeight = value; } }; });
   if (table.m_glassBottomHeight > table.m_glassTopHeight)
   {
      TableAuditItem &item = addItem(TableAuditItem::Severity::Warning, "Glass height seems invalid: bottom is higher than top");
      item.part = &table;
      item.fix = fixGlassHeight;
      item.fixLabel = "Reset glass height to default";
   }
   if (table.m_glassBottomHeight < INCHESTOVPU(2) || table.m_glassTopHeight < INCHESTOVPU(2))
   {
      TableAuditItem &item = addItem(TableAuditItem::Severity::Warning, "Glass height seems invalid: glass is below 2\"");
      item.part = &table;
      item.fix = fixGlassHeight;
      item.fixLabel = "Reset glass height to default";
   }

   if (table.m_ballSphericalMapping)
   {
      TableAuditItem &item
         = addItem(TableAuditItem::Severity::Warning, "Ball uses legacy 'spherical mapping', it will be rendered like a 2D object and therefore will look bad in VR, stereo or headtracking");
      item.part = &table;
      item.fix = TableFieldsFix([](PinTable &table) { table.m_ballSphericalMapping = false; },
         [&table]() -> std::vector<std::function<void()>> { return { [&table, value = table.m_ballSphericalMapping]() { table.m_ballSphericalMapping = value; } }; });
      item.fixLabel = "Disable legacy spherical mapping";
   }

   for (const string &fix : table.m_loadFixes)
      addItem(TableAuditItem::Severity::Warning, fix);

   // Search for inconsistencies in the table parts
   bool hasPulseTimer = false, hasPinMameTimer = false;
   for (const auto part : table.GetParts())
   {
      auto type = part->GetItemType();
      Primitive *const prim = type == eItemPrimitive ? (Primitive *)part : nullptr;
      Light *const light = type == eItemLight ? (Light *)part : nullptr;
      Textbox *const textbox = type == eItemTextbox ? (Textbox *)part : nullptr;

      // Referencing a static object from script (ok if it is for reading properties, not for writing)
      if (type == eItemPrimitive && prim->m_d.m_staticRendering && FindIndexOf(identifiers, prim->GetName()) != -1)
         addItem(TableAuditItem::Severity::Warning,
            "Primitive '" + prim->GetName() + "' seems to be referenced from the script while it is marked as static (most properties of a static object may not be modified at runtime).")
            .part
            = part;

      if (type == eItemTextbox && (textbox->m_d.m_isDMD || StrFindNoCase(textbox->m_d.m_text, "DMD"s) != string::npos))
         addItem(TableAuditItem::Severity::Warning, "legacy Textbox '" + textbox->GetName() + "' is used for DMD rendering. It should be replaced by a flasher to get better rendering.").part
            = part;

      if (type == eItemTimer)
      {
         const string name = lowerCase(((Timer *)part)->GetName());
         hasPulseTimer |= name == "pulsetimer";
         hasPinMameTimer |= name == "pinmametimer";
      }

      // Warning on very fast timers (lower than 5ms)
      switch (type)
      {
      case eItemBall:
      // eItemDecal
      case eItemPartGroup:
      // eItemPrimitive
      case eItemTextbox:
      case eItemDispReel:
      case eItemSurface:
      case eItemTimer:
      case eItemLight:
      case eItemRamp:
      case eItemPlunger:
      case eItemSpinner:
      case eItemTrigger:
      case eItemKicker:
      case eItemRubber:
      case eItemFlasher:
      case eItemLightSeq:
      case eItemHitTarget:
      case eItemBumper:
      case eItemFlipper:
      case eItemGate:
         if (part->m_timerEnabled && part->m_timerInterval != -1 && part->m_timerInterval != -2 && part->m_timerInterval < 17)
            addItem(TableAuditItem::Severity::Warning,
               "Part '" + part->GetName() + "' uses a timer with a very short period of " + std::to_string(part->m_timerInterval)
                  + "ms, below a 60FPS framerate. This will likely cause stutters and the table will not support 'frame pacing'.")
               .part
               = part;
         break;
      // eItemTable
      // eItemLightCenter
      // eItemDragPoint
      // eItemCollection
      default: break;
      }

      if (type == eItemPrimitive && prim->m_d.m_visible && prim->m_d.m_disableLightingBelow != 1.f && !prim->m_d.m_staticRendering
         && (!table.GetMaterial(prim->m_d.m_szMaterial)->m_bOpacityActive || table.GetMaterial(prim->m_d.m_szMaterial)->m_fOpacity == 1.f)
         && (table.GetImage(prim->m_d.m_szImage) == nullptr || table.GetImage(prim->m_d.m_szImage)->IsOpaque()))
      {
         TableAuditItem &item = addItem(
            TableAuditItem::Severity::Warning, "Primitive '" + prim->GetName() + "' uses translucency (lighting from below) while it is fully opaque. Translucency will be discarded.");
         item.part = part;
         item.fix = PartFieldFix(prim, &PrimitiveData::m_disableLightingBelow, 1.f);
         item.fixLabel = "Disable translucency";
      }

      if (type == eItemLight && light->m_d.m_intensity < 0.f)
      {
         TableAuditItem &item = addItem(TableAuditItem::Severity::Error, "Light '" + light->GetName() + "' has a negative intensity.");
         item.part = part;
         item.fix = PartFieldFix(light, &LightData::m_intensity, 0.f);
         item.fixLabel = "Clamp intensity to 0";
      }
      if (type == eItemLight && light->m_d.m_intensity_scale < 0.f)
      {
         TableAuditItem &item = addItem(TableAuditItem::Severity::Error, "Light '" + light->GetName() + "' has a negative intensity scale.");
         item.part = part;
         item.fix = PartFieldFix(light, &LightData::m_intensity_scale, 0.f);
         item.fixLabel = "Clamp intensity scale to 0";
      }

      // Disabled as this is now enforced in the rendering
      // Enabling translucency (light from below) won't work with static parts: otherwise the rendering will be different in VR/Headtracked vs desktop modes. It also needs a non opaque alpha.
      //if (type == eItemPrimitive && prim->m_d.m_disableLightingBelow != 1.f && prim->m_d.m_staticRendering)
      //   ss << ". Warning: Primitive '" << prim->GetName() << "' has translucency enabled but is also marked as static. Translucency will not be applied on desktop, and it will look different between VR/headtracked and desktop.\r\n";
      //if (type == eItemSurface && surf->m_d.m_disableLightingBelow != 1.f && surf->StaticRendering())
      //   ss << ". Warning: Wall '" << surf->GetName() << "' has translucency enabled but will be statically rendered (not droppable with opaque materials). Translucency will not be applied on desktop, and it will look different between VR/headtracked and desktop.\r\n";
   }

   // The flasher VideoCap API (capturing the window of an external application) is deprecated in favor of plugins and may be removed in a future version
   if (FindIndexOf(identifiers, "videocapwidth"s) != -1 || FindIndexOf(identifiers, "videocapheight"s) != -1 || FindIndexOf(identifiers, "videocapupdate"s) != -1)
   {
      // Name the flashers referenced as '<name>.VideoCap...' (whole name, case insensitive like VBScript), or the script for an indirect use
      const string script = lowerCase(table.m_script_text);
      bool attributed = false;
      for (const auto part : table.GetParts())
      {
         if (part->GetItemType() != eItemFlasher)
            continue;
         const string ref = lowerCase(string(part->GetName())) + ".videocap";
         for (size_t pos = script.find(ref); pos != string::npos; pos = script.find(ref, pos + 1))
         {
            if (pos > 0 && (isalnum(static_cast<unsigned char>(script[pos - 1])) || script[pos - 1] == '_'))
               continue; // Part of a longer name
            addItem(TableAuditItem::Severity::Error,
               "Flasher '" + part->GetName()
                  + "' uses the deprecated 'VideoCap' API (VideoCapWidth/VideoCapHeight/VideoCapUpdate). It is superseded by plugins and may be removed in a future version.")
               .part
               = part;
            attributed = true;
            break;
         }
      }
      if (!attributed)
         addItem(TableAuditItem::Severity::Error,
            "script uses the deprecated Flasher 'VideoCap' API (VideoCapWidth/VideoCapHeight/VideoCapUpdate). It is superseded by plugins and may be removed in a future version.");
   }

   if ((FindIndexOf(identifiers, "loadvpm"s) != -1) || (FindIndexOf(identifiers, "loadvpmalt"s) != -1))
   {
      if (!hasPinMameTimer)
      {
         TableAuditItem &item = addItem(TableAuditItem::Severity::Warning, "VPM controller is used but table is missing a Timer object named 'PinMAMETimer'.");
         if (table.IsNameUnique("PinMAMETimer"s))
         {
            item.fix = CreateTimerFix("PinMAMETimer"s);
            item.fixLabel = "Create 'PinMAMETimer' timer";
         }
      }
      if (FindIndexOf(identifiers, "vpminit"s) == -1)
         addItem(TableAuditItem::Severity::Warning, "VPM controller is used but vpmInit is not called. pause/resume/exit will likely exhibit bugs and physic outputs won't be supported.");
   }

   if (!hasPulseTimer && (FindIndexOf(identifiers, "vpmTimer"s) != -1))
   {
      TableAuditItem &item = addItem(TableAuditItem::Severity::Warning, "script uses 'vpmTimer' but table is missing a Timer object named 'PulseTimer'. vpmTimer will not work as expected.");
      if (table.IsNameUnique("PulseTimer"s))
      {
         item.fix = CreateTimerFix("PulseTimer"s);
         item.fixLabel = "Create 'PulseTimer' timer";
      }
   }

   // Only used to query sound informations: no need for the optional haptic output
   auto audioPlayer = std::make_unique<VPX::AudioPlayer>(table.GetSettings().GetPlayer_SoundDeviceBG(), table.GetSettings().GetPlayer_SoundDevice(),
      static_cast<VPX::SoundConfigTypes>(table.GetSettings().GetPlayer_Sound3D()), static_cast<VPX::BackglassConfigTypes>(table.GetSettings().GetPlayer_Sound3DBackglass()), ""s,
      VPX::SNDCFG_SND3D2CH);
   for (const auto sound : table.m_vsound)
   {
      auto specs = audioPlayer->GetSoundInformations(sound);
      if (specs.nChannels > 1 && sound->GetOutputTarget() == VPX::SNDOUT_TABLE)
         addItem(TableAuditItem::Severity::Error, "sound '" + sound->GetName() + "' is used for playfield physical sound but has multiple channels (not mono).").sound = sound;
   }
   audioPlayer = nullptr;

   const bool issueFound = std::ranges::any_of(items, [](const TableAuditItem &item) { return item.severity != TableAuditItem::Severity::Info; });
   if (!issueFound)
      items.push_back({ .severity = TableAuditItem::Severity::Info, .summary = "No issue identified.", .details = "No issue identified.", .bulletInReport = false });

   // Also output a log of the table file content to allow easier size optimization
   size_t totalSize = 0, totalGpuSize = 0;
   for (const auto sound : table.m_vsound)
   {
      //ss << "  . Sound: '" << sound->m_name << "', size: " << SizeToReadable(sound->m_cdata) << "\r\n";
      totalSize += sound->GetFileSize();
   }
   addItem(TableAuditItem::Severity::Info, "Total sound size: "s + SizeToReadable(totalSize));

   totalSize = 0;
   for (const auto image : table.m_vimage)
   {
      size_t imageSize = image->GetFileSize();
      size_t gpuSize = image->GetEstimatedGPUSize();
      //ss << "  . Image: '" << image->m_name << "', size: " << SizeToReadable(imageSize) << ", GPU mem size: " << SizeToReadable(gpuSize) << "\r\n";
      totalSize += imageSize;
      totalGpuSize += gpuSize;
   }
   addItem(TableAuditItem::Severity::Info, "Total image size: "s + SizeToReadable(totalSize) + " in VPX file, at least "s + SizeToReadable(totalGpuSize) + " in GPU memory when played");

   // Backdrop images referenced per view mode but not present in the table would render black.
   // An empty name or the "<None>" sentinel means no backdrop is set.
   static const char *const bgSetNames[NUM_BG_SETS] = { "Desktop", "Cabinet", "Full Single Screen" };
   for (unsigned int i = 0; i < NUM_BG_SETS; ++i)
      if (const string &bg = table.m_BG_image[i]; !bg.empty() && !StrCompareNoCase(bg, g_szNoneSelection) && table.GetImage(bg) == nullptr)
      {
         TableAuditItem &item = addItem(TableAuditItem::Severity::Warning, string(bgSetNames[i]) + " backdrop image '" + bg + "' is not present in the table (renders black)");
         item.part = &table;
         item.fix = TableFieldsFix([i](PinTable &table) { table.m_BG_image[i].clear(); },
            [&table, i]() -> std::vector<std::function<void()>> { return { [&table, i, value = table.m_BG_image[i]]() { table.m_BG_image[i] = value; } }; });
         item.fixLabel = "Clear missing backdrop reference";
      }

   int nPrimTris = 0, primMemSize = 0;
   for (const auto part : table.GetParts())
      if (part->GetItemType() == eItemPrimitive && ((Primitive *)part)->m_d.m_use3DMesh /* && ((Primitive *)part)->m_d.m_visible */)
      {
         primMemSize += (((Primitive *)part)->m_mesh.NumIndices() > 65536 ? 4 : 2) * (int)((Primitive *)part)->m_mesh.NumIndices();
         primMemSize += (int)((Primitive *)part)->m_mesh.NumVertices() * sizeof(Vertex3D_NoTex2);
         nPrimTris += (int)((Primitive *)part)->m_mesh.NumIndices() / 3;
      }
   addItem(
      TableAuditItem::Severity::Info, "Total number of faces used in primitives: "s + std::to_string(nPrimTris) + ", needing "s + SizeToReadable(primMemSize) + " in GPU memory when played");

   return audit;
}

string TableAudit::Format() const
{
   std::stringstream ss;
   for (const TableAuditItem &item : items)
   {
      if (item.bulletInReport)
      {
         ss << ". ";
         switch (item.severity)
         {
         case TableAuditItem::Severity::Error: ss << "Error: "; break;
         case TableAuditItem::Severity::Warning: ss << "Warning: "; break;
         case TableAuditItem::Severity::Info: break;
         }
      }
      ss << item.summary << "\r\n";
   }
   return ss.str();
}

bool TableAudit::HasErrors() const
{
   return std::ranges::any_of(items, [](const TableAuditItem &item) { return item.severity == TableAuditItem::Severity::Error; });
}
