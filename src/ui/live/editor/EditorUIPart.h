#pragma once

#include "PropertyPane.h"
#include "core/ieditable.h"
#include "parts/dragpoint.h"

#include <functional>
#include <initializer_list>
#include <type_traits>
#include <utility>

class DragPointCurve;

namespace VPX::EditorUI
{

enum class ViewMode
{
   PreviewCam,
   EditorCam,
   DesktopBackdrop
};


class EditorRenderContext
{
public:
   virtual ~EditorRenderContext() { }

   virtual bool NeedsLiveTableSync() const = 0;
   virtual ImU32 GetColor(bool selected) const = 0;
   virtual bool IsSelected() const = 0;
   virtual bool IsShowInvisible() const = 0;
   virtual ViewMode GetViewMode() const = 0;
   virtual ImDrawList* GetDrawList() const = 0;
   virtual ImVec2 Project(const Vertex3Ds& point) const = 0;

   virtual void DrawLine(const Vertex3Ds& a, const Vertex3Ds& b, ImU32 color) const = 0;
   virtual void DrawCircle(const Vertex3Ds& center, const Vertex3Ds& x, const Vertex3Ds& y, float radius, ImU32 color) const = 0;
   virtual void DrawHitObjects(IEditable* editable) const = 0;
   virtual void DrawWireframe(IEditable* editable) const = 0;
};


// Editor interface exposed to a UIPart while its drag point curve is being edited (point edit mode).
// It gives access to the current point selection and to the editor-side side effects
// (undo marking, renderable/physics refresh) needed by point editing commands.
class DragPointEditContext
{
public:
   virtual ~DragPointEditContext() = default;

   virtual const vector<DragPoint*>& GetSelectedPoints() const = 0;
   virtual void BeginPointEdit() = 0; // Mark the edited part for undo, before modifying its curve points
   virtual void EndPointEdit() = 0; // Commit point modifications: refresh curve bounds, rendering & physics
};


class EditorUIPart
{
public:
   EditorUIPart(IEditable* editable)
      : m_editable(editable)
   {
   }
   virtual ~EditorUIPart() { }

   IEditable* GetEditable() const { return m_editable; }

   const string& GetOutlinerPath() const { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) { m_outlinerPath = path; }

   enum TransformMask
   {
      TM_TransX = 0x0001,
      TM_TransY = 0x0002,
      TM_TransZ = 0x0004,
      TM_RotX = 0x0008,
      TM_RotY = 0x0010,
      TM_RotZ = 0x0020,
      TM_ScaleX = 0x0040,
      TM_ScaleY = 0x0080,
      TM_ScaleZ = 0x0100,
      TM_ScaleAll = 0x0200,

      TM_None = 0x0000,
      TM_TransAny = 0x0007,
      TM_RotAny = 0x0038,
      TM_ScaleAny = 0x03c0,
      TM_TRSAny = 0x03ff
   };
   virtual TransformMask GetTransform(Matrix3D& transform) = 0;
   virtual void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) = 0;

   virtual void Render(const EditorRenderContext& ctx) = 0;

   // Returns the editable drag point curve of this part, nullptr if it does not have one
   virtual DragPointCurve* GetDragPointCurve() { return nullptr; }

   // Returns the Z coordinate at which a point of this part's drag point curve should be displayed
   // (drag point curves are 2D in the table XY plane, the display height is part specific)
   virtual float GetDragPointZ(const DragPoint* point) const { return point->m_v.z; }

   // Drag point edit mode: the editor sets the context while this part's curve is being edited
   void SetPointEditContext(DragPointEditContext* ctx) { m_pointEditCtx = ctx; }

   // Adds a drag point on the curve segment nearest to pos (in table XY coordinates), returning it (nullptr on failure)
   DragPoint* AddPointOnCurve(const Vertex2D& pos);

   virtual void UpdatePropertyPane(PropertyPane& props) = 0;

protected:
   // Renders the 'Curve' section of the property pane (only shown while in drag point edit mode)
   void UpdateCurveSection(PropertyPane& props);

   // Inserts a new drag point on the curve segment nearest to pos (default implementation for closed 2D curves)
   virtual void InsertPointOnCurve(const Vertex2D& pos);
   // Whether points added to the curve are smooth by default
   virtual bool IsNewPointSmooth() const { return false; }
   // Whether the curve points can be flagged as slingshot segments (surfaces only)
   virtual bool HasSlingshotSegments() const { return false; }
   // Renders the read only Z coordinate field of a single selected drag point in the curve section
   virtual void UpdatePointZField(PropertyPane& props, DragPoint* point);

private:
   void SetSelectedPointsSmooth(bool smooth);
   void FlipSelectedPoints(bool flipX);
   void AlignSelectedPoints(bool onX, bool toMax);
   void ToggleSelectedPointsSlingshot();

