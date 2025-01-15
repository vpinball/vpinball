// license:GPLv3+

#pragma once

#include "core/stdafx.h"

#include "core/pininput.h"

#include "imgui/imgui.h"
#include "imguizmo/ImGuizmo.h"
#include "imgui_markdown/imgui_markdown.h"

class LiveUI
{
public:
   LiveUI(RenderDevice* const rd);
   ~LiveUI();

   void Update(const int width, const int height);
   void Render();
   bool HasKeyboardCapture() const;
   bool HasMouseCapture() const;

   bool IsOpened() const { return m_ShowUI || m_ShowSplashModal; }
   void OpenMainSplash();
   void OpenLiveUI();

   void OpenTweakMode();
   bool IsTweakMode() const { return m_tweakMode; }
   void OnTweakModeEvent(const int keyEvent, const int keycode);

   void HideUI();

   void ToggleFPS();
   bool IsShowingFPSDetails() const { return m_show_fps > 1; }
   void PushNotification(const string &message, const U32 lengthMs) { m_notifications.push_back(Notification(message, msec() + lengthMs)); }

private:
   // Main UI frame & panels
   void UpdateMainUI();
   void UpdateOutlinerUI();
   void UpdatePropertyUI();

   // FPS and performance overlays
   void UpdatePerfOverlay();

   // Tweak UI
   enum TweakType { TT_Int, TT_Float, TT_Set };
   struct TweakOption
   {
      TweakType type;
      float min, max, step, def;
      string name, unit;
      vector<string> options;
      TweakOption(TweakType _type, float _min, float _max, float _step, float _def, const string& _name, const string& _unit, std::initializer_list<string> _options): 
         type(_type), min(_min), max(_max), step(_step), def(_def), name(_name), unit(_unit), options(_options) { }
   };
   enum TweakPage { TP_Info, TP_Rules, TP_PointOfView, TP_TableOption, TP_Plugin00 };
   enum BackdropSetting
   {
      BS_Page,
      // Point of View
      BS_ViewMode, BS_LookAt, BS_FOV, BS_Layback, BS_ViewHOfs, BS_ViewVOfs, BS_XYZScale, BS_XScale, BS_YScale, BS_ZScale, BS_XOffset, BS_YOffset, BS_ZOffset, BS_WndTopZOfs, BS_WndBottomZOfs,
      // Table tweaks & Custom table defined options (must be the last of this enum)
      BS_MusicVolume, BS_SoundVolume, BS_DayNight, BS_Difficulty, BS_Tonemapper, BS_Exposure, BS_Custom
   };
   U32 m_lastTweakKeyDown = 0;
   int m_activeTweakIndex = 0;
   int m_activeTweakPageIndex = 0;
   vector<TweakPage> m_tweakPages;
   int m_tweakState[BS_Custom + 100]; // 0 = unmodified, 1 = modified, 2 = resetted
   vector<BackdropSetting> m_tweakPageOptions;
   bool m_tweakMode = false;
   float m_tweakScroll = 0.f;
   void UpdateTweakPage();
   void UpdateTweakModeUI();

   // Touch UI
   void UpdateTouchUI();

   // Popups & Modals
   void UpdateMainSplashModal();
   void UpdateAudioOptionsModal();
   void UpdateVideoOptionsModal();
   void UpdateAnaglyphCalibrationModal();
   void UpdateRendererInspectionModal();
   void UpdateHeadTrackingModal();
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
   void CloseTweakMode();
   void ResetCameraFromPlayer();

   // MarkDown support
   ImGuiID markdown_start_id;
   static ImGui::MarkdownConfig markdown_config;
   static void MarkdownFormatCallback(const ImGui::MarkdownFormatInfo &markdownFormatInfo, bool start);
   static void MarkdownLinkCallback(ImGui::MarkdownLinkCallbackData data);
   static ImGui::MarkdownImageData MarkdownImageCallback(ImGui::MarkdownLinkCallbackData data);

