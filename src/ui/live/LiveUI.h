// license:GPLv3+

#pragma once

#include "input/InputManager.h"

#include "imgui/imgui.h"
#include "imgui_markdown/imgui_markdown.h"

#include "PerfUI.h"
#include "ingameui/InGameUI.h"
#include "EditorUI.h"
#include "NotificationOverlay.h"
#include "PlumbOverlay.h"
#include "BallControl.h"

constexpr const char * const ID_BAM_SETTINGS = "Headtracking Settings";

class LiveUI final
{
public:
   LiveUI(RenderDevice* const rd);
   ~LiveUI();

   void Render3D(); // Called to contribute to 3D Scene
   void RenderUI(); // Called to render UI overlay

   bool IsOpened() const { return IsEditorUIOpened() || IsInGameUIOpened(); }
   void HideUI();

   void OpenEditorUI() { m_editorUI.Open(); }
   bool IsEditorUIOpened() const { return m_editorUI.IsOpened(); }
   bool IsEditorViewMode() const { return m_editorUI.IsOpened() && !m_editorUI.IsPreview(); }

   void OpenInGameUI(const string& page = "homepage"s);
   void OpenMainSplash() { OpenInGameUI("exit"); }
   bool IsInGameUIOpened() const { return m_inGameUI.IsOpened(); }

   void ToggleFPS() { m_perfUI.NextPerfMode(); }
   bool IsShowingFPSDetails() const { return m_perfUI.GetPerfMode() != PerfUI::PerfMode::PM_DISABLED; }
   
   bool ProposeInputLayout(const string &deviceName, const std::function<void(bool, bool)> &handler);

   void ShowTouchOverlay(bool show) { m_showTouchOverlay = show; }

   unsigned int PushNotification(const string &message, const int lengthMs, const unsigned int reuseId = 0) { return m_notificationOverlay.PushNotification(message, lengthMs, reuseId); }

   // Ball Control
   BallControl m_ballControl;

   // In Game UI
   VPX::InGameUI::InGameUI m_inGameUI;

   // Profiler display data
   PerfUI m_perfUI;

   // UI context (to be moved outside of LiveUI API)
   void SetMarkdownStartId(const unsigned int startId) { markdown_start_id = startId; }
   const ImGui::MarkdownConfig &GetMarkdownConfig() const { return markdown_config; }
   ImFont *GetOverlayFont() const { return m_overlayFont; }
   float GetDPI() const { return m_uiScale; }
   int GetUIOrientation() const { return m_rotate; }
   static ImGuiKey GetImGuiKeyFromSDLScancode(const SDL_Scancode sdlk);
   static void CenteredText(const string &text);

   void HandleSDLEvent(SDL_Event &e) const;

private:
   void SetupImGuiStyle(const bool isEditor) const;
   
   void NewFrame();
   void AddMousePosEvent(bool isTouch, float x, float y) const;
   void UpdateScale();

   vector<std::shared_ptr<MeshBuffer>> m_meshBuffers;

   // Editor UI
   VPX::EditorUI::EditorUI m_editorUI;

   // Touch UI overlay
   void UpdateTouchUI();
   bool m_showTouchOverlay;

   // Emulated plumb overlay
   PlumbOverlay m_plumbOverlay;

   // Notifications
   NotificationOverlay m_notificationOverlay;

   // Autodetected Input Device popup
   string m_deviceLayoutName;
   bool m_deviceLayoutDontAskAgain;
   std::function<void(bool, bool)> m_deviceLayoutHandler;
   void UpdateDeviceLayoutPopup();

   // MarkDown support
   ImGuiID markdown_start_id;
   static ImGui::MarkdownConfig markdown_config;
   static void MarkdownFormatCallback(const ImGui::MarkdownFormatInfo &markdownFormatInfo, bool start);
   static void MarkdownLinkCallback(ImGui::MarkdownLinkCallbackData data);
   static ImGui::MarkdownImageData MarkdownImageCallback(ImGui::MarkdownLinkCallbackData data);

   // UI Context
   VPinball *m_app;
   Player   *m_player;
   InputManager *m_pininput;
   Renderer *m_renderer;

   // Rendering
   RenderDevice* const m_rd;
   int m_rotate = 0;
   float m_uiScale = 0.f;
   ImFont *m_baseFont = nullptr;
   ImFont *m_overlayBoldFont = nullptr;
   ImFont *m_overlayFont = nullptr;
};

namespace plog
{
Record &operator<<(Record &record, const ImVec2 &pt);
}
