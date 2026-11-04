// license:GPLv3+

#include "core/stdafx.h"

#include "ui/live/editor/AreaLayout.h"

#include "doctest.h"

#include <cmath>
#include <limits>

using json = nlohmann::json;

namespace
{
struct Computed
{
   std::vector<LeafRect> leaves;
   std::vector<SplitterRect> splitters;
   const LeafRect* Leaf(const AreaLayout& l, const AreaType t, const int nth = 0) const
   {
      std::vector<int> ids;
      l.FindLeaves(t, ids);
      if (nth < 0 || nth >= static_cast<int>(ids.size()))
         return nullptr;
      for (const LeafRect& r : leaves)
         if (r.id == ids[nth])
            return &r;
      return nullptr;
   }
};

Computed Compute(const AreaLayout& l, const float w = 1920.f, const float h = 1080.f, const float dpi = 1.f, const int maximized = -1)
{
   Computed c;
   l.Compute({ 0.f, 0.f, w, h }, dpi, c.leaves, c.splitters, maximized);
   return c;
}

// Id of the leaf sibling to keptLeaf under their common parent split
int SiblingLeafId(const AreaLayout& l, const int keptLeaf)
{
   const int parentId = l.ParentId(keptLeaf);
   REQUIRE(parentId >= 0);
   int keptIdx = -1;
   for (size_t i = 0; i < l.Nodes().size(); ++i)
      if (static_cast<int>(l.Nodes()[i].id) == keptLeaf)
         keptIdx = static_cast<int>(i);
   REQUIRE(keptIdx >= 0);
   const AreaNode* parent = l.Find(parentId);
   REQUIRE(parent != nullptr);
   const int sibIdx = parent->child[0] == keptIdx ? parent->child[1] : parent->child[0];
   return static_cast<int>(l.Nodes()[sibIdx].id);
}

json Leaf(const char* name, const bool hidden = false)
{
   json j = { { "editor", name } };
   if (hidden)
      j["hidden"] = true;
   return j;
}

json Split(const char* axis, const char* fixed, const json& size, const json& a, const json& b)
{
   return { { "split", axis }, { "fixed", fixed }, { "size", size }, { "children", json::array({ a, b }) } };
}

// Valid base: Outliner 200 | Viewport | Properties 280
json ValidJson() { return Split("vertical", "first", 200, Leaf("Outliner"), Split("vertical", "second", 280, Leaf("Viewport"), Leaf("Properties"))); }

void CheckRejected(const json& j, const char* msgPart = nullptr)
{
   AreaLayout l;
   const AreaLayout before = l;
   CHECK_FALSE(l.FromJson(j));
   CHECK(l == before); // unchanged on failure
   CHECK(l.Validate());
   if (msgPart)
      CHECK(l.LastError().find(msgPart) != std::string::npos);
}
}

