// license:GPLv3+

#include "core/stdafx.h"
#include "VRSpaces.h"

#include "core/def.h"

// Computes the space to world transforms of the 2 VR space references (room, playfield) from the
// headset room positioning and the target lockbar dimensions.
//
// The anchored assembly (feet + cabinet + playfield, a single rigid body) is placed so that its
// lockbar lands at the target lockbar height above the room ground, scaled so that its lockbar
// matches the target width, and inclined by the effective playfield slope.
//
// roomOffset corrects imprecise headset room positioning (tracking origin, floor height, standing
// spot): it is applied as a single rigid world translation on top of all the spaces, so that the
// whole anchored assembly (room furniture and cabinet included) shifts together. It is purposely
// not injected inside the transform chains where it would be affected by the playfield slope and
// the cabinet scale.
VRSpaces ComputeVRSpaces(const VRSpaceSetup& setup, const VRTableGeometry& table)
{
   VRSpaces out;

   // Update fixed scaling, considering lockbar size to be the width of the playfield + 2"1/4
   // When not overriding the lockbar width, the cabinet is rendered at its native size
   const float tableWidth = VPUTOCM(table.playfieldWidth) + 2.25f * 2.54f;
   out.cabinetScale = setup.overrideLockbarWidth ? clamp(setup.customLockbarWidth / tableWidth, 0.1f, 2.0f) : 1.f;

   // The users define in their settings the real world height where they want the top of the lockbar to be.
   // The real world playfield height is then computed by removing the glass distance at the playfield bottom (typically 2 to 3").
   // The table offset allows to slightly adjust this height on a per table basis.
   // In the end Playfield height = tableOffset.z (user adjustment) + lockbar height (target) - glass distance * scale
   // When not overriding the lockbar height, the cabinet stands at its native (scaled) height
   const float lockbarHeight = setup.overrideLockbarHeight ? setup.customLockbarHeight : out.cabinetScale * VPUTOCM(table.groundToLockbarHeight);
   out.targetLockbarHeight = lockbarHeight;

   // Adjust coord from RH to LH system, and use the playfield bottom center as the pivot
   const Matrix3D coords = Matrix3D::MatrixScale(1.f, -1.f, 1.f) // RH to LH system
      * Matrix3D::MatrixTranslate(-table.playfieldWidth * 0.5f, table.playfieldDepth, 0.f); //

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
   out.playfield = coords
      * Matrix3D::MatrixTranslate(0.f, 0.f, -table.glassBottomHeight) // Apply glass gap perpendicular to the playfield
      * Matrix3D::MatrixScale(out.cabinetScale) // Adjust cabinet size to the target real world cabinet
      * Matrix3D::MatrixRotateX(ANGTORAD(setup.liveSlope + setup.seatedTilt)) // Tilt to playfield slope (and eventual seated play)
      * Matrix3D::MatrixTranslate(0.f, 0.f, CMTOVPU(lockbarHeight)) // finally apply target lockbar height in the room frame
      * adjustRoom;

   // Room does not apply the cabinet scaling nor any inclination, as it is the real world room, just the offsets & coords adjustments
   out.room = coords * adjustRoom;

   return out;
}
