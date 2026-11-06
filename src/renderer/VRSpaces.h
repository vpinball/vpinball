// license:GPLv3+

#pragma once

#include "math/vector.h"
#include "math/matrix.h"

// Fixed distance between the playfield bottom and the end of the lockbar border, in cm.
// This is part of the (universal) cabinet model and could be made a table data if needed.
inline constexpr float kVRLockbarToPlayfield = 5.f;

// Cabinet geometry authored in the table file, all distances in VPU, angles in degrees
struct VRTableGeometry
{
   float playfieldWidth = 0.f;
   float playfieldDepth = 0.f;
   float groundToLockbarHeight = 0.f; // Height of the cabinet model's lockbar from its feet
   float glassBottomHeight = 0.f; // Height of the playfield bottom (below the glass) from the feet
   float angleTiltMin = 0.f;
   float angleTiltMax = 0.f;
   float difficulty = 0.f; // 0..1, selects the authored base slope between angleTiltMin and angleTiltMax
};

// User-adjustable VR setup. Distances in cm, angles in degrees.
struct VRSpaceSetup
{
   // Room positioning: corrects imprecise headset room positioning (tracking origin, floor height,
   // or desired standing spot). Global, persisted between sessions.
   Vertex3Ds roomOffset { 0.f, 0.f, 0.f }; // Shifts the whole anchored assembly (feet stay on the corrected ground)
   float roomYaw = 0.f;

   // Per-table offsets, compensating imprecise cabinet geometry definitions
   Vertex3Ds tableOffset { 0.f, 0.f, 0.f }; // y: playfield end to lockbar distance, z: glass height

   // Lockbar target dimensions: custom (measured on the user's real world cabinet) or table authored
   bool overrideLockbarWidth = true;
   bool overrideLockbarHeight = true;
   float customLockbarWidth = 57.f;
   float customLockbarHeight = 85.f;

   bool lockFeetToGround = true;
   float seatedTilt = 0.f; // Cabinet tilt toward the player for seated play
   float liveSlope = 0.f; // Effective playfield slope (authored slope possibly adjusted by the user)
};

// Space to world transforms for the 4 VR space references (see PartGroupData::SpaceReference)
struct VRSpaces
{
   Matrix3D playfield; // Simulation space, follows the cabinet with playfield slope applied
   Matrix3D cabinet; // Cabinet, scaled/positioned so its lockbar matches the target width & height
   Matrix3D cabinetFeet; // Cabinet feet, scaled with the cabinet and touching the (corrected) ground
   Matrix3D room; // Real world room, unscaled
   float cabinetScale = 1.f;
   float targetLockbarHeight = 0.f; // Effective lockbar height above the (corrected) ground, in cm
};

VRSpaces ComputeVRSpaces(const VRSpaceSetup& setup, const VRTableGeometry& table);