TEST_CASE("AreaLayout default and JSON round trip")
{
   SUBCASE("default is Outliner | Viewport | Properties")
   {
      const AreaLayout a;
      CHECK(a.Validate());
      CHECK(a.LeafCount() == 3);
      CHECK(a.FindLeaf(AreaType::Outliner) >= 0);
      CHECK(a.FindLeaf(AreaType::Viewport) >= 0);
      CHECK(a.FindLeaf(AreaType::Properties) >= 0);

      const std::string text = a.ToJson().dump();
      AreaLayout b;
      REQUIRE(b.FromJson(json::parse(text)));
      CHECK(b == a);
      CHECK(b.Validate());
      CHECK(b.ToJson() == a.ToJson());
   }

   SUBCASE("a tree with a nested split survives the round trip")
   {
      AreaLayout a;
      const int props = a.FindLeaf(AreaType::Properties);
      REQUIRE(a.Split(props, SplitAxis::Horizontal, true, 380.f));
      const int newLeaf = SiblingLeafId(a, props);
      REQUIRE(a.SetType(newLeaf, AreaType::Script));
      CHECK(a.LeafCount() == 4);

      const json j = a.ToJson();
      CHECK(j["children"][1]["children"][1]["split"] == "horizontal");
      CHECK(j["children"][1]["children"][1]["fixed"] == "second");
      CHECK(j["children"][1]["children"][1]["size"] == 380.f);
      CHECK(j["children"][1]["children"][1]["children"][1]["editor"] == "Script");

      AreaLayout b;
      REQUIRE(b.FromJson(json::parse(j.dump())));
      CHECK(b == a);
   }

   SUBCASE("hidden flag survives a JSON round trip")
   {
      AreaLayout a;
      REQUIRE(a.SetHidden(a.FindLeaf(AreaType::Outliner), true));
      AreaLayout b;
      REQUIRE(b.FromJson(a.ToJson()));
      CHECK(b == a);
      CHECK(b.Find(b.FindLeaf(AreaType::Outliner))->hidden);
   }

   SUBCASE("Close replaces the parent split by the sibling")
   {
      AreaLayout a;
      REQUIRE(a.Close(a.FindLeaf(AreaType::Outliner)));
      CHECK(a.LeafCount() == 2);
      CHECK_FALSE(a.IsHosted(AreaType::Outliner));
      CHECK(a.Validate());
   }
}

