// license:GPLv3+

#include "core/stdafx.h"
#include "core/SceneLayout.h"

#include "core/def.h"
#include "parts/pintable.h"

// Computes the space to scene root transforms of the 2 space references (room, playfield) from the
// authored cabinet geometry and the session adjustments (room positioning, target lockbar dimensions).
//
// The anchored assembly (feet + cabinet + playfield, a single rigid body) is placed so that its
// lockbar lands at the target lockbar height above the room ground, scaled so that its lockbar
// matches the target width, and inclined by the effective playfield slope.
//
// roomOffset corrects imprecise headset room positioning (tracking origin, floor height, standing
// spot): it is applied as a rigid translation of the room frame shared by all the spaces (after the
// playfield slope and the cabinet scale), so that the whole anchored assembly (room furniture and
// cabinet included) shifts together.
void SceneLayout::Recompute(const TableGeometry &geometry, const Setup &setup) const
{
   m_geometry = geometry;
   m_setup = setup;
   m_dirty = false;

   // Update fixed scaling, considering lockbar size to be the width of the playfield + 2"1/4
   // When not overriding the lockbar width, the cabinet is rendered at its native size
   const float tableWidth = VPUTOCM(geometry.playfieldWidth) + 2.25f * 2.54f;
   m_cabinetScale = setup.overrideLockbarWidth ? clamp(setup.customLockbarWidth / tableWidth, 0.1f, 2.0f) : 1.f;

   // The users define in their settings the real world height where they want the top of the lockbar to be.
   // The real world playfield height is then computed by removing the glass distance at the playfield bottom (typically 2 to 3").
   // The table offset allows to slightly adjust this height on a per table basis.
   // In the end Playfield height = tableOffset.z (user adjustment) + lockbar height (target) - glass distance * scale
   // When not overriding the lockbar height, the cabinet stands at its native (scaled) height
   const float lockbarHeight = setup.overrideLockbarHeight ? setup.customLockbarHeight : m_cabinetScale * VPUTOCM(geometry.groundToLockbarHeight);
   m_targetLockbarHeight = lockbarHeight;

   // Adjust coord from RH to LH system, and use the playfield bottom center as the pivot
   const Matrix3D coords = Matrix3D::MatrixScale(1.f, -1.f, 1.f) // RH to LH system
      * Matrix3D::MatrixTranslate(-geometry.playfieldWidth * 0.5f, geometry.playfieldDepth, 0.f); //

   // Apply the table offset and the room positioning correction inside the room frame, as rigid
   // translations the whole anchored assembly (room and cabinet included) shifts with. They are
   // expressed in the table oriented frame (x: lateral, y: front/back, z: up), the view orientation
   // mapping them to world axes (y: up) as it aligns the room to the view.
   const Matrix3D adjustRoom = //
      Matrix3D::MatrixTranslate(0.f, CMTOVPU(setup.tableOffset.y + kVRLockbarToPlayfield), CMTOVPU(setup.tableOffset.z)) // Table authoring adjustment (and fixed lockbar distance)
      * Matrix3D::MatrixTranslate(-CMTOVPU(setup.roomOffset.x), CMTOVPU(setup.roomOffset.y), CMTOVPU(setup.roomOffset.z)) // Headset error correction
      * Matrix3D::MatrixRotateZ(ANGTORAD(setup.roomYaw)) //
      * Matrix3D::MatrixRotateX(ANGTORAD(-90.f)); //

   // The table defines the height of the lockbar of its cabinet model, as well as the playfield base inclination.
   // This allows to fit the cabinet & playfield models to the real world space.
   // If user adjust the inclination, then the cab is rotated as it would in real life (still missing the legs stretching a bit using caster adjustments)
   // Seated play tilt rotates the whole cabinet (feet and playfield included) around the lockbar axis to face the player, the room staying upright
   // Note that the cabinet is authored with the playfield so tilted against it when authoring
   const Matrix3D playfield = coords * Matrix3D::MatrixTranslate(0.f, 0.f, -geometry.glassBottomHeight) // Apply glass gap perpendicular to the playfield
      * Matrix3D::MatrixScale(m_cabinetScale) // Adjust cabinet size to the target real world cabinet
      * Matrix3D::MatrixRotateX(ANGTORAD(setup.liveSlope + setup.seatedTilt)) // Tilt to playfield slope (and eventual seated play)
      * Matrix3D::MatrixTranslate(0.f, 0.f, CMTOVPU(lockbarHeight)) // finally apply target lockbar height in the room frame
      * adjustRoom;

   // Room does not apply the cabinet scaling nor any inclination, as it is the real world room, just the offsets & coords adjustments
   const Matrix3D room = coords * adjustRoom;

   m_spaces[static_cast<int>(VPX::SpaceReference::Playfield)] = { playfield, m_cabinetScale };
   m_spaces[static_cast<int>(VPX::SpaceReference::Room)] = { room, 1.f };
   // Inherit is never resolved (PartGroup::GetReferenceSpace resolves it to Playfield at the root): map it to playfield anyway
   m_spaces[static_cast<int>(VPX::SpaceReference::Inherit)] = m_spaces[static_cast<int>(VPX::SpaceReference::Playfield)];

   // Derived space -> playfield transforms (the playfield space itself maps to identity): the other
   // spaces are expressed in the playfield space so their scale is the cabinet scale inverse
   const Matrix3D invPlayfield = Matrix3D::MatrixInverse(playfield);
   for (int s = 0; s < 3; s++)
      m_spaceToPlayfield[s] = { m_spaces[s].m_transform * invPlayfield, m_spaces[s].m_scale / m_cabinetScale };
}

void SceneLayout::Update(const TableGeometry &geometry, const Setup &setup) { Recompute(geometry, setup); }

// Lazily recompute the transforms when the setup changed or the table's authored geometry did
// (the table fields are public so edits cannot be hooked). The stored setup is kept as is, except
// the live slope which is always refreshed from the table.
void SceneLayout::EnsureUpdated() const
{
   if (m_table == nullptr)
      return; // Driven through explicit Update calls (tests)
   const TableGeometry geometry { m_table->m_right - m_table->m_left, m_table->m_bottom - m_table->m_top, m_table->m_groundToLockbarHeight, m_table->m_glassBottomHeight };
   const float liveSlope = m_table->GetPlayfieldSlope();
   if (!m_dirty && geometry == m_geometry && liveSlope == m_setup.liveSlope)
      return;
   Setup setup = m_setup;
   setup.liveSlope = liveSlope;
   Recompute(geometry, setup);
}
