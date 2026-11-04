// license:GPLv3+

#pragma once

#include "EditorAreaView.h"

class RenderView;

namespace VPX::EditorUI
{

// 3D View pane: a leaf hosting a live render of the table. Each instance owns its own
// camera and rendering setup (added in a later step: for now it draws the header row
// only and the scene shows through the leaf rect).
class ViewportPane final : public EditorAreaView
{
public:
   explicit ViewportPane(EditorUI& editor)
      : EditorAreaView(editor)
   {
   }

   AreaType GetType() const override { return AreaType::Viewport; }
   void RenderPane(const AreaRect& rect, int leafId) override;
   void OnUnhosted() override;
};

}