TEST_CASE("AreaLayout compute")
{
   const AreaLayout l;

   SUBCASE("1920x1080 at dpi 1")
   {
      const Computed c = Compute(l);
      REQUIRE(c.leaves.size() == 3);
      const LeafRect& out = *c.Leaf(l, AreaType::Outliner);
      const LeafRect& vp = *c.Leaf(l, AreaType::Viewport);
      const LeafRect& props = *c.Leaf(l, AreaType::Properties);
      CHECK(out.rect.x0 == 0.f);
      CHECK(out.rect.x1 == 200.f);
      CHECK(vp.rect.x0 == 200.f);
      CHECK(vp.rect.x1 == 1640.f);
      CHECK(props.rect.x0 == 1640.f);
      CHECK(props.rect.x1 == 1920.f);
      for (const LeafRect& r : c.leaves)
      {
         CHECK(r.visible);
         CHECK(r.rect.y0 == 0.f);
         CHECK(r.rect.y1 == 1080.f);
      }
      REQUIRE(c.splitters.size() == 2);
      CHECK(c.splitters[0].pos == 200.f);
      CHECK(c.splitters[1].pos == 1640.f);
      CHECK(c.splitters[1].axis == SplitAxis::Vertical);
      CHECK(c.splitters[1].grab.x0 == 1637.f);
      CHECK(c.splitters[1].grab.x1 == 1643.f);
      CHECK(c.splitters[1].grab.y1 == 1080.f);
      // Dragging the Properties splitter 100 px left gives a stored width of 380
      CHECK(c.splitters[1].SizeForPos(1540.f, 1.f) == 380.f);
      CHECK(c.splitters[0].SizeForPos(250.f, 1.f) == 250.f);
      CHECK(c.splitters[1].minPos == 200.f + 240.f);
      CHECK(c.splitters[1].maxPos == 1920.f - 140.f);
   }

   SUBCASE("dpi scales sizes, positions are whole pixels")
   {
      const Computed c = Compute(l, 3840.f, 2160.f, 2.f);
      CHECK(c.Leaf(l, AreaType::Outliner)->rect.x1 == 400.f);
      CHECK(c.Leaf(l, AreaType::Properties)->rect.x0 == 3840.f - 560.f);
      const Computed odd = Compute(l, 1001.f, 700.f, 1.25f);
      for (const SplitterRect& s : odd.splitters)
         CHECK(s.pos == std::round(s.pos));
   }

   SUBCASE("hidden leaves take no space and their splitter is gone")
   {
      AreaLayout h = l;
      REQUIRE(h.SetHidden(h.FindLeaf(AreaType::Outliner), true));
      const Computed c = Compute(h);
      CHECK_FALSE(c.Leaf(h, AreaType::Outliner)->visible);
      CHECK(c.Leaf(h, AreaType::Viewport)->rect.x0 == 0.f);
      CHECK(c.splitters.size() == 1);
      REQUIRE(h.SetHidden(h.FindLeaf(AreaType::Properties), true));
      const Computed c2 = Compute(h);
      CHECK(c2.splitters.empty());
      CHECK(c2.Leaf(h, AreaType::Viewport)->rect.x1 == 1920.f);
      // exactly reversible
      REQUIRE(h.SetHidden(h.FindLeaf(AreaType::Outliner), false));
      REQUIRE(h.SetHidden(h.FindLeaf(AreaType::Properties), false));
      const Computed c3 = Compute(h);
      CHECK(c3.leaves.size() == Compute(l).leaves.size());
      CHECK(c3.Leaf(h, AreaType::Viewport)->rect.x0 == 200.f);
      CHECK(c3.Leaf(h, AreaType::Viewport)->rect.x1 == 1640.f);
   }

   SUBCASE("minimum sizes at 600x400")
   {
      const Computed c = Compute(l, 600.f, 400.f);
      const LeafRect& out = *c.Leaf(l, AreaType::Outliner);
      const LeafRect& vp = *c.Leaf(l, AreaType::Viewport);
      const LeafRect& props = *c.Leaf(l, AreaType::Properties);
      CHECK(out.rect.Width() >= AreaLayout::kPanelMinW);
      CHECK(vp.rect.Width() >= AreaLayout::kViewportMinW);
      CHECK(props.rect.Width() >= AreaLayout::kPanelMinW);
      CHECK(out.rect.Width() + vp.rect.Width() + props.rect.Width() == 600.f);
      CHECK(out.rect.x1 == 200.f);
      CHECK(vp.rect.x1 == 440.f); // Properties squeezed to its minimum, Viewport keeps 240
      CHECK(props.rect.Width() == 160.f);
      CHECK(vp.rect.Height() >= AreaLayout::kViewportMinH);
   }

   SUBCASE("smaller than all minimums: proportional shrink, never negative, covers the bounds")
   {
      for (const float w : { 500.f, 300.f, 100.f, 10.f, 0.f })
      {
         const Computed c = Compute(l, w, 50.f);
         float sum = 0.f, prevX1 = 0.f;
         for (const LeafRect& r : c.leaves)
         {
            CHECK(r.rect.Width() >= 0.f);
            CHECK(r.rect.Height() >= 0.f);
            CHECK(r.rect.x0 == prevX1); // no gaps, no seams
            prevX1 = r.rect.x1;
            sum += r.rect.Width();
         }
         CHECK(sum == w);
      }
   }

   SUBCASE("maximize shows only that leaf on the whole rect, without splitters")
   {
      const int propsId = l.FindLeaf(AreaType::Properties);
      const Computed c = Compute(l, 1920.f, 1080.f, 1.f, propsId);
      CHECK(c.splitters.empty());
      REQUIRE(c.leaves.size() == 3);
      int visible = 0;
      for (const LeafRect& r : c.leaves)
      {
         if (r.visible)
         {
            ++visible;
            CHECK(r.id == propsId);
            CHECK(r.rect.x0 == 0.f);
            CHECK(r.rect.x1 == 1920.f);
         }
      }
      CHECK(visible == 1);
      // unknown / hidden id: ignored
      CHECK(Compute(l, 1920.f, 1080.f, 1.f, 9999999).splitters.size() == 2);
      // restoring gives the same rects exactly
      const Computed a = Compute(l), b = Compute(l);
      CHECK(a.leaves[1].rect.x0 == b.leaves[1].rect.x0);
   }

   SUBCASE("LeafAt")
   {
      const Computed c = Compute(l);
      CHECK(AreaLayout::LeafAt(c.leaves, 10.f, 10.f) == l.FindLeaf(AreaType::Outliner));
      CHECK(AreaLayout::LeafAt(c.leaves, 200.f, 10.f) == l.FindLeaf(AreaType::Viewport)); // half open
      CHECK(AreaLayout::LeafAt(c.leaves, 1700.f, 500.f) == l.FindLeaf(AreaType::Properties));
      CHECK(AreaLayout::LeafAt(c.leaves, 2000.f, 500.f) == -1);
      CHECK(AreaLayout::LeafAt(c.leaves, 10.f, -1.f) == -1);
   }
}

