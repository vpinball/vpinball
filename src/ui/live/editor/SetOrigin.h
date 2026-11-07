// license:GPLv3+

#pragma once

class Primitive;

namespace VPX::EditorUI
{

class EditorUI;

// Object > Set Origin (Blender style): moves the origin (position) of the selected mesh primitives without moving their geometry.
// The mesh vertices are shifted by the inverse transformed origin offset, which is exact for any rotation and non zero (also non
// uniform) scale, as the primitive transform is scale, rotation, translation applied to the object space vertices.
// Not applicable: point based parts (ramps, rubbers, walls, flashers), builtin (non mesh) primitives, primitives with a zero scale.
class SetOrigin final
{
public:
   enum class Mode
   {
      Geometry, // Centre of the bounding box (table space)
      GeometryBottom // Centre of the bounding box, at its lowest Z (parts standing on the playfield)
   };
   static bool CanApply(const EditorUI &ed);
   static void Apply(EditorUI &ed, Mode mode);

private:
   static vector<::Primitive *> SelectedPrimitives(const EditorUI &ed, int &skipped);
};

}
