// license:GPLv3+

#pragma once

#include "imgui/imgui.h"

namespace VPX::InGameUI
{

// A piece of page-owned custom UI rendered outside the standard item list (cabinet
// diagram, sensor plots, previews, backdrop, modal) or embedded inside it. Pages
// declare their panes each frame through CollectPanes(); the renderer measures then
// places them. The default renderer keeps the legacy layout: backdrops fullscreen
// behind the page, attached panes stacked above the page window, a modal pane
// replacing the page entirely, embedded panes inline at the end of the item list.
struct InGameUIPane
{
   enum class Placement
   {
      Attached, // Bound to the page window (default renderer stacks it above it)
      Backdrop, // Fullscreen, drawn behind the page window
      Modal, // Fullscreen, replaces the page window entirely
      Embedded, // Drawn inline at the end of the page's item list (rect is the remaining item region)
      Header // Drawn at the top of the page's item list, scrolling with it (rect is offered the content width and a height share)
   };
   Placement placement = Placement::Attached;
   string id;

   // Desired size inside the offered bounds (UI points); used for Attached panes
   std::function<ImVec2(float availW, float availH)> measure;

   // Draws the pane inside rect (x, y, w, h, UI points): draw-list primitives, a
   // borderless window positioned at rect, render-device draws — page's choice
   std::function<void(const ImVec4& rect)> render;
};

}