TEST_CASE("AreaLayout split")
{
   AreaLayout l;

   SUBCASE("a split clones the leaf type, on the requested side")
   {
      const int vpId = l.FindLeaf(AreaType::Viewport);
      REQUIRE(l.CanSplit(vpId));
      REQUIRE(l.Split(vpId, SplitAxis::Vertical, true, 300.f));
      CHECK(l.Validate());
      std::vector<int> viewports;
      l.FindLeaves(AreaType::Viewport, viewports);
      REQUIRE(viewports.size() == 2);
      CHECK(viewports[0] == vpId); // the split leaf keeps its id, new leaf on the second side
      const int newVp = viewports[1];
      const Computed c = Compute(l);
      const LeafRect* first = c.Leaf(l, AreaType::Viewport, 0);
      const LeafRect* second = c.Leaf(l, AreaType::Viewport, 1);
      REQUIRE(first != nullptr);
      REQUIRE(second != nullptr);
      CHECK(second->rect.Width() == 300.f);
      CHECK(second->rect.x1 == 1640.f);
      CHECK(first->rect.x1 == 1340.f);
      CHECK(l.ParentId(vpId) == l.ParentId(newVp));
   }

   SUBCASE("new leaf on the first (left/top) side keeps the given size")
   {
      REQUIRE(l.Split(l.FindLeaf(AreaType::Viewport), SplitAxis::Horizontal, false, 250.f));
      const Computed c = Compute(l);
      CHECK(c.Leaf(l, AreaType::Viewport, 0)->rect.y0 == 0.f);
      CHECK(c.Leaf(l, AreaType::Viewport, 0)->rect.Height() == 250.f);
      CHECK(c.Leaf(l, AreaType::Viewport, 1)->rect.y0 == 250.f);
   }

   SUBCASE("split of a panel leaf clones the panel type")
   {
      REQUIRE(l.Split(l.FindLeaf(AreaType::Outliner), SplitAxis::Horizontal, true, 300.f));
      const Computed c = Compute(l);
      CHECK(c.Leaf(l, AreaType::Outliner, 0)->rect.Height() == 780.f);
      CHECK(c.Leaf(l, AreaType::Outliner, 1)->rect.Height() == 300.f);
      CHECK(c.Leaf(l, AreaType::Outliner, 1)->rect.Width() == 200.f);
   }

   SUBCASE("refused at the leaf count limit")
   {
      // Split the Viewport until kMaxLeaves leaves exist (3 + 5 splits)
      for (int i = 0; i < AreaLayout::kMaxLeaves - 3; ++i)
      {
         std::vector<int> vps;
         l.FindLeaves(AreaType::Viewport, vps);
         REQUIRE(l.Split(vps.back(), SplitAxis::Vertical, true, 100.f));
      }
      CHECK(l.LeafCount() == AreaLayout::kMaxLeaves);
      CHECK(l.Validate());
      const AreaLayout before = l;
      for (const AreaNode& n : l.Nodes())
         if (!n.isSplit)
         {
            CHECK_FALSE(l.CanSplit(static_cast<int>(n.id)));
            CHECK_FALSE(l.Split(static_cast<int>(n.id), SplitAxis::Vertical, true, 100.f));
         }
      CHECK(l == before);
   }

   SUBCASE("unknown ids and split nodes are refused")
   {
      CHECK_FALSE(l.CanSplit(-1));
      CHECK_FALSE(l.CanSplit(123456789));
      CHECK_FALSE(l.CanSplit(static_cast<int>(l.Nodes()[l.Root()].id))); // a split node
   }
}

