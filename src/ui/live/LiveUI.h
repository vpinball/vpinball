// license:GPLv3+

#pragma once

#include "input/pininput.h"

#include "imgui/imgui.h"
#include "imgui_markdown/imgui_markdown.h"

#include "PerfUI.h"
#include "InGameUI.h"
#include "EditorUI.h"
#include "EscSplashModal.h"
#include "NotificationOverlay.h"
#include "BallControl.h"

inline constexpr const char* ID_BAM_SETTINGS = "Headtracking Settings";

class LiveUI final
{
public:
   LiveUI(RenderDevice* const rd);
   ~LiveUI();

   void Update();

   bool HasKeyboardCapture() const;
   bool HasMouseCapture() const;
   bool IsOpened() const { return m_editorUI.IsOpened() || m_escSplashModal.IsOpened() || m_inGameUI.IsOpened(); }
   void OpenMainSplash() { m_escSplashModal.Open(); }
   void OpenEditorUI() { m_editorUI.Open(); }
   void HideUI();

   void OpenTweakMode();
   bool IsTweakMode() const { return m_inGameUI.IsOpened(); }

   void ToggleFPS() { m_perfUI.NextPerfMode(); }
   bool IsShowingFPSDetails() const { return m_perfUI.GetPerfMode() != PerfUI::PerfMode::PM_DISABLED; }
   
   unsigned int PushNotification(const string &message, const int lengthMs, const unsigned int reuseId = 0) { return m_notificationOverlay.PushNotification(message, lengthMs, reuseId); }

   // Ball Control
   BallControl m_ballControl;

   // In Game UI
   InGameUI m_inGameUI;

   // Profiler display data
   PerfUI m_perfUI;

   // UI context (to be moved outside of LiveUI API)
   void SetMarkdownStartId(const unsigned int startId) { markdown_start_id = startId; }
   const ImGui::MarkdownConfig &GetMarkdownConfig() const { return markdown_config; }
   ImFont *GetOverlayFont() const { return m_overlayFont; }
   float GetDPI() const { return m_dpi; }
   int GetUIOrientation() const { return m_rotate; }
   static ImGuiKey GetImGuiKeysFromDIkeycode(const int dik);
   static void CenteredText(const string &text);

private:
   void NewFrame();

   vector<std::unique_ptr<MeshBuffer>> m_meshBuffers;

   // Splash modal when user press exit key
   EscSplashModal m_escSplashModal;

   // Editor UI
   EditorUI m_editorUI;

   // Touch UI overlay
   void UpdateTouchUI();

   // Notifications
   NotificationOverlay m_notificationOverlay;

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

   // Rendering
   RenderDevice* const m_rd;
   int m_rotate = 0;
   float m_dpi = 1.0f;
   ImFont *m_baseFont = nullptr;
   ImFont *m_overlayBoldFont = nullptr;
   ImFont *m_overlayFont = nullptr;
};
