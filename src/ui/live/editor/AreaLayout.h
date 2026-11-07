// license:GPLv3+

#pragma once

// Blender-style area layout tree of the live editor.
//
// Pure C++20 plus nlohmann JSON: no ImGui and no EditorUI includes, so it can be unit tested.
//
// All public functions that take a node (`leaf`, `split`, `keep`, `consume`) take the stable AreaNode::id,
// never a vector index: indices change when the tree is compacted after a mutation, ids never do.
// Ids are unique within a session (drawn from a process wide counter, never reused), so an id kept by a
// drag or a menu can safely be re-resolved every frame: if Find(id) is null, the node is gone.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

enum class AreaType : uint8_t
{
   Outliner = 0,
   Properties = 1,
   Script = 2,
   PartLibrary = 3,
   Viewport = 4,
   ViewportBackdrop = 5,
   ViewportPreview = 6,
   Blueprint = 7,
   Count
};

// The 3D area family: one ViewportPane class, a fixed camera mode per type
inline bool IsViewportType(const AreaType type) { return type == AreaType::Viewport || type == AreaType::ViewportBackdrop || type == AreaType::ViewportPreview; }

// Names used in the layout JSON ("Outliner", "Properties", "Script", "PartLibrary", "Viewport")
const char* AreaTypeJsonName(AreaType type);
bool AreaTypeFromJsonName(const std::string& name, AreaType& type);

// Blender naming: Vertical = divider line vertical, areas side by side
enum class SplitAxis : uint8_t
{
   Vertical,
   Horizontal
};

struct AreaRect
{
   float x0 = 0.f, y0 = 0.f, x1 = 0.f, y1 = 0.f;
   float Width() const { return x1 - x0; }
   float Height() const { return y1 - y0; }
};

struct AreaNode
{
   uint32_t id = 0; // stable, never reused within a session (ImGui IDs, drag targets)
   int parent = -1;
   bool isSplit = false;
   // split
   SplitAxis axis = SplitAxis::Vertical;
   int child[2] = { -1, -1 };
   uint8_t fixed = 1; // child (0/1) whose size is stored
   float size = 0.f; // logical px of child[fixed] along axis
   // leaf
   AreaType type = AreaType::Viewport;
   bool hidden = false;
   // Opaque per-leaf payload, persisted with the layout ("data" key) and cloned on split
   // (e.g. a viewport leaf carries its 3D view setup: camera pose, shading, ...)
   nlohmann::json data;
};

// Result of AreaLayout::Compute for one leaf. Hidden leaves, leaves of an invisible subtree and leaves not
// shown because another leaf is maximized have visible == false and an empty rect.
struct LeafRect
{
   int id = -1;
   AreaType type = AreaType::Viewport;
   AreaRect rect;
   bool visible = false;
};

// Result of AreaLayout::Compute for one split whose two children are both visible.
struct SplitterRect
{
   int nodeId = -1;
   SplitAxis axis = SplitAxis::Vertical;
   uint8_t fixed = 1;
   AreaRect bounds; // rect of the whole split node (its cross extent is the splitter length)
   float pos = 0.f; // divider coordinate along the axis (x for Vertical, y for Horizontal), whole pixels
   AreaRect grab; // grab strip: 6 * dpi wide, centred on the divider, spans bounds across the axis
   float minPos = 0.f, maxPos = 0.f; // allowed range of pos so that both children keep their minimum size

   // Logical size to store in AreaNode::size for a divider at the given position
   float SizeForPos(float p, float dpi) const;
};

class AreaLayout // value type, copyable (each saved layout slot holds one)
{
public:
   static constexpr int kMaxDepth = 8; // split levels from the root to a leaf
   static constexpr int kMaxLeaves = 8;

   // Minimum sizes, logical px (multiplied by dpi in Compute)
   static constexpr float kPanelMinW = 140.f, kPanelMinH = 100.f;
   static constexpr float kViewportMinW = 240.f, kViewportMinH = 180.f;
   static constexpr float kGrabWidth = 6.f; // splitter grab strip

   static constexpr float kDefaultOutlinerWidth = 200.f; // first-run default; saved layouts keep their stored width
   static constexpr float kDefaultPropertiesWidth = 280.f; // == PropertiesPanel::PaneWidth

