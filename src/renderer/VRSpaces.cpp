// license:GPLv3+

#include "core/stdafx.h"
#include "VRSpaces.h"

#include "core/def.h"

// Computes the space to world transforms of the 3 VR space references (room, cabinet feet,
// playfield) from the headset room positioning and the target lockbar dimensions.
//
// The anchored assembly (cabinet + playfield, a single rigid body) is placed so that its lockbar
// lands at the target lockbar height above the room ground, scaled so that its lockbar matches the
// target width, and inclined by the effective playfield slope. The feet stand upright on the room
// ground with their space origin below the playfield origin, the cabinet scaling applied on X/Y,
// and the legs stretched in Z so their top reaches the cabinet's leg attach point.
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

   // Move table (in VPU coordinates), adjust coord from RH to LH system
   const Matrix3D coords = Matrix3D::MatrixScale(1.f, -1.f, 1.f);
   const Matrix3D rotz = Matrix3D::MatrixRotateZ(ANGTORAD(setup.roomYaw));
   const Matrix3D rotx2 = Matrix3D::MatrixRotateX(ANGTORAD(-90.f));
   const Matrix3D viewOrientation = rotz * rotx2;

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

   // Seated play tilt rotates the whole cabinet (feet and playfield included) around the lockbar axis to face the player, the room staying upright
   const float viewSlope = setup.liveSlope + setup.seatedTilt;
   const Matrix3D playfieldSlope = Matrix3D::MatrixRotateX(ANGTORAD(viewSlope));
   const Matrix3D tableCoords = Matrix3D::MatrixTranslate(-out.cabinetScale * table.playfieldWidth * 0.5f, +out.cabinetScale * table.playfieldDepth, 0.f);
   const Matrix3D playfieldPos = Matrix3D::MatrixTranslate(0.f, CMTOVPU(setup.tableOffset.y + kVRLockbarToPlayfield), CMTOVPU(setup.tableOffset.z + lockbarHeight - scaledGlassHeight));
   out.playfield = sceneScale * coords * tableCoords * playfieldSlope * playfieldPos * viewOrientation;

   // Room does not apply the cabinet scaling nor any inclination, as it is the real world room
   // For the ease of positioning, align the room to the table view setting, except for z which must stay on ground
   out.room = coords * Matrix3D::MatrixTranslate(-table.playfieldWidth * 0.5f, table.playfieldDepth + CMTOVPU(setup.tableOffset.y + kVRLockbarToPlayfield), CMTOVPU(setup.tableOffset.z))
      * viewOrientation;

   // Feet are authored like the room space (z origin at the feet level), usually as a primitive
   // placed at the space origin spanning the cabinet from its back left floor corner. When locked
   // to ground, their space origin sits on the ground right below the playfield origin (aligned on
   // its z axis, under the cabinet, shifted on Y as the assembly slopes): the frame stays aligned
   // on the vertical axis with the cabinet scaling applied on X/Y, and the legs are stretched in Z
   // so their top reaches the leg attach point on the inclined cabinet.
   // roomOffset.z is not part of the stretch ratio: it shifts the whole anchored assembly including the
   // corrected ground plane, so the leg proportions are unchanged (unlike tableOffset.z which raises the
   // playfield within the room and therefore stretches the legs).
   if (setup.lockFeetToGround)
   {
      constexpr float cabHeight = 0.4f * INCHESTOVPU(16.25f); // Magic value where the top of the leg should approximately stand (16.25" is the front height of a modern Stern cabinet)
      const float legAttach = groundToPlayfieldHeight - out.cabinetScale * cabHeight; // Leg attach depth below the playfield surface (scaled frame)
      const float slopeCos = cosf(ANGTORAD(viewSlope));
      const float feetScale = (CMTOVPU(lockbarHeight - scaledGlassHeight) - legAttach * slopeCos) / cabHeight;
      out.cabinetFeet = sceneScale * coords //
         * Matrix3D::MatrixScale(1.f, 1.f, feetScale / out.cabinetScale)
         * Matrix3D::MatrixTranslate(-out.cabinetScale * table.playfieldWidth * 0.5f,
            out.cabinetScale * table.playfieldDepth * slopeCos + CMTOVPU(setup.tableOffset.y + kVRLockbarToPlayfield), CMTOVPU(setup.tableOffset.z))
         * viewOrientation;
   }
   else
   {
      // Shift the feet z origin (at the feet level) to the playfield one so the feet follow the anchored assembly
      out.cabinetFeet = Matrix3D::MatrixTranslate(0.f, 0.f, table.glassBottomHeight - table.groundToLockbarHeight) * out.playfield;
   }

   // Apply the room positioning correction as a rigid world translation on top of all the spaces:
   // the whole anchored assembly (room and cabinet included) shifts together and the feet stay on
   // the corrected ground plane. The offset is expressed in the table oriented frame (x: lateral,
   // y: front/back, z: up) and mapped to world axes (y: up) by the view orientation.
   const Vertex3Ds worldOffset = viewOrientation * Vertex3Ds(-CMTOVPU(setup.roomOffset.x), CMTOVPU(setup.roomOffset.y), CMTOVPU(setup.roomOffset.z));
   const Matrix3D roomShift = Matrix3D::MatrixTranslate(worldOffset.x, worldOffset.y, worldOffset.z);
   out.playfield = out.playfield * roomShift;
   out.cabinetFeet = out.cabinetFeet * roomShift;
   out.room = out.room * roomShift;

   return out;
}
