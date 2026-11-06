// license:GPLv3+

#pragma once

#include "EditorAreaView.h"

namespace VPX::EditorUI
{

class EditorUI;
class EditorUIPart;

// Panel listing the view setups, materials, images, sounds, render probes
// and the scene parts tree, used to select the edited object. Hosted by a
// leaf of the area layout (several instances may coexist).
class OutlinerPanel : public EditorAreaView
{
public:
   explicit OutlinerPanel(EditorUI &editor)
      : EditorAreaView(editor)
   {
   }

   AreaType GetType() const override { return AreaType::Outliner; }
   void RenderPane(const AreaRect &rect, int leafId) override;

   // Default width of a newly hosted panel in logical pixels (scaled by the UI DPI)
   static constexpr float PaneWidth = 200.f;

   // Anchor part for shift+click range selection (the last part clicked, here or in the 3D view):
   // shared by all outliner instances since the selection it ranges over is shared
   static std::shared_ptr<EditorUIPart> m_anchor;
   static std::shared_ptr<EditorUIPart> m_savedAnchor; // Anchor saved across live-table switches

private:
   void SelectOutlinerRange(const std::shared_ptr<EditorUIPart> &part); // Select the parts between the anchor and the given part
   bool MatchesFilter(const string &name) const;

   // Render a resource list section (images, sounds, materials) with ctrl/shift multi selection.
   // multiSel holds all the selected items, anchor is the last clicked one for range selection.
   template <class T> void RenderResourceList(const char *label, vector<T *> &items, vector<T *> &multiSel, T *&anchor, const std::function<string(T *)> &nameOf);
   string m_filter;
   bool m_syncToSelection = false; // When enabled, the tree is expanded & scrolled to reveal the selected part on selection change
   std::shared_ptr<EditorUIPart> m_lastSyncedPart; // Last selection revealed in the tree while syncing
};

}
