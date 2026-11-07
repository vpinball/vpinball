// license:GPLv3+

#pragma once

#include "AreaLayout.h"

namespace VPX::EditorUI
{

class EditorUI;

// One status bar helper message of the bottom bar: a [key chip] followed by its action label (e.g. {ICON_LC_MOUSE_LEFT, "Select"})
struct StatusHint
{
   string key, label;
};

// Name of the platform modifier key used in status hint chips (ImGui maps Cmd to Ctrl on macOS)
#ifdef __APPLE__
inline constexpr const char* StatusModKey = "Cmd";
#else
inline constexpr const char* StatusModKey = "Ctrl";
#endif

// A view hosted by one leaf of the area layout tree (Outliner, Properties, 3D View, ...).
// One instance per leaf: created by the editor when the leaf appears, destroyed when the
// leaf is removed or retyped. The view draws the whole pane window at the leaf's rect,
// including its header row which starts with the shared editor-type switcher button.
class EditorAreaView
{
public:
   explicit EditorAreaView(EditorUI& editor)
      : m_editor(editor)
   {
   }
   virtual ~EditorAreaView() = default;

   virtual AreaType GetType() const = 0;

   // Draws the pane window at the given rect (called once per frame for visible leaves).
   // leafId is the stable id of the hosting leaf (for ImGui ids and layout edits).
   virtual void RenderPane(const AreaRect& rect, int leafId) = 0;

   // Called once when the leaf stops hosting this view (closed, retyped or unhosted by a
   // layout switch): the place to release GPU resources before the instance is dropped
   virtual void OnUnhosted() { }

   // The view's persisted setup, saved into / restored from the hosting leaf's opaque data
   // payload (e.g. camera for the 3D panes, zoom & offset for the blueprint pane)
   virtual nlohmann::json SaveSetup() const { return nlohmann::json::object(); }
   virtual void LoadSetup(const nlohmann::json& j) { }

   // Helper messages of the bottom status bar while this pane's context is active or its
   // rect is hovered (Blender status bar): appends [key chip] label pairs, most relevant first
   virtual void AppendStatusHints(vector<StatusHint>& hints) const { }

protected:
   EditorUI& m_editor;
};

}
