// license:GPLv3+

#pragma once

namespace VPX::EditorUI
{

class EditorUI;

// Left side panel listing the view setups, materials, images, render probes and
// the scene parts tree, used to select the edited object.
class OutlinerPanel
{
public:
   explicit OutlinerPanel(EditorUI &editor)
      : m_editor(editor)
   {
   }

   void Render(float topBarHeight);

private:
   bool MatchesFilter(const string &name) const;

   EditorUI &m_editor;
   string m_filter;
};

}