   DragPointEditContext* m_pointEditCtx = nullptr;
   IEditable* const m_editable;
   string m_outlinerPath;
};


// Base implementation for UI part attached to a scene part.
// 
// The UI Part needs to control the part's visibility in the rendered view, so it shadows the part's visibility
// fields in its m_d member (there may be more than one, for example for top & side of walls), eventually syncing
// it from a live table state.
template <class T> class EditableUIPart : public EditorUIPart
{
public:
   using Data = std::remove_cv_t<std::remove_reference_t<decltype(std::declval<T&>().m_d)>>;

   EditableUIPart(T* part, std::initializer_list<bool Data::*> visibilityFields = {})
      : EditorUIPart(part)
      , m_part(part)
   {
      for (bool Data::* field : visibilityFields)
         m_visibilityFields.emplace_back(field, part->m_d.*field);
   }

   ~EditableUIPart() override
   {
      for (auto& visibilityField : m_visibilityFields)
         m_part->m_d.*visibilityField.field = visibilityField.visible;
   }

   void Render(const EditorRenderContext& ctx) override
   {
      const bool isUIVisible = m_part->IsUIVisible(true);

      // If running against a playing live table, we need to update visibility from it as the script may have changed it
      if (ctx.NeedsLiveTableSync())
         for (size_t i = 0; i < m_visibilityFields.size(); i++)
            m_visibilityFields[i].visible = m_part->m_d.*(m_visibilityFields[i].field);

      // Render the overlay wireframe for selection or if the part would be invisible and the user turned on forced overlay for invisible parts (not rendered by the main renderer)
      if (isUIVisible && (ctx.IsSelected() || (ctx.IsShowInvisible() && IsShadowHidden())))
      {
         for (auto& visibilityField : m_visibilityFields)
            m_part->m_d.*visibilityField.field = true;
         RenderOverlay(ctx);
      }

      // Enable the main renderer rendering
      const bool extraHidden = IsExtraHidden();
      for (size_t i = 0; i < m_visibilityFields.size(); i++)
         m_part->m_d.*(m_visibilityFields[i].field) = isUIVisible && m_visibilityFields[i].visible && !extraHidden;
   }

protected:
   T* const m_part;

   virtual void RenderOverlay(const EditorRenderContext& ctx) = 0;

   // Additional non-shadowed condition hiding the part (e.g. Trigger shape 'None')
   virtual bool IsExtraHidden() const { return false; }

   // Editor-side value of a render-visibility field: the editor value for this part, or the model value for the other (startup/live) instance or for unshadowed fields.
   bool GetVisibility(const T* part) const { return GetVisibility(part, m_visibilityFields[0].field); }
   bool GetVisibility(const T* part, bool Data::* field) const
   {
      const size_t i = FieldIndex(field);
      return (part == m_part && i != m_visibilityFields.size()) ? m_visibilityFields[i].visible : part->m_d.*field;
   }
   void SetVisibility(T* part, const bool v) { SetVisibility(part, v, m_visibilityFields[0].field); }
   void SetVisibility(T* part, const bool v, bool Data::* field)
   {
      const size_t i = FieldIndex(field);
      if (part == m_part && i != m_visibilityFields.size())
         m_visibilityFields[i].visible = v;
      else
         part->m_d.*field = v;
   }

private:
   size_t FieldIndex(bool Data::* field) const
   {
      for (size_t i = 0; i < m_visibilityFields.size(); i++)
         if (m_visibilityFields[i].field == field)
            return i;
      return m_visibilityFields.size();
   }

   bool IsShadowHidden() const
   {
      return IsExtraHidden() || std::ranges::all_of(m_visibilityFields, [](const auto& v) { return !v.visible; });
   }

   struct VisibilityField
   {
      bool Data::* field; // the m_d.xxxx field in the scene part
      bool visible; // the editor value overriding the scen part value
   };
   vector<VisibilityField> m_visibilityFields;
};


// Fallback UI part for editables without a dedicated implementation
class BaseUIPart final : public EditorUIPart
{
public:
   BaseUIPart(IEditable* editable)
      : EditorUIPart(editable)
   {
   }

   TransformMask GetTransform(Matrix3D& transform) override { return TM_None; }
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override { }

   void Render(const EditorRenderContext& ctx) override { }

   void UpdatePropertyPane(PropertyPane& props) override { }
};
}