   // UI Context
   VPinball *m_app;
   Player   *m_player;
   PinTable *m_table; // The edited table
   PinTable *m_live_table; // The live copy of the edited table being played by the player (all properties can be changed at any time by the script)
   PinInput *m_pininput;
   Renderer *m_renderer;
   bool m_staticPrepassDisabled = false;
   struct Selection
   {
      enum SelectionType { S_NONE, S_CAMERA, S_MATERIAL, S_IMAGE, S_EDITABLE, S_RENDERPROBE } type = S_NONE;
      bool is_live;
      union
      {
         int camera;
         IEditable* editable;
         Material *material;
         Texture *image;
         RenderProbe *renderprobe;
         int ball_index;
      };
      Selection() {}
      Selection(SelectionType t, bool live, int ball) { type = t; is_live = live; ball_index = ball; }
      Selection(bool live, IEditable *data) { type = S_EDITABLE; is_live = live; editable = data; }
      Selection(bool live, Material *data) { type = S_MATERIAL; is_live = live; material = data; }
      Selection(bool live, Texture *data) { type = S_IMAGE; is_live = live; image = data; }
      Selection(bool live, RenderProbe *data) { type = S_RENDERPROBE; is_live = live; renderprobe = data; }
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

   // Outliner
   float m_outliner_width = 0.0f;
   string m_outlinerFilter;
   bool m_outlinerSelectLiveTab = true;
   bool IsOutlinerFiltered(const string& name);

   // Properties
   float m_properties_width = 0.0f;
   bool m_propertiesSelectLiveTab = true;

   // Notifications
   struct Notification
   {
      string message;
      U32 disappearTick;
      Notification() {}
      Notification(const string& msg, const U32 tick)  : message(msg), disappearTick(tick) {}
   };
   vector<Notification> m_notifications;

   // Rendering
   RenderDevice* const m_rd;
   bool m_rotation_callback_added = false;
   int m_rotate = 0;
   float m_dpi = 1.0f;
   ImFont *m_baseFont = nullptr;
   ImFont *m_overlayBoldFont = nullptr;
   ImFont *m_overlayFont = nullptr;
   float m_menubar_height = 0.0f;
   float m_toolbar_height = 0.0f;
   enum PhysicOverlay { PO_NONE, PO_SELECTED, PO_ALL } m_physOverlay = PO_NONE;
   bool m_selectionOverlay = true;
   enum SelectionFilter { SF_Playfield = 0x0001, SF_Primitives = 0x0002, SF_Lights = 0x0004, SF_Flashers = 0x0008};
   int m_selectionFilter = 0xFFFF;

   // UI state
   bool m_ShowUI = false;
   bool m_ShowSplashModal = false;
   bool m_ShowBAMModal = false;
   bool m_ShowPlumb = false;
   bool m_flyMode = false;
   bool m_RendererInspection = false;
   bool m_disable_esc = false; // Option for keyboard shortcuts
   int m_esc_mode = 0; // What to do if Esc is pressed while on splash screen (depends on why the splash was opened)
   U32 m_OpenUITime = 0; // Used to delay keyboard shortcut
   U64 m_StartTime_msec = 0; // Used for timed splash overlays
   
   // Profiler display data
   int m_show_fps = 0; // 0=disabled / 1=FPS / 2=FPS+dynamic plot
   bool m_showAvgFPS = true;
   bool m_showRollingFPS = true;
   class PlotData
   {
   public:
      PlotData() : m_maxSize(500)
      {
         m_data.reserve(m_maxSize);
         SetRolling(!m_rolling);
      }
      void SetRolling(bool rolling)
      {
         if (rolling == m_rolling)
            return;
         if (!m_data.empty())
            m_data.shrink(0);
         m_offset = 0;
         m_rolling = rolling;
      }
      void AddPoint(const float x, const float y)
      {
         if (!m_data.empty() && m_data.back().x == x)
            return;
         if (std::isinf(y))
            return;
         if (m_rolling)
         {
            const float xmod = fmodf(x, m_timeSpan);
            if (!m_data.empty() && xmod < m_data.back().x)
               m_data.shrink(0);
            m_data.push_back(ImVec2(xmod, y));
         }
         else
         {
            if (m_data.size() < m_maxSize)
               m_data.push_back(ImVec2(x, y));
            else
            {
               m_data[m_offset] = ImVec2(x, y);
               m_offset++;
               if (m_offset == m_maxSize)
                  m_offset = 0;
            }
         }
      }
      bool HasData() const { return !m_data.empty(); }
      ImVec2 GetLast() const
      {
         if (m_data.empty())
            return ImVec2 { 0.f, 0.f };
         else if (m_data.size() < m_maxSize || m_offset == 0)
            return m_data.back();
         else
            return m_data[m_offset - 1];
      }

   public:
      int m_offset = 0;
      float m_timeSpan = 2.5f;
      ImVector<ImVec2> m_data;
      bool m_rolling = true;

   private:
      const int m_maxSize;
   };
   PlotData m_plotFPS, m_plotFPSSmoothed, m_plotPhysx, m_plotPhysxSmoothed, m_plotScript, m_plotScriptSmoothed;

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
