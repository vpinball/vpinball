// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "math/dragpoint.h"
#include "parts/timer.h"
#include "utils/BiffReader.h"
#include "utils/BiffWriter.h"
#include "core/vpversion.h"

#include "doctest.h"

namespace
{

// DragPoint curves are owned by an IEditable; a detached Timer is used as the owner here
DragPoint* AddPoint(DragPointCurve& curve, float x, float y, float z = 0.f, bool smooth = false)
{
   auto point = std::make_unique<DragPoint>(&curve, x, y, z, smooth);
   DragPoint* const raw = point.get();
   curve.PushPoint(std::move(point));
   return raw;
}

} // namespace

TEST_CASE("DragPoint curve")
{
   Timer* const owner = Timer::COMCreate();
   owner->Init(0.f, 0.f, false);

   SUBCASE("point ownership, bounds and center")
   {
      DragPointCurve curve(owner, 3);
      AddPoint(curve, 0.f, 0.f);
      AddPoint(curve, 10.f, 0.f);
      AddPoint(curve, 10.f, 10.f);
      AddPoint(curve, 0.f, 10.f);

      CHECK(curve.GetPoints().size() == 4);
      CHECK(curve.GetMinimumPoints() == 3);
      CHECK(curve.GetIEditable() == owner);
      CHECK(curve.GetPoints()[0]->GetIEditable() == owner);
      CHECK(curve.GetPoints()[0]->GetCurve() == &curve);

      // Bounds are computed from the tessellated curve (all corners are non-smooth
      // here so they coincide with the control points)
      CHECK(curve.GetMinBound().x == doctest::Approx(0.f));
      CHECK(curve.GetMinBound().y == doctest::Approx(0.f));
      CHECK(curve.GetMaxBound().x == doctest::Approx(10.f));
      CHECK(curve.GetMaxBound().y == doctest::Approx(10.f));
      CHECK(curve.GetCenter().x == doctest::Approx(5.f));
      CHECK(curve.GetCenter().y == doctest::Approx(5.f));
   }

   SUBCASE("curve transforms")
   {
      DragPointCurve curve(owner, 3);
      AddPoint(curve, 0.f, 0.f);
      AddPoint(curve, 10.f, 0.f);
      AddPoint(curve, 10.f, 10.f);
      AddPoint(curve, 0.f, 10.f);
      // Snapshot point identities: FlipPointX/Y reorder the owning vector
      const vector<DragPoint*> points { curve.GetPoints()[0].get(), curve.GetPoints()[1].get(), curve.GetPoints()[2].get(), curve.GetPoints()[3].get() };

      curve.TranslatePoints(Vertex2D(5.f, -5.f));
      CHECK(points[0]->GetX() == 5.f);
      CHECK(points[0]->GetY() == -5.f);
      CHECK(points[2]->GetX() == 15.f);
      CHECK(points[2]->GetY() == 5.f);
      CHECK(curve.GetCenter().x == doctest::Approx(10.f));
      CHECK(curve.GetCenter().y == doctest::Approx(0.f));

      curve.ScalePoints(2.f, 1.f, Vertex2D(0.f, 0.f));
      CHECK(points[0]->GetX() == 10.f);
      CHECK(points[0]->GetY() == -5.f);
      CHECK(points[2]->GetX() == 30.f);
      CHECK(points[2]->GetY() == 5.f);

      curve.RotatePoints(90.f, Vertex2D(0.f, 0.f));
      CHECK(points[0]->GetX() == doctest::Approx(5.f));
      CHECK(points[0]->GetY() == doctest::Approx(10.f));
      CHECK(points[2]->GetX() == doctest::Approx(-5.f));
      CHECK(points[2]->GetY() == doctest::Approx(30.f));

      curve.FlipPointX(Vertex2D(0.f, 0.f));
      CHECK(points[0]->GetX() == doctest::Approx(-5.f));
      CHECK(points[2]->GetX() == doctest::Approx(5.f));

      curve.FlipPointY(Vertex2D(0.f, 0.f));
      CHECK(points[0]->GetY() == doctest::Approx(-10.f));
      CHECK(points[2]->GetY() == doctest::Approx(-30.f));
   }

   SUBCASE("single point translate")
   {
      DragPointCurve curve(owner, 3);
      const auto point = AddPoint(curve, 1.f, 2.f, 3.f);
      CHECK(point->GetCenter().x == 1.f);
      CHECK(point->GetCenter().y == 2.f);
      point->Translate(Vertex2D(4.f, 5.f));
      CHECK(point->GetX() == 5.f);
      CHECK(point->GetY() == 7.f);
      CHECK(point->GetZ() == 3.f); // z is not affected by the 2D translation
   }

   SUBCASE("point deletion is limited by the curve minimum")
   {
      DragPointCurve curve(owner, 3);
      const auto p1 = AddPoint(curve, 0.f, 0.f);
      const auto p2 = AddPoint(curve, 1.f, 0.f);
      const auto p3 = AddPoint(curve, 1.f, 1.f);
      const auto p4 = AddPoint(curve, 0.f, 1.f);

      CHECK(p1->CanDelete());
      p4->Delete();
      CHECK(curve.GetPoints().size() == 3);
      CHECK_FALSE(p1->CanDelete());
      p1->Delete(); // Refused: the curve is already at its minimum
      CHECK(curve.GetPoints().size() == 3);
      (void)p2;
      (void)p3;
   }

   SUBCASE("smooth and slingshot flags are mutually exclusive")
   {
      DragPointCurve curve(owner, 3);
      const auto p1 = AddPoint(curve, 0.f, 0.f);
      const auto p2 = AddPoint(curve, 1.f, 0.f, 0.f, true);
      AddPoint(curve, 1.f, 1.f);

      // Enabling slingshot clears smooth on the point and on the next point
      p1->SetSmooth(true);
      p1->ToggleSlingshot();
      CHECK(p1->m_slingshot);
      CHECK_FALSE(p1->IsSmooth());
      CHECK_FALSE(p2->IsSmooth());

      // Enabling smooth clears slingshot on the point and on the previous point
      p1->ToggleSmooth();
      CHECK(p1->IsSmooth());
      CHECK_FALSE(p1->m_slingshot);
   }

   SUBCASE("copy/paste moves point coordinates")
   {
      DragPointCurve curve(owner, 3);
      const auto p1 = AddPoint(curve, 1.f, 2.f, 3.f);
      const auto p2 = AddPoint(curve, 10.f, 20.f, 30.f);
      AddPoint(curve, 5.f, 5.f);

      p1->Copy();
      p2->Paste();
      CHECK(p2->GetX() == 1.f);
      CHECK(p2->GetY() == 2.f);
      CHECK(p2->GetZ() == 3.f);
   }

   SUBCASE("reverse order keeps the point set and flips winding")
   {
      DragPointCurve curve(owner, 3);
      AddPoint(curve, 0.f, 0.f);
      AddPoint(curve, 1.f, 0.f);
      AddPoint(curve, 2.f, 0.f);
      AddPoint(curve, 3.f, 0.f);

      curve.ReverseOrder();
      const auto& points = curve.GetPoints();
      CHECK(points[0]->GetX() == 3.f);
      CHECK(points[1]->GetX() == 2.f);
      CHECK(points[2]->GetX() == 1.f);
      CHECK(points[3]->GetX() == 0.f);
   }

   SUBCASE("point data round-trips through BIFF")
   {
      DragPointCurve curve(owner, 1);
      const auto p1 = AddPoint(curve, 1.f, 2.f, 3.f, true);
      p1->m_slingshot = false;
      p1->SetAutoTextureCoordinate(false);
      p1->SetTextureCoordinateU(0.25f);
      p1->m_uiLocked = true;
      p1->m_uiVisible = false;
      AddPoint(curve, 4.f, 5.f, 6.f);

      InMemStream stream;
      {
         BiffWriter writer(&stream, nullptr);
         curve.SavePoints(writer);
         writer.EndObject();
         CHECK_FALSE(writer.HasError());
      }

      DragPointCurve loaded(owner, 1);
      BiffReader reader(stream.Data(), static_cast<uint32_t>(stream.Size()), CURRENT_FILE_FORMAT_VERSION, nullptr, 0);
      reader.AsObject(
         [&loaded](int tag, IObjectReader& fieldReader)
         {
            if (tag == FID(DPNT))
               loaded.LoadPointToken(fieldReader);
            return true;
         });
      CHECK_FALSE(reader.HasError());

      const auto& points = loaded.GetPoints();
      REQUIRE(points.size() == 2);
      CHECK(points[0]->GetX() == 1.f);
      CHECK(points[0]->GetY() == 2.f);
      CHECK(points[0]->GetZ() == 3.f);
      CHECK(points[0]->IsSmooth() == true);
      CHECK(points[0]->IsAutoTextureCoordinate() == false);
      CHECK(points[0]->GetTextureCoordinateU() == 0.25f);
      CHECK(points[0]->m_uiLocked == true);
      CHECK(points[0]->m_uiVisible == false);
      CHECK(points[1]->GetX() == 4.f);
      CHECK(points[1]->GetY() == 5.f);
      CHECK(points[1]->GetZ() == 6.f);
   }

   owner->Release();
}
