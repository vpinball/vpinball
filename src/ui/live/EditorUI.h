// license:GPLv3+

#pragma once

#include "input/pininput.h"

#include "imgui/imgui.h"
#include "imguizmo/ImGuizmo.h"
#include "imgui_markdown/imgui_markdown.h"

#include "LiveUI.h"

class EditorUI final
{
public:
   EditorUI(LiveUI &liveUI);
   ~EditorUI();

   void Open();
   bool IsOpened() const { return m_isOpened; }
   void Update();
   void Close();

   // FIXME to be moved away from EditorUI
   void OpenMainSplash();

private:
   // UI Context
   LiveUI &m_liveUI;
   VPinball *m_app;
   Player *m_player;
   PinTable *m_table; // The edited table
   PinTable *m_live_table; // The live copy of the edited table being played by the player (all properties can be changed at any time by the script)
   PinInput *m_pininput;
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
      bool is_live;
      union
      {
         int camera;
         IEditable *editable;
         Material *material;
         Texture *image;
         RenderProbe *renderprobe;
         int ball_index;
      };
      Selection() { }
      Selection(SelectionType t, bool live, int ball)
      {
         type = t;
         is_live = live;
         ball_index = ball;
      }
      Selection(bool live, IEditable *data)
      {
         type = S_EDITABLE;
         is_live = live;
         editable = data;
      }
      Selection(bool live, Material *data)
      {
         type = S_MATERIAL;
         is_live = live;
         material = data;
      }
      Selection(bool live, Texture *data)
      {
         type = S_IMAGE;
         is_live = live;
         image = data;
      }
      Selection(bool live, RenderProbe *data)
      {
         type = S_RENDERPROBE;
         is_live = live;
         renderprobe = data;
      }
      bool operator==(Selection s)
      {
         if (type != s.type || is_live != s.is_live)
            return false;
         switch (type)
         {
         case S_NONE: return true;
         case S_CAMERA: return camera == s.camera;
         case S_MATERIAL: return material == s.material;
         case S_IMAGE: return image == s.image;
         case S_EDITABLE: return editable == s.editable;
         case S_RENDERPROBE: return renderprobe == s.renderprobe;
         }
         assert(false);
         return false;
      }
   } m_selection;

   // Main UI frame & panels
   void UpdateOutlinerUI();
   void UpdatePropertyUI();

   // Popups & Modals
   void UpdateAudioOptionsModal();
   void UpdateVideoOptionsModal();
   void UpdateAnaglyphCalibrationModal();
   void UpdateRendererInspectionModal();
   void UpdatePlumbWindow();

   // UI Selection & properties
   void ImageProperties();
   void RenderProbeProperties(bool is_live);
   void CameraProperties(bool is_live);
   void MaterialProperties(bool is_live);
   void TableProperties(bool is_live);
   void BallProperties(bool is_live, Ball *startup_obj, Ball *live_obj);
   void BumperProperties(bool is_live, Bumper *startup_obj, Bumper *live_obj);
   void FlasherProperties(bool is_live, Flasher *startup_obj, Flasher *live_obj);
   void KickerProperties(bool is_live, Kicker *startup_obj, Kicker *live_obj);
   void LightProperties(bool is_live, Light *startup_light, Light *live_light);
   void PrimitiveProperties(bool is_live, Primitive *startup_obj, Primitive *live_obj);
   void RampProperties(bool is_live, Ramp *startup_obj, Ramp *live_obj);
   void RubberProperties(bool is_live, Rubber *startup_obj, Rubber *live_obj);
   void SurfaceProperties(bool is_live, Surface *startup_obj, Surface *live_obj);
   void TriggerProperties(bool is_live, Trigger *startup_obj, Trigger *live_obj);

   // Helpers for property edition
   typedef std::function<void(bool is_live, float prev, float v)> OnFloatPropChange;
   typedef std::function<void(bool is_live, int prev, int v)> OnIntPropChange;
   typedef std::function<void(bool is_live, bool prev, bool v)> OnBoolPropChange;
   typedef std::function<void(bool is_live, string prev, string v)> OnStringPropChange;
   typedef std::function<void(bool is_live, vec3& prev, vec3& v)> OnVec3PropChange;
   void PropSeparator(const char *label = nullptr);
   void PropCheckbox(const char *label, IEditable *undo_obj, bool is_live, bool *startup_v, bool *live_v, const OnBoolPropChange &chg_callback = nullptr);
   void PropFloat(const char *label, IEditable* undo_obj, bool is_live, float *startup_v, float *live_v, float step, float step_fast, const char *format = "%.3f", ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsDecimal, const OnFloatPropChange &chg_callback = nullptr);
   void PropInt(const char *label, IEditable *undo_obj, bool is_live, int *startup_v, int *live_v);
   void PropRGB(const char *label, IEditable *undo_obj, bool is_live, COLORREF *startup_v, COLORREF *live_v, ImGuiColorEditFlags flags = 0);
   void PropVec3(const char *label, IEditable* undo_obj, bool is_live, Vertex3Ds *startup_v, Vertex3Ds *live_v, const char *format = "%.3f", ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsDecimal, const OnVec3PropChange &chg_callback = nullptr);
   void PropVec3(const char *label, IEditable *undo_obj, bool is_live, float *startup_v, float *live_v, const char *format = "%.3f", ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsDecimal, const OnVec3PropChange &chg_callback = nullptr);
   void PropVec3(const char *label, IEditable *undo_obj, bool is_live, float *startup_x, float *startup_y, float *startup_z, float *live_x, float *live_y, float *live_z, const char *format = "%.3f", ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsDecimal, const OnVec3PropChange &chg_callback = nullptr);
   void PropCombo(const char *label, IEditable *undo_obj, bool is_live, int *startup_v, int *live_v, size_t n_values, const string labels[], const OnIntPropChange &chg_callback = nullptr);
   void PropImageCombo(const char *label, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, const OnStringPropChange &chg_callback = nullptr);
   void PropMaterialCombo(const char *label, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, const OnStringPropChange &chg_callback = nullptr);
   void PropLightmapCombo(const char *label, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, const OnStringPropChange &chg_callback = nullptr);
   void PropRenderProbeCombo(const char *label, RenderProbe::ProbeType type, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, const OnStringPropChange &chg_callback = nullptr);

   // Enter/Exit edit mode (manage table backup, dynamic mode,...)
   void ResetCameraFromPlayer();

   // Outliner
   float m_outliner_width = 0.0f;
   string m_outlinerFilter;
   bool m_outlinerSelectLiveTab = true;
   bool IsOutlinerFiltered(const string& name);

   // Properties
   float m_properties_width = 0.0f;
   bool m_propertiesSelectLiveTab = true;

   // Rendering
   float m_menubar_height = 0.0f;
   float m_toolbar_height = 0.0f;
   enum PhysicOverlay { PO_NONE, PO_SELECTED, PO_ALL } m_physOverlay = PO_NONE;
   bool m_selectionOverlay = true;
   enum SelectionFilter { SF_Playfield = 0x0001, SF_Primitives = 0x0002, SF_Lights = 0x0004, SF_Flashers = 0x0008};
   int m_selectionFilter = 0xFFFF;

   // UI state
   bool m_isOpened = false;
   bool m_showPlumbDialog = false;
   bool m_flyMode = false;
   bool m_showRendererInspection = false;
   uint32_t m_OpenUITime = 0; // Used to delay keyboard shortcut
   uint32_t m_StartTime_msec = 0; // Used for timed splash overlays

   // 3D editor
   ImGuizmo::OPERATION m_gizmoOperation = ImGuizmo::NONE;
   ImGuizmo::MODE m_gizmoMode = ImGuizmo::WORLD;
   bool GetSelectionTransform(Matrix3D& transform);
   void SetSelectionTransform(const Matrix3D& transform, bool clearPosition = false, bool clearScale = false, bool clearRotation = false);

   // Editor camera
   bool m_useEditorCam = false;
   bool m_orthoCam = true;
   Matrix3D m_camView, m_camProj;
   float m_camDistance;
};
