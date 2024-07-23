#pragma once

#include "stdafx.h"

#include "imgui/imgui.h"
#include "imgui/ImGuizmo.h"
#include "imgui_markdown/imgui_markdown.h"

class LiveUI
{
public:
   LiveUI(RenderDevice* const rd);
   ~LiveUI();
   void Update(const RenderTarget* rt);
   void Render();
   void Render(int LR);
   bool HasKeyboardCapture() const;
   bool HasMouseCapture() const;

   bool IsOpened() const { return m_ShowUI || m_ShowSplashModal; }
   void OpenMainSplash();
   void OpenLiveUI();

   void OpenTweakMode();
   bool IsTweakMode() const { return m_tweakMode; }
   void OnTweakModeEvent(const int keyEvent, const int keycode);

   void HideUI();

   void PausePlayer(bool pause);

   void ToggleFPS();
   bool IsShowingFPSDetails() const { return m_show_fps > 1; }
   void PushNotification(const string &message, const U32 lengthMs) { m_notifications.push_back(Notification(message, msec() + lengthMs)); }

private:
   // Interactive Camera Mode
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
   enum TweakPage { TP_Info, TP_Rules, TP_PointOfView, TP_TableOption, TP_VRTweakUI, TP_Count };
   enum BackdropSetting
   {
      BS_Page,
      // Point of View
      BS_ViewMode, BS_LookAt, BS_FOV, BS_Layback, BS_ViewHOfs, BS_ViewVOfs, BS_XYZScale, BS_XScale, BS_YScale, BS_ZScale, BS_XOffset, BS_YOffset, BS_ZOffset, BS_WndTopZOfs, BS_WndBottomZOfs,
      // Table tweaks & Custom table defined options (must be the last of this enum)
      BS_MusicVolume, BS_SoundVolume, BS_DayNight, BS_Difficulty, BS_Tonemapper, BS_TweakUI_X, BS_TweakUI_Y, BS_Custom
   };
   TweakPage m_activeTweakPage = TP_PointOfView;
   U32 m_lastTweakKeyDown = 0;
   int m_activeTweakIndex = 0;
   int m_tweakState[BS_Custom + 100]; // 0 = unmodified, 1 = modified, 2 = reseted
   vector<BackdropSetting> m_tweakPageOptions;
   bool m_tweakMode = false;
   float m_tweakScroll = 0.f;
   void UpdateTweakPage();
   void UpdateTweakModeUI();

   // Touch UI
   void UpdateTouchUI();

   // Main UI frame & panels
   void UpdateMainUI();
   void UpdateOutlinerUI();
   void UpdatePropertyUI();

   // Popups & Modals
   void UpdateMainSplashModal();
   void UpdateAudioOptionsModal();
   void UpdateVideoOptionsModal();
   void UpdateAnaglyphCalibrationModal();
   void UpdateRendererInspectionModal();
   void UpdateHeadTrackingModal();

   // UI Selection & properties
   void RenderProbeProperties(bool is_live);
   void BallProperties(bool is_live);
   void CameraProperties(bool is_live);
   void MaterialProperties(bool is_live);
   void TableProperties(bool is_live);
   void FlasherProperties(bool is_live, Flasher *startup_obj, Flasher *live_obj);
   void LightProperties(bool is_live, Light *startup_obj, Light *live_obj);
   void PrimitiveProperties(bool is_live, Primitive *startup_obj, Primitive *live_obj);
   void RampProperties(bool is_live, Ramp *startup_obj, Ramp *live_obj);
   void RubberProperties(bool is_live, Rubber *startup_obj, Rubber *live_obj);
   void SurfaceProperties(bool is_live, Surface *startup_obj, Surface *live_obj);

   // Helpers for property edition
   typedef std::function<void(bool is_live, float prev, float v)> OnFloatPropChange;
   typedef std::function<void(bool is_live, int prev, int v)> OnIntPropChange;
   typedef std::function<void(bool is_live, bool prev, bool v)> OnBoolPropChange;
   typedef std::function<void(bool is_live, string prev, string v)> OnStringPropChange;
   typedef std::function<void(bool is_live, vec3& prev, vec3& v)> OnVec3PropChange;
   void PropSeparator(const char *label = nullptr);
   void PropCheckbox(const char *label, IEditable *undo_obj, bool is_live, bool *startup_v, bool *live_v, OnBoolPropChange chg_callback = nullptr);
   void PropFloat(const char *label, IEditable* undo_obj, bool is_live, float *startup_v, float *live_v, float step, float step_fast, const char *format = "%.3f", ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsDecimal, OnFloatPropChange chg_callback = nullptr);
   void PropInt(const char *label, IEditable *undo_obj, bool is_live, int *startup_v, int *live_v);
   void PropRGB(const char *label, IEditable *undo_obj, bool is_live, COLORREF *startup_v, COLORREF *live_v, ImGuiColorEditFlags flags = 0);
   void PropVec3(const char *label, IEditable* undo_obj, bool is_live, Vertex3Ds *startup_v, Vertex3Ds *live_v, const char *format = "%.3f", ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsDecimal, OnVec3PropChange chg_callback = nullptr);
   void PropVec3(const char *label, IEditable *undo_obj, bool is_live, float *startup_v, float *live_v, const char *format = "%.3f", ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsDecimal, OnVec3PropChange chg_callback = nullptr);
   void PropVec3(const char *label, IEditable *undo_obj, bool is_live, float *startup_x, float *startup_y, float *startup_z, float *live_x, float *live_y, float *live_z, const char *format = "%.3f", ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsDecimal, OnVec3PropChange chg_callback = nullptr);
   void PropCombo(const char *label, IEditable *undo_obj, bool is_live, int *startup_v, int *live_v, int n_values, const string labels[], OnIntPropChange chg_callback = nullptr);
   void PropImageCombo(const char *label, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, OnStringPropChange chg_callback = nullptr);
   void PropMaterialCombo(const char *label, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, OnStringPropChange chg_callback = nullptr);

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
   Player *m_player;
   PinTable *m_table; // The edited table
   PinTable *m_live_table; // The live copy of the edited table being played by the player (all properties can be changed at any time by the script)
   PinInput *m_pininput;
   Pin3D *m_pin3d;
   struct Selection
   {
      enum SelectionType { S_NONE, S_CAMERA, S_MATERIAL, S_BALL, S_EDITABLE, S_RENDERPROBE } type = S_NONE;
      bool is_live;
      union
      {
         int camera;
         IEditable* editable;
         Material *material;
         RenderProbe *renderprobe;
         int ball_index;
      };
      Selection() {}
      Selection(SelectionType t, bool live, int ball) { type = t; is_live = live; ball_index = ball; }
      Selection(bool live, IEditable *data) { type = S_EDITABLE; is_live = live; editable = data; }
      Selection(bool live, Material *data) { type = S_MATERIAL; is_live = live; material = data; }
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
         case S_BALL: return ball_index == s.ball_index;
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

   // UI state
   bool m_ShowUI = false;
   bool m_ShowSplashModal = false;
   bool m_flyMode = false;
   bool m_RendererInspection = false;
   bool m_disable_esc = false; // Option for keyboard shortcuts
   U32 m_OpenUITime = 0; // Used to delay keyboard shortcut
   U64 m_StartTime_msec = 0; // Used for timed splash overlays
   int m_show_fps = 0; // 0=disabled / 1=FPS / 2=FPS+dynamic plot

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