   AreaLayout(); // default: the 'Layout' preset (index 0)
   static AreaLayout Default(int index); // The preset layouts: 0 Layout, 1 Library, 2 Scripting, 3 Desktop Backdrop, 4 Blueprint

   // Parses a serialized layout. On failure returns false, logs PLOGW once, sets LastError() and leaves
   // *this unchanged.
   bool FromJson(const nlohmann::json& j);
   nlohmann::json ToJson() const;
   const std::string& LastError() const { return m_lastError; }

   // Depth <= kMaxDepth; <= kMaxLeaves leaves; split sizes finite and > 0; consistent node links.
   bool Validate() const;
   bool Validate(std::string& why) const;

   // --- Queries ---

   const std::vector<AreaNode>& Nodes() const { return m_nodes; }
   int Root() const { return m_root; } // index into Nodes()
   const AreaNode* Find(int id) const; // null if the id is not in the tree
   int FindLeaf(AreaType type) const; // id of the first leaf of that type (hidden or not), -1 if none
   void FindLeaves(AreaType type, std::vector<int>& out) const; // ids of all leaves of that type, in tree order
   int ParentId(int id) const; // id of the parent split, -1 for the root or an unknown id
   int LeafCount() const;
   bool IsHosted(AreaType type) const { return FindLeaf(type) >= 0; }
   // Structural equality (types, axes, sizes, hidden flags), ignoring node ids
   bool operator==(const AreaLayout& other) const;
   bool operator!=(const AreaLayout& other) const { return !(*this == other); }

   // Rects of every leaf (in order, first child first) and splitter strips of every split with two visible
   // children. bounds is the tree rect, dpi scales the logical sizes. If maximizedLeafId names a visible
   // leaf, only that leaf is shown, on the whole bounds, and there are no splitters.
   void Compute(AreaRect bounds, float dpi, std::vector<LeafRect>& leaves, std::vector<SplitterRect>& splitters, int maximizedLeafId = -1) const;

   // Id of the visible leaf under the point (half open rects), -1 if none. Works on the computed rects.
   static int LeafAt(const std::vector<LeafRect>& leaves, float x, float y);

   // --- Mutations (tree is compacted afterwards, ids survive) ---

   // The leaf exists and the tree stays within kMaxDepth / kMaxLeaves.
   bool CanSplit(int leaf) const;
   // Replaces the leaf by a split of the leaf (keeps its id) and a new leaf of the same type (Blender
   // duplicates the area). The new leaf is the split's fixed child: newSizePx (logical) is its extent
   // along the axis. Returns false (no change) if !CanSplit.
   bool Split(int leaf, SplitAxis axis, bool newOnSecondSide, float newSizePx);
   // Siblings, both leaves, both visible (not hidden)
   bool CanJoin(int keepLeaf, int consumeLeaf) const;
   // Replaces the parent split by keep (keeps its id); consume is removed. Returns false (no change) if !CanJoin.
   bool Join(int keep, int consume);
   // Closes a leaf (not the root): its parent split is replaced by the sibling subtree, which takes the freed space
   bool CanClose(int leaf) const;
   bool Close(int leaf);
   // Swaps the children (whole subtrees) of a split and sets fixed = 1 - fixed, so each side keeps its size
   bool Swap(int split);
   // Any leaf may take any type (duplicates allowed: each leaf hosts its own view instance)
   bool SetType(int leaf, AreaType type);
   bool SetHidden(int leaf, bool hidden);
   // Mutable access to a leaf's opaque persisted payload; nullptr if the id is absent or a split
   nlohmann::json* LeafData(int id);
   // Stored size (logical px) of child[fixed] of a split, clamped to a positive finite value
   bool SetSplitSize(int split, float logicalPx);

private:
   explicit AreaLayout(std::nullptr_t) { } // empty tree (no root), to be filled by the factories

   std::vector<AreaNode> m_nodes;
   int m_root = -1;
   std::string m_lastError;

   int IndexOf(int id) const;
   void Compact();
   bool Vis(int idx, bool ignoreHidden) const;
   void CollectLeaves(int idx, std::vector<int>& out) const;
   int Depth(int idx) const;
   bool ValidateImpl(std::string& why) const;
   void Layout(int idx, AreaRect r, float dpi, bool ignoreHidden, std::vector<LeafRect>& leaves, std::vector<SplitterRect>* splitters) const;
   void MinSize(int idx, float dpi, bool ignoreHidden, float& w, float& h) const;
};
