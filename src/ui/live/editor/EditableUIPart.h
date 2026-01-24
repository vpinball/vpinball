#pragma once

#include "PropertyPane.h"
#include "core/ieditable.h"

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


class EditableUIPart
{
public:
   virtual ~EditableUIPart() { }

   virtual IEditable* GetEditable() const = 0;

   virtual const string& GetOutlinerPath() const = 0;
   virtual void SetOutlinerPath(const string& path) = 0;

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

   virtual void UpdatePropertyPane(PropertyPane& props) = 0;
};


class BaseUIPart final : public EditableUIPart
{
public:
   BaseUIPart(IEditable* editable)
      : m_editable(editable)
   {
   }
   ~BaseUIPart() override { }

   IEditable* GetEditable() const { return m_editable; }

   const string& GetOutlinerPath() const override { return m_outlinerPath; }
   void SetOutlinerPath(const string& path) override { m_outlinerPath = path; }

   TransformMask GetTransform(Matrix3D& transform) override { return TM_None; }
   void SetTransform(const vec3& pos, const vec3& scale, const vec3& rot) override { }

   void Render(const EditorRenderContext& ctx) override { }

   void UpdatePropertyPane(PropertyPane& props) override { }

private:
   IEditable* const m_editable;
   string m_outlinerPath;
};

}
