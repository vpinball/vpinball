// license:GPLv3+

#pragma once

#include "input/InputManager.h"

#include "imgui/imgui.h"
#include "imguizmo/ImGuizmo.h"
#include "imgui_markdown/imgui_markdown.h"

#include "LiveUI.h"
#include "editor/EditableUIPart.h"

namespace VPX::EditorUI
{

class EditorUI final
{
public:
   EditorUI(LiveUI &liveUI);
   ~EditorUI();

   void Open();
   bool IsOpened() const { return m_isOpened; }
   void Render3D();
   void RenderUI();
   void Close();
   
   bool IsPreview() const { return m_camMode == ViewMode::PreviewCam; }

   bool IsInspectMode() const { return m_table->m_liveBaseTable != nullptr; }

private:
   // UI Context
   LiveUI &m_liveUI;
   VPinball *m_app;
   Player *m_player;
   PinTable *m_table; // The table displayed by the player
   InputManager *m_pininput;
   Renderer *m_renderer;
   struct Selection
   {
      enum SelectionType
      {
         S_NONE,
         S_CAMERA,
         S_MATERIAL,
         S_IMAGE,
         S_EDITABLE,
         S_RENDERPROBE
      } type = S_NONE;
      union
      {
         int camera;
         Material *material;
         Texture *image;
         RenderProbe *renderprobe;
         int ball_index;
      };
      std::shared_ptr<EditableUIPart> uiPart;

      Selection() { }
      Selection(SelectionType t, int ball)
      {
         type = t;
         ball_index = ball;
      }
      Selection(std::shared_ptr<EditableUIPart> data)
      {
         type = S_EDITABLE;
         uiPart = data;
      }
      Selection(Material *data)
      {
         type = S_MATERIAL;
         material = data;
      }
      Selection(Texture *data)
      {
         type = S_IMAGE;
         image = data;
      }
      Selection(RenderProbe *data)
      {
         type = S_RENDERPROBE;
         renderprobe = data;
      }
      bool operator==(Selection s) const
      {
         if (type != s.type)
            return false;
         switch (type)
         {
         case S_NONE: return true;
         case S_CAMERA: return camera == s.camera;
         case S_MATERIAL: return material == s.material;
         case S_IMAGE: return image == s.image;
         case S_EDITABLE: return uiPart == s.uiPart;
         case S_RENDERPROBE: return renderprobe == s.renderprobe;
         }
         assert(false);
         return false;
      }
   } m_selection;

   // Decorated editable parts
   vector<std::shared_ptr<EditableUIPart>> m_editables;
   void UpdateEditableList();

   // Main UI frame & panels
   void UpdateOutlinerUI();
   void UpdatePropertyUI();

   // Popups & Modals
   void UpdateRendererInspectionModal();

   // Properties
   void TableProperties(PropertyPane &props);
   void ImageProperties(PropertyPane &props, Texture* image);
   void RenderProbeProperties(PropertyPane &props, RenderProbe* probe);
   void CameraProperties(PropertyPane &props, int bgSet);
   void MaterialProperties(PropertyPane &props, Material* material);

   // Enter/Exit edit mode (manage table backup, dynamic mode,...)
   void ResetCameraFromPlayer();

   // Undo support
   void PushUndo(IEditable *part, unsigned int editId);
   IEditable *m_lastUndoPart = nullptr;
   unsigned int m_lastUndoId = 0;

   // Add/Remove parts
   void DeleteSelection();

   // Outliner
   float m_outliner_width = 0.0f;
   string m_outlinerFilter;
   bool m_outlinerSelectLiveTab = true;
   bool IsOutlinerFiltered(const string &name) const;

   // Properties
   bool m_propertiesSelectLiveTab = true;

   // Rendering
   float m_menubar_height = 0.0f;
   float m_toolbar_height = 0.0f;
   enum PhysicOverlay
   {
      PO_NONE,
      PO_SELECTED,
      PO_ALL
   } m_physOverlay = PO_NONE;
   bool m_selectionOverlay = true;
   enum SelectionFilter
   {
      SF_Playfield = 0x0002,
      SF_Primitives = 0x0004,
      SF_Lights = 0x0008,
      SF_Flashers = 0x0010
   };
   int m_selectionFilter = 0xFFFF;

   // UI state
   bool m_isOpened = false;
   bool m_showPlumbDialog = false;
   bool m_flyMode = false;
   bool m_showRendererInspection = false;
   uint32_t m_OpenUITime = 0; // Used to delay keyboard shortcut
   uint32_t m_StartTime_msec = 0; // Used for timed splash overlays
   enum class Units
   {
      VPX, Metric, Imperial
   } m_units = Units::VPX;

   // 3D editor
   ImGuizmo::OPERATION m_gizmoOperation = ImGuizmo::NONE;
   ImGuizmo::MODE m_gizmoMode = ImGuizmo::WORLD;
   bool GetSelectionTransform(Matrix3D &transform) const;
   void SetSelectionTransform(const Matrix3D &transform, bool clearPosition = false, bool clearScale = false, bool clearRotation = false) const;

   // Editor camera
   ViewMode m_camMode = ViewMode::PreviewCam;
   bool m_perspectiveCam = true;
   Renderer::ShadeMode m_shadeMode = Renderer::ShadeMode::Default;
   enum class PredefinedView
   {
      None, Front, Back, Right, Left, Top, Bottom
   } m_predefinedView = PredefinedView::None;
   Matrix3D m_camView, m_camProj;
   float m_camDistance;

   class RenderContext : public EditorRenderContext
   {
   public:
      RenderContext(Player *player, ImDrawList *drawlist, ViewMode viewMode, Renderer::ShadeMode shadeMode, bool needsLiveTableSync);
      ~RenderContext() override = default;

      bool NeedsLiveTableSync() const { return m_needsLiveTableSync; }
      ImU32 GetColor(bool selected) const { return selected ? IM_COL32(255, 128, 0, 255) : IM_COL32_BLACK; };
      bool IsSelected() const override { return m_isSelected; }
      bool IsShowInvisible() const override;
      ViewMode GetViewMode() const override { return m_viewMode; }
      ImDrawList *GetDrawList() const override { return m_drawlist; }

      ImVec2 Project(const Vertex3Ds &v) const override;
      void DrawLine(const Vertex3Ds &a, const Vertex3Ds &b, ImU32 color) const override;
      void DrawCircle(const Vertex3Ds &center, const Vertex3Ds &x, const Vertex3Ds &y, float radius, ImU32 color) const override;
      void DrawHitObjects(IEditable *editable) const override;
      void DrawWireframe(IEditable *editable) const override;

      bool m_isSelected = false;

   private:
      Player *m_player;
      ImDrawList *const m_drawlist;
      const ViewMode m_viewMode;
      const Renderer::ShadeMode m_shadeMode;
      const bool m_needsLiveTableSync;
   };
};

}
