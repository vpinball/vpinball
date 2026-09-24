// license:GPLv3+

#pragma once

namespace VPX::EditorUI
{

class EditorUI;
class EditorUIPart;

// Left side panel listing the view setups, materials, images, sounds, render probes
// and the scene parts tree, used to select the edited object.
class OutlinerPanel
{
public:
   explicit OutlinerPanel(EditorUI &editor)
      : m_editor(editor)
   {
   }

   void Render(float topBarHeight);

   // Width of the panel in logical pixels (scaled by the UI DPI)
   static constexpr float PaneWidth = 200.f;

private:
   bool MatchesFilter(const string &name) const;

   // Render a resource list section (images, sounds, materials) with ctrl/shift multi selection.
   // multiSel holds all the selected items, anchor is the last clicked one for range selection.
   template <class T> void RenderResourceList(const char *label, vector<T *> &items, vector<T *> &multiSel, T *&anchor, const std::function<string(T *)> &nameOf);

   EditorUI &m_editor;
   string m_filter;
   bool m_syncToSelection = false; // When enabled, the tree is expanded & scrolled to reveal the selected part on selection change
   std::shared_ptr<EditorUIPart> m_lastSyncedPart; // Last selection revealed in the tree while syncing
};

}
