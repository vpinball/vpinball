// license:GPLv3+

#pragma once

#include "math/vector.h"

#include <functional>

class IEditable;

namespace VPX::EditorUI
{

class EditorUI;

// Add Part > Round Wall: a wall whose outline is a regular polygon (12 faces by default), e.g. a post. Clicking places its center, dragging
// before releasing the mouse sets its radius (a click alone gives the default radius). Radius, faces and center stay adjustable with Adjust
// Last Operation (F9), inside the undo record of the creation. The points are plain drag points. See CircleMath for the geometry.
class CircleWallOp final
{
public:
   static void Arm(EditorUI &ed); // Arms the add part mode for a round wall (part picker)
   static void Disarm(); // Another part type is picked
   static void OnCreated(EditorUI &ed, IEditable *part, const Vertex2D &center); // A wall was placed: if armed, makes it round and starts the radius drag
   static void Drag(EditorUI &ed, const Vertex2D &pos); // Mouse position on the table plane while the placing button is held (viewport and blueprint panes)
   static void Reapply(EditorUI &ed); // Adjust Last Operation
   static bool RenderParameters(const std::function<void(const char *)> &row);

private:
   static bool IsLive(const EditorUI &ed);
   static void Rebuild(EditorUI &ed);
};

}
