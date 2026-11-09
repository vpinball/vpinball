// license:GPLv3+

#include "core/stdafx.h"
#include "SetOriginOp.h"
#include "ui/live/editor/parts/EditorUIPart.h"
#include "ui/live/editor/Selection.h"
#include "ui/live/EditorUI.h"
#include "renderer/Renderer.h"
#include "parts/primitive.h"
#include "physics/PhysicsEngine.h"

namespace VPX::EditorUI
{

namespace
{
   // Absolute snapshot of the meshes of the edited primitives (undo and redo restore it; the positions are restored by the part records)
   struct MeshState
   {
      Primitive *prim;
      vector<Vertex3D_NoTex2> vertices;
      vector<Mesh::FrameData> frames;
   };
   using MeshStates = vector<MeshState>;

   std::shared_ptr<const MeshStates> Snapshot(const vector<Primitive *> &prims)
   {
      auto states = std::make_shared<MeshStates>();
      for (Primitive *const prim : prims)
         states->push_back({ prim, prim->m_mesh.m_vertices, prim->m_mesh.m_animationFrames });
      return states;
   }

   void Restore(PinTable *const table, const MeshStates &states)
   {
      for (const MeshState &state : states)
      {
         // A part deleted since (not undoable) is gone; a mesh that changed size is another mesh (reimported): leave it
         if (std::ranges::find(table->GetParts(), static_cast<IEditable *>(state.prim)) == table->GetParts().end() || state.prim->m_mesh.m_vertices.size() != state.vertices.size())
            continue;
         state.prim->m_mesh.m_vertices = state.vertices;
         state.prim->m_mesh.m_animationFrames = state.frames;
         state.prim->OnMeshVerticesChanged();
         // The renderable and physics are reinitialized by the part record of the same undo step (it restores the position)
      }
   }

}

vector<Primitive *> SetOriginOp::SelectedPrimitives(const EditorUI &ed, int &skipped)
{
   vector<std::shared_ptr<EditorUIPart>> parts = ed.m_multiSel;
   if (parts.empty() && ed.m_selection.GetType() == Selection::S_EDITABLE && ed.m_selection.GetPart())
      parts.push_back(ed.m_selection.GetPart());
   vector<Primitive *> prims;
   skipped = 0;
   for (const auto &part : parts)
   {
      IEditable *const edit = part ? part->GetEditable() : nullptr;
      if (edit && edit->GetItemType() == eItemPrimitive)
         prims.push_back(static_cast<Primitive *>(edit));
      else
         skipped++;
   }
   return prims;
}

bool SetOriginOp::CanApply(const EditorUI &ed)
{
   if (ed.m_table->IsLocked() || ed.IsInspectMode())
      return false;
   int skipped;
   return !SelectedPrimitives(ed, skipped).empty();
}

void SetOriginOp::Apply(EditorUI &ed, const Mode mode)
{
   const char *const what = mode == Mode::Geometry ? "Set Origin to Geometry" : mode == Mode::GeometryBottom ? "Set Origin to Geometry Bottom" : "Set Origin to Scene Origin";
   if (ed.m_table->IsLocked() || ed.IsInspectMode())
   {
      ed.Report(string(what) + ": not available (table locked or inspecting the game)");
      return;
   }
   int skipped;
   const vector<Primitive *> selected = SelectedPrimitives(ed, skipped);

   // Target origins (in the space of the primitive position) of the primitives that can be edited
   struct Edit
   {
      Primitive *prim;
      Vertex3Ds target;
   };
   vector<Edit> edits;
   int noMesh = 0, zeroScale = 0, already = 0;
   for (Primitive *const prim : selected)
   {
      if (!prim->m_d.m_use3DMesh || prim->m_mesh.NumVertices() == 0)
      {
         noMesh++; // Builtin primitive: its geometry is generated around the origin, not stored
         continue;
      }
      const Vertex3Ds &sz = prim->m_d.m_vSize;
      if (fabsf(sz.x) < 1e-6f || fabsf(sz.y) < 1e-6f || fabsf(sz.z) < 1e-6f)
      {
         zeroScale++; // The geometry can not be shifted back through a flattened transform
         continue;
      }
      Vertex3Ds target(0.f, 0.f, 0.f); // Scene origin: positions are stored in the part's reference space, so (0,0,0) is the origin of the space it belongs to
      if (mode != Mode::Scene)
      {
         const Matrix3D &m = prim->RecalculateMatrices();
         Vertex3Ds lo(FLT_MAX, FLT_MAX, FLT_MAX), hi(-FLT_MAX, -FLT_MAX, -FLT_MAX);
         for (const Vertex3D_NoTex2 &v : prim->m_mesh.m_vertices)
         {
            const Vertex3Ds w = m * v;
            lo.x = min(lo.x, w.x);
            lo.y = min(lo.y, w.y);
            lo.z = min(lo.z, w.z);
            hi.x = max(hi.x, w.x);
            hi.y = max(hi.y, w.y);
            hi.z = max(hi.z, w.z);
         }
         target = Vertex3Ds(0.5f * (lo.x + hi.x), 0.5f * (lo.y + hi.y), mode == Mode::Geometry ? 0.5f * (lo.z + hi.z) : lo.z);
      }
      const Vertex3Ds d = target - prim->m_d.m_vPosition;
      if (d.Dot(d) < 1e-8f)
      {
         already++;
         continue;
      }
      edits.push_back({ prim, target });
   }

   string notes;
   const auto note = [&notes](int n, const char *msg)
   {
      if (n > 0)
         notes += (notes.empty() ? " (" : ", ") + std::to_string(n) + ' ' + msg;
   };
   note(skipped, "not a primitive");
   note(noMesh, "builtin primitive, no mesh");
   note(zeroScale, "zero scale");
   note(already, "already there");
   if (!notes.empty())
      notes += ')';
   if (edits.empty())
   {
      ed.Report(string(what) + ": nothing to do" + (notes.empty() ? " (select mesh primitives)" : notes));
      return;
   }

   vector<Primitive *> prims;
   for (const Edit &e : edits)
      prims.push_back(e.prim);
   PinTable *const table = ed.m_table;
   ed.m_undo.BeginUndo();
   for (Primitive *const prim : prims)
      ed.m_undo.MarkForUndo(prim); // Position (the mesh is not part of the part undo stream)
   ed.m_undo.MarkForAction(
      [table, before = Snapshot(prims)]() { Restore(table, *before); }, [table, prims]() -> std::function<void()> { return [table, after = Snapshot(prims)]() { Restore(table, *after); }; });

   for (const Edit &e : edits)
   {
      Primitive *const prim = e.prim;
      // world = v * S * R * T(pos): moving pos by d keeps the geometry if v moves by -d through the inverse of the linear part
      Matrix3D inv = prim->RecalculateMatrices();
      inv.Invert();
      const Vertex3Ds d = e.target - prim->m_d.m_vPosition;
      const Vertex3Ds dv = inv.MultiplyVectorNoTranslate(Vertex3Ds(-d.x, -d.y, -d.z));
      for (Vertex3D_NoTex2 &v : prim->m_mesh.m_vertices)
      {
         v.x += dv.x;
         v.y += dv.y;
         v.z += dv.z;
      }
      for (Mesh::FrameData &frame : prim->m_mesh.m_animationFrames)
         for (Mesh::VertData &v : frame.m_frameVerts)
         {
            v.x += dv.x;
            v.y += dv.y;
            v.z += dv.z;
         }
      prim->m_d.m_vPosition = e.target;
      prim->OnMeshVerticesChanged();
      prim->RecalculateMatrices();
      ed.m_renderer->ReinitRenderable(prim);
      ed.m_player->m_physics->Update(prim); // Collision mesh rebuilt from the shifted vertices and the new position
   }
   ed.m_undo.SetLabel(prims.size() == 1 ? "Set Origin: "s + prims[0]->GetName() : "Set Origin: "s + std::to_string(prims.size()) + " parts");
   ed.m_undo.EndUndo();
   ed.Report(string(what) + ": " + std::to_string(prims.size()) + (prims.size() == 1 ? " part" : " parts") + notes);
}

}