TEST_CASE("AreaLayout join")
{
   const AreaLayout original;

   SUBCASE("refused for non-siblings, subtrees, hidden and self")
   {
      AreaLayout l = original;
      const int out = l.FindLeaf(AreaType::Outliner);
      const int vp = l.FindLeaf(AreaType::Viewport);
      const int props = l.FindLeaf(AreaType::Properties);
      const int innerSplit = l.ParentId(vp);
      REQUIRE(innerSplit >= 0);

      CHECK_FALSE(l.CanJoin(out, props)); // not siblings
      CHECK_FALSE(l.CanJoin(out, vp));
      CHECK_FALSE(l.CanJoin(out, innerSplit)); // a subtree
      CHECK_FALSE(l.CanJoin(innerSplit, out));
      CHECK_FALSE(l.CanJoin(props, props));
      CHECK_FALSE(l.CanJoin(props, 99999999));
      CHECK_FALSE(l.Join(out, props));
      CHECK(l == original);

      REQUIRE(l.SetHidden(props, true));
      CHECK_FALSE(l.CanJoin(vp, props)); // hidden
      REQUIRE(l.SetHidden(props, false));
      CHECK(l.CanJoin(vp, props));
   }

   SUBCASE("a split then a join restores the layout")
   {
      AreaLayout l = original;
      const int vp = l.FindLeaf(AreaType::Viewport);
      REQUIRE(l.Split(vp, SplitAxis::Vertical, true, 300.f));
      const int newVp = SiblingLeafId(l, vp);
      REQUIRE(l.CanJoin(vp, newVp));
      REQUIRE(l.Join(vp, newVp));
      std::vector<int> vps;
      l.FindLeaves(AreaType::Viewport, vps);
      CHECK(vps.size() == 1);
      CHECK(vps[0] == vp);
      CHECK(l.Validate());
      CHECK(l == original);
   }

   SUBCASE("the Viewport may also be consumed")
   {
      AreaLayout l = original;
      const int vp = l.FindLeaf(AreaType::Viewport);
      REQUIRE(l.Split(vp, SplitAxis::Vertical, true, 300.f));
      const int newVp = SiblingLeafId(l, vp);
      REQUIRE(l.CanJoin(newVp, vp)); // either direction, any type
      REQUIRE(l.Join(newVp, vp));
      CHECK(l.Find(vp) == nullptr);
      CHECK(l.Validate());
      CHECK(l.LeafCount() == 3);
   }

   SUBCASE("joining panel siblings")
   {
      AreaLayout l = original;
      const int props = l.FindLeaf(AreaType::Properties);
      REQUIRE(l.Split(props, SplitAxis::Horizontal, true, 380.f));
      const int second = SiblingLeafId(l, props);
      REQUIRE(l.CanJoin(props, second));
      REQUIRE(l.Join(second, props)); // either side may win
      CHECK(l.Find(props) == nullptr);
      CHECK(l.Validate());
      CHECK(l.LeafCount() == 3);
      const Computed c = Compute(l);
      CHECK(c.Leaf(l, AreaType::Properties)->rect.Height() == 1080.f);
   }
}

