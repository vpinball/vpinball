// license:GPLv3+

#pragma once

#include "math/vector.h"
#include "math/matrix.h"

class PinTable;

// Fixed distance between the playfield bottom and the end of the lockbar border, in cm.
// This is part of the (universal) cabinet model and could be made a table data if needed.
inline constexpr float kVRLockbarToPlayfield = 5.f;

namespace VPX
{
// Reference space a part's coordinates are expressed in (see PartGroupData::m_spaceReference)
enum class SpaceReference : int
{
   Inherit = 0, // Inherit space reference from parent (note that root defaults to Playfield reference space)
   Playfield = 1, // Relative to the room with scaling, height adjustment and playfield inclination applied (table design coordinate system, including the cabinet which moves with it)
   Room = 2, // Base space, aligned to (offsetted) real world, without any scaling (to match real world room in AR/VR)
};
}

// Defines how the scene is laid out between the playfield and the room: the authored cabinet
// geometry plus the (optionally user-adjusted) room positioning, lockbar target dimensions and
// playfield inclination, resolved into a transform for each reference space.
//
// Each reference space is described by a transform to a common 'scene root' frame: the anchored
// room frame, y-up, in VPU. In the desktop and the editor, the layout is the authored one (no user
// adjustments); in VR, the VRDevice pushes the session's room positioning and lockbar adjustments
// so that the layout matches the headset tracking. Transforms between any two spaces derive from
// the space to scene root transforms (see GetSpaceToPlayfieldTransform).
class SceneLayout final
{
public:
   // Cabinet geometry authored in the table file, all distances in VPU, angles in degrees
   struct TableGeometry
   {
      float playfieldWidth = 0.f;
      float playfieldDepth = 0.f;
      float groundToLockbarHeight = 0.f; // Height of the cabinet model's lockbar from its feet
      float glassBottomHeight = 0.f; // Height of the playfield bottom (below the glass) from the feet

      bool operator==(const TableGeometry &) const = default;
   };

   // User-adjustable scene setup. Distances in cm, angles in degrees.
   // The default setup describes the authored layout (native cabinet dimensions, no room positioning)
   struct Setup
   {
      // Room positioning: corrects imprecise headset room positioning (tracking origin, floor height,
      // or desired standing spot). Global, persisted between sessions.
      Vertex3Ds roomOffset { 0.f, 0.f, 0.f }; // Shifts the whole anchored assembly (cabinet and room together)
      float roomYaw = 0.f;

      // Per-table offsets, compensating imprecise cabinet geometry definitions
      Vertex3Ds tableOffset { 0.f, 0.f, 0.f }; // y: playfield end to lockbar distance, z: glass height

      // Lockbar target dimensions: custom (measured on the user's real world cabinet) or table authored
      bool overrideLockbarWidth = false;
      bool overrideLockbarHeight = false;
      float customLockbarWidth = 57.f;
      float customLockbarHeight = 85.f;

      float seatedTilt = 0.f; // Cabinet tilt toward the player for seated play
      float liveSlope = 0.f; // Effective playfield slope (authored slope possibly adjusted by the user)

      bool operator==(const Setup &) const = default;
   };

   // Space to scene root transform, stored with its uniform scale. The transform may be non
   // orthonormal (cabinet scaling): ComposeView keeps the view orthonormal by folding the
   // scale out of the view and into the projection.
   struct SpaceTransform
   {
      Matrix3D m_transform;
      float m_scale = 1.f;
   };

   explicit SceneLayout(const PinTable *table = nullptr)
      : m_table(table)
   {
   }

   void Update(const TableGeometry &geometry, const Setup &setup); // Recompute all the transforms
   void EnsureUpdated() const; // Lazily recompute when the table geometry or the setup changed (no-op when created without a table)

   // Space to scene root transform (the anchored room frame, y-up, VPU)
   const SpaceTransform &GetSpaceTransform(VPX::SpaceReference space) const
   {
      EnsureUpdated();
      return m_spaces[static_cast<int>(space)];
   }
   // Space to playfield transform, derived from the space to scene root transforms (identity for the
   // playfield space): this is the frame the simulation and the UI colliders are expressed in
   const SpaceTransform &GetSpaceToPlayfieldTransform(VPX::SpaceReference space) const
   {
      EnsureUpdated();
      return m_spaceToPlayfield[static_cast<int>(space)];
   }
   float GetCabinetScale() const
   {
      EnsureUpdated();
      return m_cabinetScale;
   }
   float GetTargetLockbarHeight() const
   {
      EnsureUpdated();
      return m_targetLockbarHeight;
   }

   // Compose a camera pose with a space transform into an orthonormal view: the space transform may
   // be non orthonormal so its uniform scale is compensated inside the view and folded back into the
   // projection to preserve the net transform:
   //   view = spaceTransform * camView * inv(spaceScale), proj = spaceScale * camProj
   static Matrix3D ComposeView(const SpaceTransform &space, const Matrix3D &camView, float &projScale)
   {
      projScale = space.m_scale;
      return space.m_transform * camView * Matrix3D::MatrixScale(1.f / space.m_scale);
   }

   void SetTable(const PinTable *table) { m_table = table; } // Rebind after a copy (see PinTable::CopyForPlay)

private:
   void Recompute(const TableGeometry &geometry, const Setup &setup) const; // Writes the mutable caches below

   const PinTable *m_table = nullptr; // Table the authored geometry is lazily read from (see EnsureUpdated)
   mutable bool m_dirty = true;
   mutable Setup m_setup;
   mutable TableGeometry m_geometry;
   mutable SpaceTransform m_spaces[3]; // Space -> scene root, indexed by VPX::SpaceReference (Inherit slot unused)
   mutable SpaceTransform m_spaceToPlayfield[3]; // Space -> playfield space (derived)
   mutable float m_cabinetScale = 1.f;
   mutable float m_targetLockbarHeight = 0.f;
};
