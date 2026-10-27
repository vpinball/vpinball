// license:GPLv3+

#include "core/stdafx.h"

#include "renderer/VRSeatedView.h"
#include "doctest.h"

namespace
{
void CheckPosition(const Vertex3Ds& actual, const Vertex3Ds& expected)
{
   CHECK(actual.x == doctest::Approx(expected.x).epsilon(0.0001).scale(1.0));
   CHECK(actual.y == doctest::Approx(expected.y).epsilon(0.0001).scale(1.0));
   CHECK(actual.z == doctest::Approx(expected.z).epsilon(0.0001).scale(1.0));
}
}

TEST_CASE("VR seated placement [vr-seated]")
{
   const Matrix3D orientation = Matrix3D::MatrixRotateX(ANGTORAD(-90.f));
   const Vertex3Ds head(CMTOVPU(25.f), CMTOVPU(110.f), CMTOVPU(-45.f));
   const Matrix3D frontToWorld = Matrix3D::MatrixTranslate(CMTOVPU(10.f), CMTOVPU(20.f), CMTOVPU(85.f)) * orientation;
   const Vertex3Ds standingFront = frontToWorld * Vertex3Ds(0.f, 0.f, 0.f);

   SUBCASE("height and distance are relative to recentered eyes")
   {
      const Matrix3D seated = VPX::VRSeatedView::GetTransform(frontToWorld, orientation, head, -30.f, 40.f, 0.f);
      CheckPosition(seated * standingFront, Vertex3Ds(head.x, head.y - CMTOVPU(30.f), head.z - CMTOVPU(40.f)));
   }

   SUBCASE("ninety degree recline places the table above the player")
   {
      const Matrix3D seated = VPX::VRSeatedView::GetTransform(frontToWorld, orientation, head, -30.f, 40.f, 90.f);
      CheckPosition(seated * standingFront, Vertex3Ds(head.x, head.y + CMTOVPU(40.f), head.z - CMTOVPU(30.f)));
      // Transform is rigid: table dimensions are preserved
      const Vertex3Ds farEnd = frontToWorld * Vertex3Ds(0.f, CMTOVPU(100.f), 0.f);
      CHECK(((seated * farEnd) - (seated * standingFront)).Length() == doctest::Approx(CMTOVPU(100.f)));
   }

   SUBCASE("placement does not depend on standing height or offsets")
   {
      const Matrix3D otherFront = Matrix3D::MatrixTranslate(-500.f, 1200.f, 40.f) * orientation;
      const Matrix3D seated = VPX::VRSeatedView::GetTransform(frontToWorld, orientation, head, -20.f, 50.f, 65.f);
      const Matrix3D otherSeated = VPX::VRSeatedView::GetTransform(otherFront, orientation, head, -20.f, 50.f, 65.f);
      CheckPosition(seated * standingFront, otherSeated * (otherFront * Vertex3Ds(0.f, 0.f, 0.f)));
   }

   SUBCASE("yaw rotates seated distance around the player")
   {
      const Matrix3D turned = Matrix3D::MatrixRotateZ(ANGTORAD(90.f)) * orientation;
      const Matrix3D turnedFront = Matrix3D::MatrixTranslate(100.f, 200.f, 300.f) * turned;
      const Matrix3D seated = VPX::VRSeatedView::GetTransform(turnedFront, turned, head, -30.f, 40.f, 0.f);
      CheckPosition(seated * (turnedFront * Vertex3Ds(0.f, 0.f, 0.f)), Vertex3Ds(head.x - CMTOVPU(40.f), head.y - CMTOVPU(30.f), head.z));
   }

   SUBCASE("recenter heading is stable while looking at the ceiling")
   {
      const Matrix3D headPose = Matrix3D::MatrixRotateX(ANGTORAD(90.f)) * Matrix3D::MatrixRotateY(ANGTORAD(45.f));
      CHECK(VPX::VRSeatedView::GetHeadYaw(headPose, 0.f) == doctest::Approx(ANGTORAD(45.f)));
   }

   SUBCASE("heading preserves the last value if the right axis is vertical")
   {
      CHECK(VPX::VRSeatedView::GetHeadYaw(Matrix3D::MatrixRotateZ(ANGTORAD(90.f)), 0.6f) == doctest::Approx(0.6f));
   }
}