TEST_CASE("AreaLayout swap")
{
   const AreaLayout original = []
   {
      AreaLayout l;
      REQUIRE(l.Split(l.FindLeaf(AreaType::Properties), SplitAxis::Horizontal, true, 380.f));
      return l;
   }();

   SUBCASE("swap twice is the identity")
   {
      AreaLayout l = original;
      const int vp = l.FindLeaf(AreaType::Viewport);
      const int inner = l.ParentId(vp);
      REQUIRE(l.Swap(inner));
      CHECK(l != original);
      CHECK(l.Validate());
      REQUIRE(l.Swap(inner));
      CHECK(l == original);
      CHECK(l.ToJson() == original.ToJson());
   }

   SUBCASE("swap mirrors the rects and keeps the sizes")
   {
      AreaLayout l = original;
      const int vp = l.FindLeaf(AreaType::Viewport);
      REQUIRE(l.Swap(l.ParentId(vp)));
      const Computed c = Compute(l);
      CHECK(c.Leaf(l, AreaType::Properties)->rect.x0 == 200.f);
      CHECK(c.Leaf(l, AreaType::Properties)->rect.Width() == 280.f);
      CHECK(c.Leaf(l, AreaType::Viewport)->rect.x1 == 1920.f);
   }

   SUBCASE("swap of a split of leaves and of subtrees; leaves are refused")
   {
      AreaLayout l = original;
      CHECK_FALSE(l.Swap(l.FindLeaf(AreaType::Viewport)));
      CHECK_FALSE(l.Swap(-1));
      const int colSplit = l.ParentId(SiblingLeafId(l, l.FindLeaf(AreaType::Properties)));
      REQUIRE(l.Swap(colSplit));
      const Computed c = Compute(l);
      // After the swap the cloned Properties (380 px) is on top
      CHECK(c.Leaf(l, AreaType::Properties, 1)->rect.y0 == 0.f);
      CHECK(c.Leaf(l, AreaType::Properties, 1)->rect.Height() == 380.f);
      CHECK(c.Leaf(l, AreaType::Properties, 0)->rect.y0 == 380.f);
      // root: swaps the Outliner and the whole right subtree
      AreaLayout r = original;
      REQUIRE(r.Swap(static_cast<int>(r.Nodes()[r.Root()].id)));
      CHECK(r.Validate());
      const Computed rc = Compute(r);
      CHECK(rc.Leaf(r, AreaType::Outliner)->rect.x0 == 1720.f);
      CHECK(rc.Leaf(r, AreaType::Outliner)->rect.Width() == 200.f);
   }
}

TEST_CASE("AreaLayout set type and hidden")
{
   SUBCASE("duplicates are allowed: any leaf takes any type")
   {
      AreaLayout l;
      const int out = l.FindLeaf(AreaType::Outliner);
      const int props = l.FindLeaf(AreaType::Properties);
      const int vp = l.FindLeaf(AreaType::Viewport);
      REQUIRE(l.SetType(out, AreaType::Properties)); // two Properties
      CHECK(l.Find(out)->type == AreaType::Properties);
      std::vector<int> props2;
      l.FindLeaves(AreaType::Properties, props2);
      CHECK(props2.size() == 2);
      REQUIRE(l.SetType(props, AreaType::Viewport)); // two Viewports
      std::vector<int> vps;
      l.FindLeaves(AreaType::Viewport, vps);
      CHECK(vps.size() == 2);
      REQUIRE(l.SetType(vp, AreaType::Script));
      CHECK(l.Validate());
      CHECK(l.FindLeaf(AreaType::Outliner) < 0);
   }

   SUBCASE("the hidden flag stays with the leaf when its type changes")
   {
      AreaLayout l;
      const int out = l.FindLeaf(AreaType::Outliner);
      REQUIRE(l.SetHidden(out, true));
      REQUIRE(l.SetType(out, AreaType::Script));
      CHECK(l.Find(out)->type == AreaType::Script);
      CHECK(l.Find(out)->hidden);
      CHECK(l.Validate());
   }

   SUBCASE("SetType refuses unknown types and splits, not leaves of any type")
   {
      AreaLayout l;
      const AreaLayout before = l;
      const int out = l.FindLeaf(AreaType::Outliner);
      CHECK_FALSE(l.SetType(out, AreaType::Count));
      CHECK_FALSE(l.SetType(static_cast<int>(l.Nodes()[l.Root()].id), AreaType::Script)); // a split
      CHECK_FALSE(l.SetHidden(-1, true));
      CHECK(l == before);
      CHECK(l.Validate());
   }

   SUBCASE("Viewport leaves may be retyped and hidden")
   {
      AreaLayout l;
      const int vp = l.FindLeaf(AreaType::Viewport);
      REQUIRE(l.SetType(vp, AreaType::Outliner));
      CHECK(l.Validate());
      CHECK(l.FindLeaf(AreaType::Viewport) < 0);
      REQUIRE(l.SetHidden(vp, true));
      CHECK(l.Find(vp)->hidden);
   }
}

