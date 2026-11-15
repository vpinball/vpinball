// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "renderer/VRSpaces.h"

#include "doctest.h"

namespace
{

// Generic WPC-like cabinet geometry, in VPU
VRTableGeometry TestTable()
{
   VRTableGeometry table;
   table.playfieldWidth = 950.f;
   table.playfieldDepth = 1900.f;
   table.groundToLockbarHeight = 1690.f;
   table.glassBottomHeight = 1560.f;
   return table;
}

// Asserts that 'm' is a pure translation matrix (linear part is identity)
void CheckPureTranslation(const Matrix3D& m)
{
   for (int r = 0; r < 3; r++)
      for (int c = 0; c < 4; c++)
         CHECK(m.m[r][c] == doctest::Approx(r == c ? 1.f : 0.f).epsilon(1e-5));
   CHECK(m.m[3][3] == doctest::Approx(1.f).epsilon(1e-5));
}

void CheckSameMatrix(const Matrix3D& a, const Matrix3D& b, float epsilon)
{
   for (int r = 0; r < 4; r++)
      for (int c = 0; c < 4; c++)
         CHECK(a.m[r][c] == doctest::Approx(b.m[r][c]).epsilon(epsilon));
}

}

TEST_CASE("VR space transforms")
{
   const VRTableGeometry table = TestTable();

   SUBCASE("lockbar dimensions: authored vs custom override")
   {
      VRSpaceSetup setup;
      setup.overrideLockbarWidth = false;
      setup.overrideLockbarHeight = false;
      const VRSpaces authored = ComputeVRSpaces(setup, table);

      // Table authored dimensions are used at native scale
      CHECK(authored.cabinetScale == doctest::Approx(1.f));
      CHECK(authored.targetLockbarHeight == doctest::Approx(VPUTOCM(table.groundToLockbarHeight)));

      // Playfield bottom sits at lockbar height minus glass distance above ground (world y is up)
      const Vertex3Ds pfOrigin = authored.playfield * Vertex3Ds(0.f, 0.f, 0.f);
      CHECK(pfOrigin.y == doctest::Approx(table.groundToLockbarHeight - table.glassBottomHeight).epsilon(1e-4));

      setup.overrideLockbarWidth = true;
      setup.overrideLockbarHeight = true;
      setup.customLockbarWidth = 57.f;
      setup.customLockbarHeight = 85.f;
      const VRSpaces custom = ComputeVRSpaces(setup, table);

      const float tableWidth = VPUTOCM(table.playfieldWidth) + 2.25f * 2.54f;
      CHECK(custom.cabinetScale == doctest::Approx(clamp(57.f / tableWidth, 0.1f, 2.0f)));
      CHECK(custom.targetLockbarHeight == doctest::Approx(85.f));
   }

   SUBCASE("lockbar width clamping")
   {
      VRSpaceSetup setup;
      setup.overrideLockbarWidth = true;
      setup.customLockbarWidth = 10000.f;
      CHECK(ComputeVRSpaces(setup, table).cabinetScale == doctest::Approx(2.0f));
      setup.customLockbarWidth = 1.f;
      CHECK(ComputeVRSpaces(setup, table).cabinetScale == doctest::Approx(0.1f));
   }

   SUBCASE("room offset shifts the whole anchored assembly rigidly")
   {
      const VRSpaces a = ComputeVRSpaces(VRSpaceSetup {}, table);

      VRSpaceSetup setup;
      setup.roomOffset = Vertex3Ds(3.f, -7.f, 12.f);
      const VRSpaces b = ComputeVRSpaces(setup, table);

      // Same world translation applied on top of every space: inv(a) * b is a pure translation
      // table oriented frame offset (x: lateral, y: front/back, z: up) -> world (x, z, -y)
      const Matrix3D spaces[3][2] = { { a.playfield, b.playfield }, { a.cabinetFeet, b.cabinetFeet }, { a.room, b.room } };
      for (const auto& s : spaces)
      {
         const Matrix3D delta = Matrix3D::MatrixInverse(s[0]) * s[1];
         CheckPureTranslation(delta);
         CHECK(delta.m[3][0] == doctest::Approx(-CMTOVPU(3.f)).epsilon(1e-4));
         CHECK(delta.m[3][1] == doctest::Approx(CMTOVPU(12.f)).epsilon(1e-4));
         CHECK(delta.m[3][2] == doctest::Approx(-CMTOVPU(-7.f)).epsilon(1e-4));
      }
   }

   SUBCASE("room z offset lands on the corrected ground plane")
   {
      // Room-space origin (z = 0 plane) maps to the corrected ground height
      VRSpaceSetup setup;
      setup.tableOffset.z = 8.f;
      setup.roomOffset.z = 15.f;
      const VRSpaces spaces = ComputeVRSpaces(setup, table);
      const Vertex3Ds roomOrigin = spaces.room * Vertex3Ds(0.f, 0.f, 0.f);
      CHECK(roomOrigin.y == doctest::Approx(CMTOVPU(setup.tableOffset.z + setup.roomOffset.z)).epsilon(1e-4));
   }

   SUBCASE("feet follow the playfield when not locked to ground")
   {
      // Feet parts are authored with their z origin at the feet level: when following the
      // anchored assembly, the origin is shifted to the playfield surface
      VRSpaceSetup setup;
      setup.lockFeetToGround = false;
      const VRSpaces spaces = ComputeVRSpaces(setup, table);
      const Matrix3D expected = Matrix3D::MatrixTranslate(0.f, 0.f, table.glassBottomHeight - table.groundToLockbarHeight) * spaces.playfield;
      CheckSameMatrix(spaces.cabinetFeet, expected, 1e-5f);
   }

   SUBCASE("locked feet stand upright on the ground below the playfield origin")
   {
      // Feet-space parts are authored like the room space (z origin at the feet level), usually a
      // primitive placed at the space origin spanning the cabinet from its back left floor corner
      VRSpaceSetup setup;
      setup.liveSlope = 6.f;
      setup.overrideLockbarWidth = true;
      setup.customLockbarWidth = 65.f;
      setup.overrideLockbarHeight = true;
      setup.customLockbarHeight = 85.f;
      const VRSpaces spaces = ComputeVRSpaces(setup, table);

      // The feet space origin sits on the ground, right below the playfield origin
      const Vertex3Ds feetOrigin = spaces.cabinetFeet * Vertex3Ds(0.f, 0.f, 0.f);
      const Vertex3Ds pfOrigin = spaces.playfield * Vertex3Ds(0.f, 0.f, 0.f);
      CHECK(feetOrigin.y == doctest::Approx(0.f).epsilon(1e-4));
      CHECK(feetOrigin.x == doctest::Approx(pfOrigin.x).epsilon(1e-5));
      CHECK(feetOrigin.z == doctest::Approx(pfOrigin.z).epsilon(1e-5));

      // The feet stay upright: the authored z axis maps to the world vertical
      const Vertex3Ds up = spaces.cabinetFeet.MultiplyVectorNoTranslate(Vertex3Ds(0.f, 0.f, 1.f));
      CHECK(up.x == doctest::Approx(0.f).epsilon(1e-5));
      CHECK(up.z == doctest::Approx(0.f).epsilon(1e-5));

      // The whole floor plane stays flat on the room ground (no tilt from the playfield slope)
      // and the cabinet scaling is applied on X/Y (same lateral extent as the playfield)
      const Vertex3Ds probe[] = { Vertex3Ds(0.f, 0.f, 0.f), Vertex3Ds(table.playfieldWidth, table.playfieldDepth, 0.f),
         Vertex3Ds(table.playfieldWidth * 0.5f, table.playfieldDepth * 0.3f, 300.f), Vertex3Ds(table.playfieldWidth, 0.f, 800.f) };
      for (const Vertex3Ds& p : probe)
      {
         CHECK((spaces.cabinetFeet * Vertex3Ds(p.x, p.y, 0.f)).y == doctest::Approx(0.f).epsilon(1e-4));
         CHECK((spaces.cabinetFeet * p).x == doctest::Approx((spaces.playfield * p).x).epsilon(1e-5));
      }

      // The top of the legs lands at the cabinet's leg attach point (on the sloped assembly)
      constexpr float cabHeight = 0.4f * INCHESTOVPU(16.25f);
      const float legAttach = spaces.cabinetScale * (table.groundToLockbarHeight - table.glassBottomHeight - cabHeight);
      const float attachZ = CMTOVPU(spaces.targetLockbarHeight - spaces.cabinetScale * VPUTOCM(table.glassBottomHeight)) - legAttach * cosf(ANGTORAD(setup.liveSlope));
      CHECK((spaces.cabinetFeet * Vertex3Ds(0.f, table.playfieldDepth, cabHeight)).y == doctest::Approx(attachZ).epsilon(1e-4));
   }

   SUBCASE("room yaw rotates the room space around the vertical axis")
   {
      VRSpaceSetup setup;
      setup.roomYaw = 30.f;
      const VRSpaces rotated = ComputeVRSpaces(setup, table);

      // Room-space +x axis maps to the yawed direction in the world horizontal (x/z) plane
      const Vertex3Ds xAxis = rotated.room.MultiplyVectorNoTranslate(Vertex3Ds(1.f, 0.f, 0.f));
      CHECK(xAxis.x == doctest::Approx(cosf(ANGTORAD(30.f))).epsilon(1e-5));
      CHECK(xAxis.y == doctest::Approx(0.f).epsilon(1e-5));
      CHECK(xAxis.z == doctest::Approx(-sinf(ANGTORAD(30.f))).epsilon(1e-5));
   }

   SUBCASE("seated play tilt does not move the room")
   {
      const VRSpaces upright = ComputeVRSpaces(VRSpaceSetup {}, table);
      VRSpaceSetup setup;
      setup.seatedTilt = 20.f;
      const VRSpaces seated = ComputeVRSpaces(setup, table);
      CheckSameMatrix(upright.room, seated.room, 1e-2f); // slope cancels analytically, small residual comes from matrix inversions
   }
}
