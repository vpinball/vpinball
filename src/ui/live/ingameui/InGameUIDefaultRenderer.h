// license:GPLv3+

#pragma once

#include "InGameUIRenderer.h"
#include "InGameUIPane.h"
#include "imgui/imgui.h"

class Player;

namespace VPX::InGameUI
{

// Base InGameUI renderer, keeping the legacy ImGui layout: pages as slide-in
// windows stacked on the right, custom panes attached above the window, modal
// panes replacing the page, backdrop panes fullscreen behind it.
class InGameUIDefaultRenderer final : public InGameUIRenderer
{
public:
   explicit InGameUIDefaultRenderer(InGameUI& owner);

   void RenderPages(const vector<InGameUIPage*>& pages, float elapsedS) override;

private:
   void RenderPage(InGameUIPage& page, float elapsedS);
   void RenderAttachedPanes(InGameUIPage& page, const vector<InGameUIPane>& panes);
   void RenderInputActionPopup(InGameUIPage& page);
   void RenderToggle(const string& label, const ImVec2& size, bool& v);
   static void TextWithEllipsis(const string& text, float maxWidth);

   InGameUI& m_inGameUI;
   Player* const m_player;

   // Touch drag-scroll state (mobile): renderer interaction state, not page state
   bool m_isDraggingScroll = false;
   float m_dragScrollVelocity = 0.f; // px/s, positive = scroll down
   ImVec2 m_dragScrollStartPos = ImVec2(0.f, 0.f);
};

}
