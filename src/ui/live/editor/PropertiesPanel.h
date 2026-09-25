// license:GPLv3+

#pragma once

#include <optional>

class Material;
class Texture;
class RenderProbe;
struct WhereUsedInfo;
namespace VPX
{
class Sound;
struct SoundSpec;
}

namespace VPX::EditorUI
{

class EditorUI;
class PropertyPane;

// Right side panel displaying the properties of the current selection (table,
// camera view setup, image, material, render probe, sound or editable part).
class PropertiesPanel
{
public:
   explicit PropertiesPanel(EditorUI &editor);
   ~PropertiesPanel();

   void Render(float topBarHeight);

   // Width of the panel in logical pixels (scaled by the UI DPI)
   static constexpr float PaneWidth = 280.f;

private:
   void TableProperties(PropertyPane &props);
   void ImageProperties(PropertyPane &props, Texture *image);
   void RenderProbeProperties(PropertyPane &props, RenderProbe *probe);
   void CameraProperties(PropertyPane &props, int bgSet);
   void MaterialProperties(PropertyPane &props, Material *material);
   void SoundProperties(PropertyPane &props, VPX::Sound *sound);

   // Resource actions offered at the top of the image/sound/material property panes,
   // matching the actions of the corresponding Win32 manager dialogs
   void ImageActions(PropertyPane &props);
   void SoundActions(PropertyPane &props);
   void MaterialActions(PropertyPane &props);
   void RequestConfirm(const string &message, const std::function<void()> &action);
   void RequestWhereUsed(bool images);
   void RenderPopups();

   EditorUI &m_editor;
   bool m_selectLiveTab = true;

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

   // Where-used popup content (computed when the popup is opened)
   vector<WhereUsedInfo> m_whereUsed;
   bool m_whereUsedImages = false;

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
