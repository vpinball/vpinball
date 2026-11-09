// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/flasher.h"
#include "parts/PartGroup.h"
#include "parts/pintable.h"
#include "renderer/Texture.h"
#include "ui/live/editor/ops/ReferenceImageOp.h"

#include "doctest.h"

#include <filesystem>

using namespace VPX::EditorUI;
using namespace VPX::EditorUI::ReferenceImageMath;

TEST_CASE("Reference image math")
{
   SUBCASE("unit conversion uses the VPX scale")
   {
      CHECK(ToVPU(1.f, Unit::Inches) == doctest::Approx(50. / 1.0625));
      CHECK(ToVPU(2.54f, Unit::Centimeters) == doctest::Approx(ToVPU(1.f, Unit::Inches)));
      CHECK(ToVPU(51.435f, Unit::Centimeters) == doctest::Approx(INCHESTOVPU(20.25f)).epsilon(1e-5)); // WPC playfield width
      CHECK(FromVPU(ToVPU(42.f, Unit::Centimeters), Unit::Centimeters) == doctest::Approx(42.f));
      CHECK(FromVPU(ToVPU(42.f, Unit::Inches), Unit::Inches) == doctest::Approx(42.f));
      CHECK(FromVPU(ToVPU(10.f, Unit::Inches), Unit::Centimeters) == doctest::Approx(25.4f));
   }

   SUBCASE("height from the pixel aspect")
   {
      CHECK(HeightForWidth(30.f, 3000, 2000) == doctest::Approx(20.f));
      CHECK(HeightForWidth(51.4f, 1000, 2000) == doctest::Approx(102.8f));
      CHECK(HeightForWidth(30.f, 0, 2000) == 0.f);
      CHECK(HeightForWidth(30.f, 3000, 0) == 0.f);
   }

   SUBCASE("aspect mismatch tolerance")
   {
      CHECK(AspectMismatch(30.f, 20.f, 3000, 2000) == doctest::Approx(0.f));
      CHECK_FALSE(IsAspectMismatch(30.f, 20.f, 3000, 2000));
      CHECK_FALSE(IsAspectMismatch(30.f, 20.1f, 3000, 2000)); // 0.5%: rounding of a measured size
      CHECK(IsAspectMismatch(30.f, 21.f, 3000, 2000)); // ~4.8%
      CHECK(AspectMismatch(30.f, 21.f, 3000, 2000) == doctest::Approx(1. - (30. / 21.) / 1.5).epsilon(1e-4));
      CHECK(IsAspectMismatch(20.f, 30.f, 3000, 2000)); // Swapped width and height
      CHECK_FALSE(IsAspectMismatch(0.f, 20.f, 3000, 2000)); // Nothing entered yet
      CHECK_FALSE(IsAspectMismatch(30.f, 20.f, 0, 0)); // No image
   }

   SUBCASE("size from the scan resolution")
   {
      CHECK(SizeFromDpi(600, 300.f) == doctest::Approx(INCHESTOVPU(2.f)));
      CHECK(SizeFromDpi(600, 0.f) == 0.f);
   }

   SUBCASE("corner generation")
   {
      const Rect rect { Vertex2D(100.f, 200.f), 40.f, 60.f };
      const std::array<Vertex2D, 4> c = RectCorners(rect);
      // Same order as a new flasher (Flasher::InitShape)
      CHECK(c[0].x == 80.f);
      CHECK(c[0].y == 170.f);
      CHECK(c[1].x == 80.f);
      CHECK(c[1].y == 230.f);
      CHECK(c[2].x == 120.f);
      CHECK(c[2].y == 230.f);
      CHECK(c[3].x == 120.f);
      CHECK(c[3].y == 170.f);
      const Rect back = BoundsOf(c);
      CHECK(back.center.x == 100.f);
      CHECK(back.center.y == 200.f);
      CHECK(back.width == 40.f);
      CHECK(back.height == 60.f);
      CHECK(BoundsOf(vector<Vertex2D>()).width == 0.f);
   }

   SUBCASE("uniform scale from a single axis tool")
   {
      // A drag along X: each frame asks for the scale from the start of the drag (1, 1), the result must follow X on both axes
      Vertex2D current(1.f, 1.f), last(1.f, 1.f);
      for (const float k : { 1.1f, 1.25f, 1.5f, 1.4f })
      {
         const Vertex2D requested(k, 1.f);
         current = UniformScale(requested, current, last);
         last = requested;
         CHECK(current.x == doctest::Approx(k));
         CHECK(current.y == doctest::Approx(k));
      }
      // Then along Y, starting from the result (a new drag)
      const Vertex2D start = current;
      for (const float k : { 0.9f, 0.8f })
      {
         const Vertex2D requested(start.x, start.y * k);
         current = UniformScale(requested, current, last);
         last = requested;
         CHECK(current.x == doctest::Approx(start.x * k));
         CHECK(current.y == doctest::Approx(start.y * k));
      }
      // The same request again (mouse held still, typed value) keeps the scale: no flip back and forth between frames
      const Vertex2D held = current;
      for (int frame = 0; frame < 3; frame++)
      {
         current = UniformScale(last, current, last);
         CHECK(current.x == doctest::Approx(held.x));
         CHECK(current.y == doctest::Approx(held.y));
      }
      // A uniform request (free scale, cancel back to the start) is kept as is, also for a non square current scale
      CHECK(UniformScale(Vertex2D(2.f, 2.f), Vertex2D(1.f, 1.f), Vertex2D(1.f, 1.f)).x == 2.f);
      const Vertex2D ratio = UniformScale(Vertex2D(3.f, 1.5f), Vertex2D(2.f, 1.f), Vertex2D(2.f, 1.f));
      CHECK(ratio.x == doctest::Approx(3.f));
      CHECK(ratio.y == doctest::Approx(1.5f));
   }

   SUBCASE("fit to the image aspect keeps the centre and the width")
   {
      // A stretched rectangle with a stray corner
      const vector<Vertex2D> points { Vertex2D(10.f, 10.f), Vertex2D(10.f, 90.f), Vertex2D(130.f, 70.f), Vertex2D(110.f, 10.f) };
      const Rect bounds = BoundsOf(points);
      const Rect fitted = FitToAspect(bounds, 2000, 1000);
      CHECK(fitted.center.x == doctest::Approx(70.f));
      CHECK(fitted.center.y == doctest::Approx(50.f));
      CHECK(fitted.width == doctest::Approx(120.f));
      CHECK(fitted.height == doctest::Approx(60.f));
      const Rect unchanged = FitToAspect(bounds, 0, 1000);
      CHECK(unchanged.height == bounds.height);
   }
}

