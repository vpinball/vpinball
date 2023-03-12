#pragma once

#include "stdafx.h"

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

   // Modals
   void UpdateMainSplashModal();
   void UpdateAudioOptionsModal();
   void UpdateVideoOptionsModal();
   void UpdateRendererInspectionModal();

   // UI Selection & properties
   void CameraProperties();
   void MaterialProperties();
   void FlasherProperties();
   void LightProperties();
   void PrimitiveProperties();
   void RampProperties();
   void RubberProperties();
   void SurfaceProperties();
   void TableProperties();

   // Enter/Exit edit mode (manage table backup, dynamic mode,...)
   void HideUI();
   void PausePlayer(bool pause);
   void EnterEditMode();
   void ExitEditMode();

   // UI Context
   VPinball *m_app;
   Player *m_player;
   PinTable *m_table; // The edited table
   PinTable *m_live_table; // The live copy of the edited being played by the player
   PinInput *m_pininput;
   Pin3D *m_pin3d;
   struct Selection
   {
      enum SelectionType { S_NONE, S_CAMERA, S_MATERIAL, S_EDITABLE } type = S_NONE;
      union
      {
         int camera;
         IEditable* editable;
         Material *material;
      };
   } m_selection;

   // Rendering
   const RenderDevice* m_rd;
   int m_rotate = 0;
   float m_dpi = 1.0f;
   float m_menubar_height = 0.0f;
   float m_toolbar_height = 0.0f;
   bool m_old_player_dynamic_mode;
   bool m_old_player_camera_mode;

   // UI state
   bool m_ShowUI = false;
   bool m_ShowSplashModal = false;
   bool m_ShowBAMModal = false;
   bool m_disable_esc = false; // Option for keyboard shortcuts
   U32 m_OpenUITime = 0; // Used to delay keyboard shortcut
   U64 m_StartTime_usec = 0; // Used for timed splash overlays
   int m_show_fps = 0; // 0=disabled / 1=FPS / 2=FPS+dynamic plot
};
