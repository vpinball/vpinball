// license:GPLv3+

#include "core/stdafx.h"
#include "VRSpaces.h"

#include "core/def.h"

// Computes the space to world transforms of the 4 VR space references (room, cabinet feet,
// cabinet, playfield) from the headset room positioning and the target lockbar dimensions.
//
// The anchored assembly (cabinet + playfield) is placed so that its lockbar lands at the target
// lockbar height above the room ground, scaled so that its lockbar matches the target width.
// The feet are stretched to keep touching the room ground.
//
// roomOffset corrects imprecise headset room positioning (tracking origin, floor height, standing
// spot): it is applied as a single rigid world translation on top of all the spaces, so that the
// whole anchored assembly (room furniture and cabinet included) shifts together and the feet stay
// on the corrected ground plane. It is purposely not injected inside the transform chains where it
// would be affected by the playfield slope and the cabinet scale.
VRSpaces ComputeVRSpaces(const VRSpaceSetup& setup, const VRTableGeometry& table)
{
   VRSpaces out;

   // Update fixed scaling, considering lockbar size to be the width of the playfield + 2"1/4
   // When not overriding the lockbar width, the cabinet is rendered at its native size
   const float tableWidth = VPUTOCM(table.playfieldWidth) + 2.25f * 2.54f;
   out.cabinetScale = setup.overrideLockbarWidth ? clamp(setup.customLockbarWidth / tableWidth, 0.1f, 2.0f) : 1.f;
   const Matrix3D sceneScale = Matrix3D::MatrixScale(out.cabinetScale);
   const Matrix3D invSceneScale = Matrix3D::MatrixScale(1.f / out.cabinetScale);

   // Move table (in VPU coordinates), adjust coord from RH to LH system
   const Matrix3D coords = Matrix3D::MatrixScale(1.f, -1.f, 1.f);
   const Matrix3D rotz = Matrix3D::MatrixRotateZ(ANGTORAD(setup.roomYaw));
   const Matrix3D rotx2 = Matrix3D::MatrixRotateX(ANGTORAD(-90.f));
   const Matrix3D viewOrientation = rotz * rotx2;
   const Matrix3D viewOrientationInv = Matrix3D::MatrixInverse(viewOrientation);

   // The users define in their settings the real world height where they want the top of the lockbar to be.
   // The real world playfield height is then computed by removing the glass distance at the playfield bottom (typically 2 to 3").
   // The table offset allows to slightly adjust this height on a per table basis.
   // In the end Playfield height = tableOffset.z (user adjustment) + lockbar height (target) - glass distance * scale
   // When not overriding the lockbar height, the cabinet stands at its native (scaled) height
   const float scaledGlassHeight = out.cabinetScale * VPUTOCM(table.glassBottomHeight);
   const float lockbarHeight = setup.overrideLockbarHeight ? setup.customLockbarHeight : out.cabinetScale * VPUTOCM(table.groundToLockbarHeight);
   out.targetLockbarHeight = lockbarHeight;

   // The table defines the height of the lockbar of its cabinet model, as well as the playfield base inclination.
   // This allows to fit the cabinet & playfield models to the real world space.
   // If user adjust the inclination, then the cab is rotated as it would in real life (still missing the legs stretching a bit using caster adjustments)
   const float groundToPlayfieldHeight = out.cabinetScale * (table.groundToLockbarHeight - table.glassBottomHeight);
   const float baseSlope = lerp(table.angleTiltMin, table.angleTiltMax, table.difficulty);

   // Before 10.8.1, there weren't multiple space reference, so room used to be inclined to compensate the playfield inclination.
   // This may leads to slight visual artefact for old VR room (that is to say very slightly inclined room).
   // Seated play tilt rotates the whole cabinet (feet and playfield included) around the lockbar axis to face the player, the room staying upright
   const float viewSlope = setup.liveSlope + setup.seatedTilt;
   const Matrix3D playfieldSlope = Matrix3D::MatrixRotateX(ANGTORAD(viewSlope));
   const Matrix3D playfieldSlopeInv = Matrix3D::MatrixRotateX(-ANGTORAD(viewSlope));
   const Matrix3D tableCoords = Matrix3D::MatrixTranslate(-out.cabinetScale * table.playfieldWidth * 0.5f, +out.cabinetScale * table.playfieldDepth, 0.f);
   const Matrix3D playfieldPos = Matrix3D::MatrixTranslate(0.f, CMTOVPU(setup.tableOffset.y + kVRLockbarToPlayfield), CMTOVPU(setup.tableOffset.z + lockbarHeight - scaledGlassHeight));
   const Matrix3D playfieldPosInv = Matrix3D::MatrixInverse(playfieldPos);
   out.playfield = sceneScale * coords * tableCoords * playfieldSlope * playfieldPos * viewOrientation;

   const Matrix3D cabinetSlope = playfieldPosInv * Matrix3D::MatrixRotateX(ANGTORAD(viewSlope - baseSlope)) * playfieldPos;
   const Matrix3D pfToCab = viewOrientationInv // Revert view orientation
      * playfieldPosInv * playfieldSlopeInv // Revert playfield slope
      * Matrix3D::MatrixTranslate(0.f, CMTOVPU(setup.tableOffset.y + kVRLockbarToPlayfield),
         // Cabinet model has its z origin at the feet level, groundToLockbarHeight corresponding to the playfield level, so we move it down (to real world ground) then up to match user setup (where we placed the playfield)
         CMTOVPU(setup.tableOffset.z + lockbarHeight - scaledGlassHeight) - groundToPlayfieldHeight)
      * cabinetSlope // Apply cabinet slope
      * viewOrientation; // Reapply view orientation
   out.cabinet = out.playfield * pfToCab;

   // Feet are always touching the ground, scaled against the real world vs model defined playfield level
   // Note that since we are rotating the cabinet with its feet, the feet may slightly leave or enter the ground.
   // roomOffset.z is not part of the stretch ratio: it shifts the whole anchored assembly including the
   // corrected ground plane, so the leg proportions are unchanged (unlike tableOffset.z which raises the
   // playfield within the room and therefore stretches the legs).
   if (setup.lockFeetToGround)
   {
      constexpr float cabHeight = 0.4f * INCHESTOVPU(16.25f); // Magic value where the top of the leg should approximately stand (16.25" is the front height of a modern Stern cabinet)
      const float feetScale = (CMTOVPU(setup.tableOffset.z + lockbarHeight) / out.cabinetScale - cabHeight) / (table.groundToLockbarHeight - cabHeight);
      const Matrix3D pfToFeet = viewOrientationInv // Revert view orientation
         * playfieldPosInv * playfieldSlopeInv // Revert playfield slope
         * Matrix3D::MatrixTranslate(0.f, CMTOVPU(setup.tableOffset.y + kVRLockbarToPlayfield),
            CMTOVPU(setup.tableOffset.z)) // Feet are always at z=0 in real world, that is to say ground
         * Matrix3D::MatrixScale(1.f, 1.f, feetScale) // Scale feet in order to match feet bottom to real world floor
         * cabinetSlope // Apply cabinet slope
         * viewOrientation; // Reapply view orientation
      out.cabinetFeet = out.playfield * pfToFeet;
   }
   else
   {
      out.cabinetFeet = out.cabinet;
   }

   // Room does not apply the cabinet scaling nor any inclination, as it is the real world room
   const Matrix3D pfToRoom = viewOrientationInv // Revert view orientation
      * playfieldPosInv * playfieldSlopeInv // Revert playfield slope
      * Matrix3D::MatrixTranslate( // Apply table coordinate but without table scale
         -(1.f - out.cabinetScale) * table.playfieldWidth * 0.5f, +(1.f - out.cabinetScale) * table.playfieldDepth,
         0.f)
      * Matrix3D::MatrixTranslate(0.f, // For the ease of positioning, align the room to the table view setting, except for z which must stay on ground
         CMTOVPU(setup.tableOffset.y + kVRLockbarToPlayfield),
         CMTOVPU(setup.tableOffset.z))
      * viewOrientation; // Reapply view orientation
   out.room = invSceneScale * out.playfield * pfToRoom;

   // Apply the room positioning correction as a rigid world translation on top of all the spaces:
   // the whole anchored assembly (room and cabinet included) shifts together and the feet stay on
   // the corrected ground plane. The offset is expressed in the table oriented frame (x: lateral,
   // y: front/back, z: up) and mapped to world axes (y: up) by the view orientation.
   const Vertex3Ds worldOffset = viewOrientation * Vertex3Ds(-CMTOVPU(setup.roomOffset.x), CMTOVPU(setup.roomOffset.y), CMTOVPU(setup.roomOffset.z));
   const Matrix3D roomShift = Matrix3D::MatrixTranslate(worldOffset.x, worldOffset.y, worldOffset.z);
   out.playfield = out.playfield * roomShift;
   out.cabinet = out.cabinet * roomShift;
   out.cabinetFeet = out.cabinetFeet * roomShift;
   out.room = out.room * roomShift;

   return out;
}
