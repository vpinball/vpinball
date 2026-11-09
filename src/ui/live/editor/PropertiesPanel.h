// license:GPLv3+

#pragma once

#include <optional>

#include "audio/AudioPlayer.h"
#include "imgui/imgui.h"
#include "EditorAreaView.h"

class Collection;
class IEditable;
class Material;
class Texture;
class RenderProbe;
struct WhereUsedInfo;
namespace VPX
{
class Sound;
}

namespace VPX::EditorUI
{

class EditorUI;
class PropertyPane;

// Panel displaying the properties of the current selection (table, camera view
// setup, image, material, render probe, sound or editable part). Hosted by a
// leaf of the area layout (several instances may coexist).
class PropertiesPanel : public EditorAreaView
{
public:
   explicit PropertiesPanel(EditorUI &editor);
   ~PropertiesPanel();

   AreaType GetType() const override { return AreaType::Properties; }
   void RenderPane(const AreaRect &rect, int leafId) override;
   void AppendStatusHints(vector<StatusHint> &hints) const override;

   // Default width of a newly hosted panel in logical pixels (scaled by the UI DPI)
   static constexpr float PaneWidth = 280.f;

   // Name of the main material of a part (empty when the type has none), e.g. for the outliner's 'm:' filter token
   static string PartMaterial(const IEditable *part);

private:
   void TableProperties(PropertyPane &props);
   void ImageProperties(PropertyPane &props, Texture *image);
   void RenderProbeProperties(PropertyPane &props, RenderProbe *probe);
   void CameraProperties(PropertyPane &props, int bgSet);
   void MaterialProperties(PropertyPane &props, Material *material);
   void SoundProperties(PropertyPane &props, VPX::Sound *sound);
   void CollectionProperties(PropertyPane &props, Collection *collection);

   // Rebuilds renderables & physics of the parts modified through the pane: the edited one, plus all
   // co-edited parts of a multi-edit selection (also resyncing their editor visibility overrides)
   void RefreshEditedParts(PropertyPane &props);

   // Resource actions offered at the top of the image/sound/material property panes,
   // matching the actions of the corresponding Win32 manager dialogs
   void ImageActions(PropertyPane &props);
   void SoundActions(PropertyPane &props);
   void MaterialActions(PropertyPane &props);
   void RequestConfirm(const string &message, const std::function<void()> &action);
   void UsersSection(PropertyPane &props, const vector<WhereUsedInfo> &whereUsed);
   void RenderPopups();
   bool m_selectLiveTab = true;
   ImGuiID m_prevActiveId = 0; // Item active on the previous frame (a different item taking the interaction ends the undo chain)
   size_t m_editChainRecord = 0; // Undo count just after the current interaction's record was kept (it is still the latest record while equal)

   // Results of the asynchronous SDL file dialogs used by the resource actions. Since the dialogs are
   // asynchronous, the item selection at the time the action was requested is captured along the way.
   std::shared_ptr<vector<string>> m_pendingImageImport;
   std::shared_ptr<string> m_pendingImageReimport;
   Texture *m_pendingImageReimportTarget = nullptr;
   std::shared_ptr<string> m_pendingImageExport;
   vector<Texture *> m_pendingImageExportSel;
   std::shared_ptr<vector<string>> m_pendingSoundImport;
   std::shared_ptr<string> m_pendingSoundReimport;
   VPX::Sound *m_pendingSoundReimportTarget = nullptr;
   std::shared_ptr<string> m_pendingSoundExport;
   vector<VPX::Sound *> m_pendingSoundExportSel;
   std::shared_ptr<string> m_pendingMaterialImport;
   std::shared_ptr<string> m_pendingMaterialExport;
   vector<Material *> m_pendingMaterialExportSel;

   // Pending destructive action confirmation popup
   string m_confirmMessage;
   std::function<void()> m_confirmAction;

   // When enabled, exported resource files are named after the object name instead of the import file name
   bool m_exportUseNames = false;
   // Last resource action result displayed in the actions section (e.g. files not found on reimport)
   string m_actionStatus;

   // Sound preview state: the sound whose playback was started from the pane, and whether
   // playback was actually observed (PlaySound commands are dispatched asynchronously)
   VPX::Sound *m_playingSound = nullptr;
   bool m_playingSoundObserved = false;
   VPX::Sound *m_soundInfoFor = nullptr;
   std::optional<VPX::SoundSpec> m_soundInfo;
};

}
