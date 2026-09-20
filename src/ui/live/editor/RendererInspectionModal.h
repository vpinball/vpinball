// license:GPLv3+

#pragma once

namespace VPX::EditorUI
{

class EditorUI;

// Modal dialog allowing to inspect a single render pass (debug mode only)
class RendererInspectionModal
{
public:
   explicit RendererInspectionModal(EditorUI &editor);

   void Show() { m_visible = true; }
   void Close();
   bool IsVisible() const { return m_visible; }

   void Render();

private:
   EditorUI &m_editor;
   bool m_visible = false;
   int m_passSelection;
};

}