TEST_CASE("AreaLayout invalid JSON is rejected")
{
   REQUIRE(AreaLayout().FromJson(ValidJson()));

   SUBCASE("two Viewports, no Viewport, duplicates and hidden Viewport are all valid")
   {
      AreaLayout l;
      REQUIRE(l.FromJson(Split("vertical", "first", 200, Leaf("Viewport"), Split("vertical", "second", 280, Leaf("Viewport"), Leaf("Properties")))));
      REQUIRE(l.FromJson(Split("vertical", "first", 200, Leaf("Outliner"), Leaf("Properties"))));
      REQUIRE(l.FromJson(Split("vertical", "first", 200, Leaf("Outliner"), Split("vertical", "second", 280, Leaf("Viewport"), Leaf("Outliner")))));
      REQUIRE(l.FromJson(Split("vertical", "first", 200, Leaf("Outliner"), Leaf("Viewport", true))));
   }
   SUBCASE("NaN, infinite, zero, negative and non numeric sizes")
   {
      CheckRejected(Split("vertical", "first", std::numeric_limits<double>::quiet_NaN(), Leaf("Outliner"), Leaf("Viewport")));
      CheckRejected(Split("vertical", "first", std::numeric_limits<double>::infinity(), Leaf("Outliner"), Leaf("Viewport")));
      CheckRejected(Split("vertical", "first", 0, Leaf("Outliner"), Leaf("Viewport")));
      CheckRejected(Split("vertical", "first", -10, Leaf("Outliner"), Leaf("Viewport")));
      CheckRejected(Split("vertical", "first", "200", Leaf("Outliner"), Leaf("Viewport")));
      CheckRejected(Split("vertical", "first", nullptr, Leaf("Outliner"), Leaf("Viewport")));
   }
   SUBCASE("depth over the limit")
   {
      json chain = Leaf("Viewport");
      for (int i = 0; i < 9; ++i)
         chain = Split("vertical", "second", 200, chain, Leaf("Outliner"));
      CheckRejected(chain, "deeper");
   }
   SUBCASE("more than kMaxLeaves leaves")
   {
      json j = Leaf("Viewport");
      for (const char* n : { "Outliner", "Properties", "Script", "PartLibrary", "Viewport", "Outliner", "Properties", "Script" })
         j = Split("vertical", "second", 100, j, Leaf(n));
      CheckRejected(j);
   }
   SUBCASE("malformed nodes")
   {
      CheckRejected(json::array());
      CheckRejected(nullptr);
      CheckRejected(Leaf("Nonsense"));
      CheckRejected(json { { "editor", 3 } });
      CheckRejected(json::object());
      CheckRejected(Split("diagonal", "first", 200, Leaf("Outliner"), Leaf("Viewport")));
      CheckRejected(Split("vertical", "third", 200, Leaf("Outliner"), Leaf("Viewport")));
      CheckRejected(json { { "split", "vertical" }, { "fixed", "first" }, { "size", 200 }, { "children", json::array({ Leaf("Viewport") }) } });
      CheckRejected(json { { "split", "vertical" }, { "fixed", "first" }, { "size", 200 }, { "children", json::array({ Leaf("Viewport"), Leaf("Outliner"), Leaf("Script") }) } });
      json badHidden = Leaf("Outliner");
      badHidden["hidden"] = "yes";
      CheckRejected(Split("vertical", "first", 200, badHidden, Leaf("Viewport")));
   }
   SUBCASE("a single leaf of any type is a valid layout")
   {
      for (const char* type : { "Viewport", "Outliner", "Properties", "Script", "PartLibrary" })
      {
         AreaLayout l;
         REQUIRE(l.FromJson(Leaf(type)));
         CHECK(l.LeafCount() == 1);
         CHECK(l.Validate());
      }
   }
}

