// license:GPLv3+

#pragma once

#include "math/vector.h"

#include <functional>

class DragPoint;
class DragPointCurve;

namespace VPX::EditorUI
{

class EditorUI;
class EditorUIPart;

// Drag point curve operations of the point edit mode (Blender's mesh edit tools, e.g. bevel): a tool rebuilds the edited curve from its
// points before the operation and the selected ones, with its own parameters. The operation is one undo step, and it stays adjustable
// (Adjust Last Operation, F9) as long as it is the last undo step and the point selection is the one it produced: adjusting rebuilds the
// curve again from the start points.
// Tools are registered by their own source file (static instance and CurveEditOp::Register), the menus and the shortcut handling list them.
class CurveEditOp final
{
public:
   // Copy of a drag point (the generated curves are made of plain drag points)
   struct PointState
   {
      Vertex3Ds pos { 0.f, 0.f, 0.f };
      float texCoord = 0.f;
      bool smooth = false;
      bool slingshot = false;
      bool autoTexture = true;
      bool uiLocked = false;
      bool uiVisible = true;
   };

   class Tool
   {
   public:
      virtual ~Tool() = default;
      virtual const char *GetName() const = 0; // Menu item, undo label and adjust panel title
      virtual int GetShortcut() const { return 0; } // ImGuiKeyChord, 0 for none
      // Called when the tool is applied, before the first Generate (not when the operation is adjusted): checks the selection and sets
      // the per operation defaults of the parameters. Returns false with a message in error (if not empty) to refuse the operation
      virtual bool Start(const EditorUIPart & /*part*/, const vector<PointState> & /*points*/, const vector<int> & /*sel*/, bool /*closed*/, string & /*error*/) { return true; }
      // Builds the curve from the points before the operation and the (sorted) indices of the selected points that are not locked.
      // Fills outSel with the indices of the points to select afterwards. Returns false if nothing changes
      virtual bool Generate(const EditorUIPart &part, const vector<PointState> &points, const vector<int> &sel, bool closed, vector<PointState> &out, vector<int> &outSel) = 0;
      // Adjust Last Operation widgets of the tool parameters (row() draws the label column), true if one changed
      virtual bool RenderParameters(const EditorUIPart &part, const std::function<void(const char *)> &row) = 0;
   };

   static bool Register(Tool *tool);
   static const vector<Tool *> &GetTools();

   static bool CanApply(const EditorUI &ed);
   static void Apply(EditorUI &ed, Tool &tool);
   static void Reapply(EditorUI &ed); // Rebuilds the curve of the last operation from its start points (Adjust Last Operation)

   static void RenderMenuItems(EditorUI &ed); // Menu items of the registered tools (point edit menus)
   static bool HandleShortcuts(EditorUI &ed); // Applies the tool whose shortcut is pressed, true if one was

   static bool IsClosed(const EditorUIPart &part); // Ramps are open curves, the other curve parts are closed loops

private:
   static vector<Tool *> &Tools();
   static void Rebuild(EditorUI &ed);
};

}