namespace
{
   Flasher *FindFlasher(PinTable *table, const string &name)
   {
      for (IEditable *const part : table->GetParts())
         if (part->GetItemType() == eItemFlasher && part->GetName() == name)
            return static_cast<Flasher *>(part);
      return nullptr;
   }

   void CheckRectangle(const Flasher *flasher, const Rect &expected)
   {
      REQUIRE(flasher->m_curve.GetPoints().size() == 4);
      const std::array<Vertex2D, 4> corners = RectCorners(expected);
      for (size_t i = 0; i < 4; i++)
      {
         const DragPoint *const point = flasher->m_curve.GetPoints()[i].get();
         CHECK(point->GetX() == doctest::Approx(corners[i].x).epsilon(1e-4));
         CHECK(point->GetY() == doctest::Approx(corners[i].y).epsilon(1e-4));
         CHECK(point->GetZ() == 0.f);
         CHECK_FALSE(point->IsSmooth());
      }
   }

   void CheckReloaded(PinTable *table, const Flasher *original, const string &imageName)
   {
      Flasher *const flasher = FindFlasher(table, original->GetName());
      REQUIRE(flasher != nullptr);
      CHECK(ReferenceImageOp::IsReferenceImage(flasher));
      REQUIRE(flasher->GetPartGroup() != nullptr);
      CHECK(flasher->GetPartGroup()->GetName() == ReferenceImageOp::GroupName);
      CHECK(flasher->GetPartGroup()->m_d.m_playerModeVisibilityMask == 0);
      CHECK(flasher->GetPartGroup()->GetPlayerModeVisibilityMask() == 0);
      CHECK(ReferenceImageOp::FindGroup(table) == flasher->GetPartGroup());
      CHECK(flasher->m_d.m_szImageA == imageName);
      CHECK(table->GetImage(imageName) != nullptr);
      CHECK(flasher->m_d.m_renderMode == FlasherData::RenderMode::FLASHER);
      CHECK(flasher->m_d.m_imagealignment == ImageModeWrap);
      CHECK(flasher->m_d.m_color == original->m_d.m_color);
      CHECK(flasher->m_d.m_alpha == original->m_d.m_alpha);
      CHECK(flasher->m_d.m_height == original->m_d.m_height);
      CHECK(flasher->m_d.m_rotX == original->m_d.m_rotX);
      CHECK(flasher->m_d.m_rotZ == original->m_d.m_rotZ);
      CHECK(flasher->IsUILocked() == original->IsUILocked());
      CheckRectangle(flasher, ReferenceImageOp::GetRect(original));
   }
}

