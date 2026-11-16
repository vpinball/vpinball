// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "core/SceneLayout.h"

#include "doctest.h"

namespace
{

// Generic WPC-like cabinet geometry, in VPU
SceneLayout::TableGeometry TestTable()
{
   SceneLayout::TableGeometry table;
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

// Asserts that the linear part of 'm' is orthonormal (shading requires it from a view matrix)
void CheckOrthonormal(const Matrix3D& m)
{
   for (int r = 0; r < 3; r++)
   {
      const Vertex3Ds row = m.MultiplyVectorNoTranslate(Vertex3Ds(r == 0 ? 1.f : 0.f, r == 1 ? 1.f : 0.f, r == 2 ? 1.f : 0.f));
      CHECK(row.Length() == doctest::Approx(1.f).epsilon(1e-5));
   }
}

}

TEST_CASE("Scene layout space transforms")
{
   const SceneLayout::TableGeometry table = TestTable();
   SceneLayout layout; // No table: the layout is driven through Update calls (as tests and VR do)

   SUBCASE("lockbar dimensions: authored vs custom override")
   {
      SceneLayout::Setup setup;
      layout.Update(table, setup);

      // Table authored dimensions are used at native scale
      CHECK(layout.GetCabinetScale() == doctest::Approx(1.f));
      CHECK(layout.GetTargetLockbarHeight() == doctest::Approx(VPUTOCM(table.groundToLockbarHeight)));

      // Playfield bottom sits at lockbar height minus glass distance above ground (world y is up)
      const Vertex3Ds pfOrigin = layout.GetSpaceTransform(VPX::SpaceReference::Playfield).m_transform * Vertex3Ds(0.f, 0.f, 0.f);
      CHECK(pfOrigin.y == doctest::Approx(table.groundToLockbarHeight - table.glassBottomHeight).epsilon(1e-4));

      setup.overrideLockbarWidth = true;
      setup.overrideLockbarHeight = true;
      setup.customLockbarWidth = 57.f;
      setup.customLockbarHeight = 85.f;
      layout.Update(table, setup);

      const float tableWidth = VPUTOCM(table.playfieldWidth) + 2.25f * 2.54f;
      CHECK(layout.GetCabinetScale() == doctest::Approx(clamp(57.f / tableWidth, 0.1f, 2.0f)));
      CHECK(layout.GetTargetLockbarHeight() == doctest::Approx(85.f));
   }

   SUBCASE("lockbar width clamping")
   {
      SceneLayout::Setup setup;
      setup.overrideLockbarWidth = true;
      setup.customLockbarWidth = 10000.f;
      layout.Update(table, setup);
      CHECK(layout.GetCabinetScale() == doctest::Approx(2.0f));
      setup.customLockbarWidth = 1.f;
      layout.Update(table, setup);
      CHECK(layout.GetCabinetScale() == doctest::Approx(0.1f));
   }

   SUBCASE("room offset shifts the whole anchored assembly rigidly")
   {
      layout.Update(table, SceneLayout::Setup {});
      const Matrix3D aPlayfield = layout.GetSpaceTransform(VPX::SpaceReference::Playfield).m_transform;
      const Matrix3D aRoom = layout.GetSpaceTransform(VPX::SpaceReference::Room).m_transform;

      SceneLayout::Setup setup;
      setup.roomOffset = Vertex3Ds(3.f, -7.f, 12.f);
      layout.Update(table, setup);

      // Same world translation applied on top of every space: inv(a) * b is a pure translation
      // table oriented frame offset (x: lateral, y: front/back, z: up) -> world (x, z, -y)
      const Matrix3D spaces[2][2]
         = { { aPlayfield, layout.GetSpaceTransform(VPX::SpaceReference::Playfield).m_transform }, { aRoom, layout.GetSpaceTransform(VPX::SpaceReference::Room).m_transform } };
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
      SceneLayout::Setup setup;
      setup.tableOffset.z = 8.f;
      setup.roomOffset.z = 15.f;
      layout.Update(table, setup);
      const Vertex3Ds roomOrigin = layout.GetSpaceTransform(VPX::SpaceReference::Room).m_transform * Vertex3Ds(0.f, 0.f, 0.f);
      CHECK(roomOrigin.y == doctest::Approx(CMTOVPU(setup.tableOffset.z + setup.roomOffset.z)).epsilon(1e-4));
   }

   SUBCASE("room yaw rotates the room space around the vertical axis")
   {
      SceneLayout::Setup setup;
      setup.roomYaw = 30.f;
      layout.Update(table, setup);

      // Room-space +x axis maps to the yawed direction in the world horizontal (x/z) plane
      const Vertex3Ds xAxis = layout.GetSpaceTransform(VPX::SpaceReference::Room).m_transform.MultiplyVectorNoTranslate(Vertex3Ds(1.f, 0.f, 0.f));
      CHECK(xAxis.x == doctest::Approx(cosf(ANGTORAD(30.f))).epsilon(1e-5));
      CHECK(xAxis.y == doctest::Approx(0.f).epsilon(1e-5));
      CHECK(xAxis.z == doctest::Approx(-sinf(ANGTORAD(30.f))).epsilon(1e-5));
   }

   SUBCASE("seated play tilt does not move the room")
   {
      layout.Update(table, SceneLayout::Setup {});
      const Matrix3D upright = layout.GetSpaceTransform(VPX::SpaceReference::Room).m_transform;
      SceneLayout::Setup setup;
      setup.seatedTilt = 20.f;
      layout.Update(table, setup);
      CheckSameMatrix(upright, layout.GetSpaceTransform(VPX::SpaceReference::Room).m_transform, 1e-2f); // slope cancels analytically, small residual comes from matrix inversions
   }

   SUBCASE("space to playfield transforms derive from the space transforms")
   {
      SceneLayout::Setup setup;
      setup.overrideLockbarWidth = true;
      setup.customLockbarWidth = 57.f;
      setup.liveSlope = 6.f;
      layout.Update(table, setup);

      // The playfield space maps to itself
      CheckSameMatrix(layout.GetSpaceToPlayfieldTransform(VPX::SpaceReference::Playfield).m_transform, Matrix3D::MatrixIdentity(), 1e-5f);

      // The other spaces map through the scene root: space -> scene -> playfield
      const Matrix3D roomToPf
         = layout.GetSpaceTransform(VPX::SpaceReference::Room).m_transform * Matrix3D::MatrixInverse(layout.GetSpaceTransform(VPX::SpaceReference::Playfield).m_transform);
      CheckSameMatrix(layout.GetSpaceToPlayfieldTransform(VPX::SpaceReference::Room).m_transform, roomToPf, 1e-3f);
   }

   SUBCASE("composed view stays orthonormal, scale compensated into the projection")
   {
      SceneLayout::Setup setup;
      setup.overrideLockbarWidth = true;
      setup.customLockbarWidth = 57.f;
      layout.Update(table, setup);
      const SceneLayout::SpaceTransform& space = layout.GetSpaceTransform(VPX::SpaceReference::Playfield);
      CHECK(space.m_scale == doctest::Approx(layout.GetCabinetScale()));
      REQUIRE(space.m_scale != doctest::Approx(1.f)); // The playfield transform really is scaled

      const Matrix3D camView = Matrix3D::MatrixRotateX(ANGTORAD(10.f)) * Matrix3D::MatrixTranslate(-30.f, -100.f, 40.f);
      float projScale = 0.f;
      const Matrix3D view = SceneLayout::ComposeView(space, camView, projScale);
      CHECK(projScale == doctest::Approx(space.m_scale));
      CheckOrthonormal(view);

      // view = transform * camView * inv(scale): the net transform is preserved, the scale being
      // folded back through the projection (proj = scale * camProj)
      CheckSameMatrix(view, space.m_transform * camView * Matrix3D::MatrixScale(1.f / space.m_scale), 1e-3f);
   }
}