TEST_CASE("AreaLayout node ids are stable across compaction")
{
   AreaLayout l;
   const int out = l.FindLeaf(AreaType::Outliner);
   const int vp = l.FindLeaf(AreaType::Viewport);
   const int props = l.FindLeaf(AreaType::Properties);

   REQUIRE(l.Split(vp, SplitAxis::Vertical, true, 300.f));
   const int newVp = SiblingLeafId(l, vp);
   const int splitId = l.ParentId(newVp);
   REQUIRE(l.Join(vp, newVp));

   // The split node and the new leaf are gone, so the vector was compacted and indices moved ...
   CHECK(l.Find(newVp) == nullptr);
   CHECK(l.Find(splitId) == nullptr);
   CHECK(l.Nodes().size() == 5);
   // ... but ids of the surviving nodes are unchanged
   CHECK(l.FindLeaf(AreaType::Outliner) == out);
   CHECK(l.FindLeaf(AreaType::Viewport) == vp);
   CHECK(l.FindLeaf(AreaType::Properties) == props);
   CHECK(l.Find(vp)->type == AreaType::Viewport);
   CHECK(l.ParentId(vp) == l.ParentId(props));

   // Compaction really moves indices: join a Properties away after a split
   AreaLayout w;
   const int wProps = w.FindLeaf(AreaType::Properties);
   const int wOut = w.FindLeaf(AreaType::Outliner);
   const int wVp = w.FindLeaf(AreaType::Viewport);
   REQUIRE(w.Split(wProps, SplitAxis::Horizontal, true, 380.f));
   const int wSecond = SiblingLeafId(w, wProps);
   int secondIdxBefore = -1;
   for (size_t i = 0; i < w.Nodes().size(); ++i)
      if (static_cast<int>(w.Nodes()[i].id) == wSecond)
         secondIdxBefore = static_cast<int>(i);
   REQUIRE(w.Join(wSecond, wProps));
   int secondIdxAfter = -1;
   for (size_t i = 0; i < w.Nodes().size(); ++i)
      if (static_cast<int>(w.Nodes()[i].id) == wSecond)
         secondIdxAfter = static_cast<int>(i);
   CHECK(secondIdxAfter >= 0);
   CHECK(secondIdxAfter != secondIdxBefore);
   CHECK(w.FindLeaf(AreaType::Outliner) == wOut);
   CHECK(w.FindLeaf(AreaType::Viewport) == wVp);
   CHECK(w.Find(wSecond) == &w.Nodes()[secondIdxAfter]);
   CHECK(w.Find(wProps) == nullptr);
   CHECK(w.Validate());

   // Ids are never reused, also not by a new layout
   const AreaLayout other;
   for (const AreaNode& n : other.Nodes())
   {
      CHECK(n.id != static_cast<uint32_t>(wOut));
      CHECK(n.id != static_cast<uint32_t>(wProps));
   }
}

TEST_CASE("AreaLayout split size edits")
{
   AreaLayout l;
   const int innerSplit = l.ParentId(l.FindLeaf(AreaType::Viewport));
   REQUIRE(l.SetSplitSize(innerSplit, 380.f));
   CHECK(Compute(l).Leaf(l, AreaType::Properties)->rect.x0 == 1540.f);
   CHECK_FALSE(l.SetSplitSize(innerSplit, std::numeric_limits<float>::quiet_NaN()));
   CHECK_FALSE(l.SetSplitSize(l.FindLeaf(AreaType::Viewport), 100.f)); // a leaf
   REQUIRE(l.SetSplitSize(innerSplit, -4.f));
   CHECK(l.Validate()); // clamped to a positive value
}