TEST_CASE("Reference image flasher")
{
   CComObject<PinTable> *table;
   CComObject<PinTable>::CreateInstance(&table);
   table->AddRef();
   TestFileFeedback feedback;
   REQUIRE(SUCCEEDED(table->LoadGameFromFilename(GetAssetPath() / "test000-default-table.vpx", feedback)));
   const size_t partCount = table->GetParts().size();

   // Images from files, as the Add dialog imports them
   Texture *const scan = table->ImportImage(GetAssetPath() / "test000-default-table-Ref.webp", "refscan_test");
   REQUIRE(scan != nullptr);
   REQUIRE(scan->m_width > 0);
   REQUIRE(scan->m_height > 0);
   Texture *const other = table->ImportImage(GetAssetPath() / "test000-stereo-sbs-Ref.webp", "refother_test");
   REQUIRE(other != nullptr);

   // Group: created once, at the root, hidden in every play mode
   CHECK(ReferenceImageOp::FindGroup(table) == nullptr);
   PartGroup *const group = ReferenceImageOp::CreateGroup(table);
   REQUIRE(group != nullptr);
   group->Release(); // Creation reference (the editor hands it to the undo record)
   CHECK(group->GetName() == ReferenceImageOp::GroupName);
   CHECK(group->GetPartGroup() == nullptr);
   CHECK(group->m_d.m_playerModeVisibilityMask == 0);
   CHECK(group->GetReferenceSpace() == PartGroupData::SpaceReference::SR_PLAYFIELD);
   CHECK(ReferenceImageOp::FindGroup(table) == group);
   CHECK(ReferenceImageOp::CreateGroup(table) == nullptr); // The name is taken

   // Flasher at the real size, centred on the playfield, flat on it
   const float width = ToVPU(40.f, Unit::Centimeters);
   const float height = HeightForWidth(width, scan->m_width, scan->m_height);
   const Vertex2D center(0.5f * (table->m_left + table->m_right), 0.5f * (table->m_top + table->m_bottom));
   Flasher *const flasher = ReferenceImageOp::CreateFlasher(table, group, scan->m_name, center, width, height);
   REQUIRE(flasher != nullptr);
   flasher->Release();
   CHECK(table->GetParts().size() == partCount + 2);
   CHECK(flasher->GetPartGroup() == group);
   CHECK(ReferenceImageOp::IsReferenceImage(flasher));
   CHECK_FALSE(ReferenceImageOp::IsReferenceImage(group));
   CHECK(flasher->m_d.m_renderMode == FlasherData::RenderMode::FLASHER);
   CHECK(flasher->m_d.m_imagealignment == ImageModeWrap);
   CHECK(flasher->m_d.m_szImageA == scan->m_name);
   CHECK(flasher->m_d.m_color != 0);
   CHECK(flasher->m_d.m_alpha > 0);
   CHECK(flasher->m_d.m_alpha <= 100);
   CHECK(flasher->m_d.m_intensity_scale == 1.f);
   CHECK(flasher->m_d.m_addBlend == FlasherData::AB_NONE);
   CHECK(flasher->m_d.m_isVisible);
   CHECK(flasher->m_d.m_displayTexture);
   CHECK_FALSE(flasher->m_desktopBackdrop);
   CHECK(flasher->m_d.m_height > 0.f);
   CHECK(flasher->m_d.m_height <= 1.f);
   CHECK(flasher->m_d.m_rotX == 0.f);
   CHECK(flasher->m_d.m_rotY == 0.f);
   CHECK(flasher->m_d.m_rotZ == 0.f);
   CheckRectangle(flasher, { center, width, height });
   CHECK(flasher->GetCenter().x == doctest::Approx(center.x));
   CHECK(flasher->GetCenter().y == doctest::Approx(center.y));

   // Lock: only a reference image's lock is a reference image lock
   CHECK_FALSE(ReferenceImageOp::IsLocked(flasher));
   flasher->SetUILock(true);
   CHECK(ReferenceImageOp::IsLocked(flasher));
   flasher->SetUILock(false);

   SUBCASE("nested groups and plain flashers")
   {
      PartGroup *const sub = PartGroup::COMCreate();
      sub->Init(0.f, 0.f, false);
      sub->SetName("RefSub");
      table->AddPart(sub);
      sub->Release();
      sub->SetPartGroup(group);
      Flasher *const nested = Flasher::COMCreate();
      nested->Init(10.f, 10.f, false);
      nested->SetName("NestedFlasher");
      table->AddPart(nested);
      nested->Release();
      nested->SetPartGroup(sub);
      CHECK(ReferenceImageOp::IsReferenceImage(nested));
      // New and pasted parts never go to the reference images group or a group nested in it
      CHECK(ReferenceImageOp::IsReferenceGroup(group));
      CHECK(ReferenceImageOp::IsReferenceGroup(sub));
      CHECK_FALSE(ReferenceImageOp::IsReferenceGroup(nullptr));
      PartGroup *const other = PartGroup::COMCreate();
      other->Init(0.f, 0.f, false);
      other->SetName("OtherLayer");
      table->AddPart(other);
      other->Release();
      CHECK_FALSE(ReferenceImageOp::IsReferenceGroup(other));
      nested->SetPartGroup(other);
      CHECK_FALSE(ReferenceImageOp::IsReferenceImage(nested));
      nested->SetPartGroup(nullptr);
      CHECK_FALSE(ReferenceImageOp::IsReferenceImage(nested));
      nested->SetUILock(true);
      CHECK_FALSE(ReferenceImageOp::IsLocked(nested));
      CHECK_FALSE(ReferenceImageOp::Rotate(nested, 90.f, Vertex2D(0.f, 0.f), false));
   }

   SUBCASE("fit to image keeps the centre, rotation and width")
   {
      // Stretch it, move a corner away and place it upright, then fit
      ReferenceImageOp::SetSize(flasher, Vertex2D(width, 2.f * height));
      flasher->m_curve.GetPoints()[2]->SetX(flasher->m_curve.GetPoints()[2]->GetX() + 7.f);
      flasher->m_d.m_rotX = 90.f;
      flasher->m_d.m_rotZ = 30.f;
      flasher->m_d.m_height = 200.f;
      const Rect stretched = ReferenceImageOp::GetRect(flasher);
      REQUIRE(ReferenceImageOp::FitToImage(flasher));
      CheckRectangle(flasher, { stretched.center, stretched.width, HeightForWidth(stretched.width, scan->m_width, scan->m_height) });
      CHECK(flasher->m_d.m_rotX == 90.f);
      CHECK(flasher->m_d.m_rotZ == 30.f);
      CHECK(flasher->m_d.m_height == 200.f);
      const Rect fitted = ReferenceImageOp::GetRect(flasher);
      CHECK_FALSE(IsAspectMismatch(fitted.width, fitted.height, scan->m_width, scan->m_height));

      flasher->m_d.m_szImageA = "no such image";
      CHECK_FALSE(ReferenceImageOp::FitToImage(flasher));
   }

   SUBCASE("replace image keeps the transform and fits the new aspect")
   {
      flasher->m_d.m_rotY = 45.f;
      const Rect before = ReferenceImageOp::GetRect(flasher);
      REQUIRE(ReferenceImageOp::ReplaceImage(flasher, other->m_name));
      CHECK(flasher->m_d.m_szImageA == other->m_name);
      CHECK(flasher->m_d.m_rotY == 45.f);
      CheckRectangle(flasher, { before.center, before.width, HeightForWidth(before.width, other->m_width, other->m_height) });
   }

   SUBCASE("rotate turns the part instead of its points")
   {
      const Rect before = ReferenceImageOp::GetRect(flasher);
      // About its own centre: the points stay, only the Z rotation changes
      CHECK(ReferenceImageOp::Rotate(flasher, 30.f, Vertex2D(0.f, 0.f), true));
      CHECK(flasher->m_d.m_rotZ == 30.f);
      CheckRectangle(flasher, before);
      // About another pivot: the centre turns around it (same direction as DragPointCurve::RotatePoints)
      const Vertex2D pivot(center.x - 100.f, center.y);
      CHECK(ReferenceImageOp::Rotate(flasher, 90.f, pivot, false));
      CHECK(flasher->m_d.m_rotZ == 120.f);
      const Rect after = ReferenceImageOp::GetRect(flasher);
      CHECK(after.center.x == doctest::Approx(pivot.x).epsilon(1e-4));
      CHECK(after.center.y == doctest::Approx(pivot.y + 100.f).epsilon(1e-4));
      CHECK(after.width == doctest::Approx(before.width));
      CHECK(after.height == doctest::Approx(before.height));
   }

   SUBCASE("a copied reference image stays in the reference images group")
   {
      PartGroup *const other = PartGroup::COMCreate();
      other->Init(0.f, 0.f, false);
      other->SetName("OtherLayer");
      table->AddPart(other);
      other->Release();
      // Copy / paste and duplicate go through the part's saved data, which names its source group
      Flasher *const copy = Flasher::COMCreate();
      LoadPartFromStream(copy, SavePartToStream(flasher));
      CHECK(copy->m_onLoadExpectedPartGroup == ReferenceImageOp::GroupName);
      CHECK(ReferenceImageOp::PasteGroup(table, nullptr, copy, other) == group);
      // Any other part goes to the target group, as before
      Flasher *const plain = Flasher::COMCreate();
      plain->Init(0.f, 0.f, false);
      plain->SetName("PlainFlasher");
      table->AddPart(plain);
      plain->Release();
      plain->SetPartGroup(other);
      Flasher *const plainCopy = Flasher::COMCreate();
      LoadPartFromStream(plainCopy, SavePartToStream(plain));
      CHECK(ReferenceImageOp::PasteGroup(table, nullptr, plainCopy, other) == other);
      plainCopy->Release();
      // Pasted into a table without reference images: the group is created, hidden in play
      PinTable *const otherTable = CreateTestTable();
      PartGroup *const created = ReferenceImageOp::PasteGroup(otherTable, nullptr, copy, nullptr);
      REQUIRE(created != nullptr);
      CHECK(created == ReferenceImageOp::FindGroup(otherTable));
      CHECK(created->m_d.m_playerModeVisibilityMask == 0);
      CHECK(ReferenceImageOp::PasteGroup(otherTable, nullptr, copy, nullptr) == created); // Found the second time
      otherTable->Release();
      copy->Release();
   }

   SUBCASE("the win32 style rotate command leaves a tilted image alone")
   {
      flasher->m_d.m_rotX = 90.f;
      const Rect before = ReferenceImageOp::GetRect(flasher);
      CHECK(ReferenceImageOp::Rotate(flasher, 30.f, Vertex2D(0.f, 0.f), false));
      CHECK(flasher->m_d.m_rotZ == 0.f);
      CheckRectangle(flasher, before);
   }

   SUBCASE("the win32 style scale command keeps the aspect")
   {
      const Rect before = ReferenceImageOp::GetRect(flasher);
      CHECK(ReferenceImageOp::Scale(flasher, 2.f, 1.f, Vertex2D(0.f, 0.f), true));
      CheckRectangle(flasher, { before.center, 2.f * before.width, 2.f * before.height });
      CHECK(ReferenceImageOp::Scale(flasher, 1.f, 0.25f, Vertex2D(0.f, 0.f), true));
      CheckRectangle(flasher, { before.center, 0.5f * before.width, 0.5f * before.height });
   }

   SUBCASE("survives a VPX and a VPZ save / load round trip, with no format change")
   {
      flasher->m_d.m_rotX = 90.f;
      flasher->m_d.m_height = 150.f;
      flasher->SetUILock(true);
      std::error_code ec;

      const std::filesystem::path vpx = GetTestTmpDir() / "refimage-roundtrip.vpx";
      std::filesystem::remove(vpx, ec);
      table->m_filename = vpx;
      TestFileFeedback saveFeedback;
      REQUIRE(SUCCEEDED(table->Save(saveFeedback)));
      {
         CComObject<PinTable> *reloaded;
         CComObject<PinTable>::CreateInstance(&reloaded);
         reloaded->AddRef();
         TestFileFeedback loadFeedback;
         REQUIRE(SUCCEEDED(reloaded->LoadGameFromFilename(vpx, loadFeedback)));
         CHECK(reloaded->GetParts().size() == table->GetParts().size());
         CheckReloaded(reloaded, flasher, scan->m_name);
         reloaded->Release();
      }
      std::filesystem::remove(vpx, ec);

      const std::filesystem::path vpz = GetTestTmpDir() / "refimage-roundtrip.vpz";
      std::filesystem::remove_all(vpz, ec);
      REQUIRE(std::filesystem::create_directories(vpz, ec));
      TestFileFeedback packFeedback;
      REQUIRE(SUCCEEDED(table->SaveToJSON(vpz, packFeedback)));
      {
         CComObject<PinTable> *reloaded;
         CComObject<PinTable>::CreateInstance(&reloaded);
         reloaded->AddRef();
         TestFileFeedback loadFeedback;
         REQUIRE(SUCCEEDED(reloaded->LoadGameFromFilename(vpz, loadFeedback)));
         CHECK(reloaded->GetParts().size() == table->GetParts().size());
         CheckReloaded(reloaded, flasher, scan->m_name);
         reloaded->Release();
      }
      std::filesystem::remove_all(vpz, ec);
   }

   table->Release();
}
