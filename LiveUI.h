#pragma once

#include "stdafx.h"

#include "imgui/imgui.h"

class LiveUI
{
public:
   LiveUI(RenderDevice* const rd);
   ~LiveUI();
   void Update();
   void Render();
   bool HasKeyboardCapture() const;
   bool HasMouseCapture() const;

   void OpenMainUI();
   void ToggleFPS();

private:
   // Interactive Camera Mode
   void UpdateCameraModeUI();

   // Main UI frame & panels
   void UpdateMainUI();
   void UpdateOutlinerUI();
   void UpdatePropertyUI();

   // Pop ups & Modals
   void UpdateMainSplashModal();
   void UpdateAudioOptionsModal();
   void UpdateVideoOptionsModal();
   void UpdateRendererInspectionModal();
   void UpdateHeadTrackingModal();

   // UI Selection & properties
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
   void PropSeparator(const char *label = nullptr);
   void PropCheckbox(const char *label, IEditable *undo_obj, bool is_live, bool *startup_v, bool *live_v, OnBoolPropChange chg_callback = nullptr);
   void PropFloat(const char *label, IEditable* undo_obj, bool is_live, float *startup_v, float *live_v, float step, float step_fast, const char *format = "%.3f", ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsDecimal, OnFloatPropChange chg_callback = nullptr);
   void PropInt(const char *label, IEditable *undo_obj, bool is_live, int *startup_v, int *live_v);
   void PropRGB(const char *label, IEditable *undo_obj, bool is_live, COLORREF *startup_v, COLORREF *live_v, ImGuiColorEditFlags flags = 0);
   void PropVec3(const char *label, IEditable* undo_obj, bool is_live, Vertex3Ds *startup_v, Vertex3Ds *live_v, const char *format = "%.3f", ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsDecimal);
   void PropCombo(const char *label, IEditable *undo_obj, bool is_live, int *startup_v, int *live_v, int n_values, const string labels[]);
   void PropImageCombo(const char *label, IEditable *undo_obj, bool is_live, string *startup_v, string *live_v, PinTable *table, OnStringPropChange chg_callback = nullptr);

   // Enter/Exit edit mode (manage table backup, dynamic mode,...)
   void HideUI();
   void PausePlayer(bool pause);
   void EnterEditMode();
   void ExitEditMode();

   // UI Context
   VPinball *m_app;
   Player *m_player;
   PinTable *m_table; // The edited table
   PinTable *m_live_table; // The live copy of the edited table being played by the player (all properties can be changed at any time by the script)
   PinInput *m_pininput;
   Pin3D *m_pin3d;
   struct Selection
   {
      enum SelectionType { S_NONE, S_CAMERA, S_MATERIAL, S_BALL, S_EDITABLE } type = S_NONE;
      bool is_live;
      union
      {
         int camera;
         IEditable* editable;
         Material *material;
         int ball_index;
      };
   } m_selection;

   // Rendering
   const RenderDevice* m_rd;
   int m_rotate = 0;
   float m_dpi = 1.0f;
   ImFont *m_baseFont = nullptr;
   ImFont *m_overlayFont = nullptr;
   float m_menubar_height = 0.0f;
   float m_toolbar_height = 0.0f;
   bool m_old_player_dynamic_mode;
   bool m_old_player_camera_mode;

   // UI state
   bool m_ShowUI = false;
   bool m_ShowSplashModal = false;
   bool m_disable_esc = false; // Option for keyboard shortcuts
   U32 m_OpenUITime = 0; // Used to delay keyboard shortcut
   U64 m_StartTime_usec = 0; // Used for timed splash overlays
   int m_show_fps = 0; // 0=disabled / 1=FPS / 2=FPS+dynamic plot

   // Editor camera
   bool m_useEditorCam = false;
   vec3 m_CamEye, m_CamAt, m_CamUp;
};
