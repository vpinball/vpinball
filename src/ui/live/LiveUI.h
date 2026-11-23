// license:GPLv3+

#pragma once

#include "core/def.h"
#include "input/InputManager.h"
#include "renderer/Renderer.h"

#include "imgui/imgui.h"
#include "imgui_markdown/imgui_markdown.h"

#include "PerfUI.h"
#include "ingameui/InGameUI.h"
#include "EditorUI.h"
#include "NotificationOverlay.h"
#include "PlumbOverlay.h"
#include "BallControl.h"
#include "DebuggerUI.h"

struct UIListGeometry;
class Sampler;

class LiveUI final : public UserMessageSink
{
public:
   LiveUI(RenderDevice* const rd);
   ~LiveUI();

   // UserMessageSink interface: non fatal messages are routed to the in-game notification
   // overlay, while fatal errors and confirmations use a (blocking) SDL message box.
   void Notify(MsgSeverity severity, const string &title, const string &message) override;
   bool Confirm(const string &title, const string &message, bool fallback) override;

   void PrepareScene(); // Called to set the scene display state of the view being recorded (editor panes)
   void Render3D(); // Called to contribute to 3D Scene
   void RenderUI(); // Called to render UI overlay

   bool IsOpened() const { return IsEditorUIOpened() || IsInGameUIOpened(); }
   void HideUI();

   void OpenEditorUI() { m_editorUI.Open(); }
   bool IsEditorUIOpened() const { return m_editorUI.IsOpened(); }
   bool IsEditorViewMode() const { return m_editorUI.IsOpened() && !m_editorUI.IsPreview(); }
   bool IsEditorBackdropViewMode() const { return m_editorUI.IsOpened() && m_editorUI.IsBackdropEditMode(); }

   void OpenInGameUI(const string& page = "homepage"s);
   bool IsInGameUIOpened() const { return m_inGameUI.IsOpened(); }
   bool IsGameplayInputSuspended() const { return m_inGameUI.IsGameplayInputSuspended(); }

   void ToggleFPS() { m_perfUI.NextPerfMode(); }
   bool IsShowingFPSDetails() const { return m_perfUI.GetPerfMode() != PerfUI::PerfMode::PM_DISABLED; }
   
   void ShowTouchOverlay(bool show) { m_showTouchOverlay = show; }
   // Show the touch overlay for a while, then fade it out, even if not enabled (presents the touch controls on first launches)
   void ShowTouchOverlayHint(uint64_t durationMs) { m_touchOverlayHintEndMs = SDL_GetTicks() + durationMs; }

   unsigned int PushNotification(const string &message, const int lengthMs, const unsigned int reuseId = 0) { return m_notificationOverlay.PushNotification(message, lengthMs, reuseId); }

   // Ball Control
   BallControl m_ballControl;

   // Debugger (play/pause/step, ball control, script console)
   DebuggerUI m_debuggerUI;
   bool IsDebuggerOpened() const { return m_debuggerUI.IsOpened(); }

   // Editor UI
   VPX::EditorUI::EditorUI m_editorUI;

   // In Game UI
   VPX::InGameUI::InGameUI m_inGameUI;

   // Profiler display data
   PerfUI m_perfUI;

   // UI context (to be moved outside of LiveUI API)
   void SetMarkdownStartId(const unsigned int startId) { markdown_start_id = startId; }
   const ImGui::MarkdownConfig &GetMarkdownConfig() const { return markdown_config; }
   ImFont *GetOverlayFont() const { return m_overlayFont; }
   ImFont *GetMonoFont() const { return m_monoFont; }
   float GetDPI() const { return m_uiScale; }
   int GetUIOrientation() const { return m_rotate; }
   float GetPinchScale() const { return m_pinchScale; }
   float GetTwistDegrees() const { return m_twistDegrees; }
   bool IsTrackpadScroll() const { return m_trackpadScroll; }
   bool IsTrackpadScrollInverted() const { return m_trackpadScrollInverted; }
   static ImGuiKey GetImGuiKeyFromSDLScancode(const SDL_Scancode sdlk);
   static void CenteredText(const string &text);

   void HandleSDLEvent(SDL_Event &e);

private:
   void SetupImGuiStyle(const bool isEditor) const;
   void ReleaseImGuiTexture(ImTextureData *tex);

   void NewFrame();
   void AddMousePosEvent(bool isTouch, float x, float y) const;
   void UpdateScale();
   void UpdatePlayfieldWindowFrame();
   bool m_playfieldWindowFramed = false;

   vector<std::shared_ptr<MeshBuffer>> m_meshBuffers;
#if defined(ENABLE_BGFX)
   vector<std::shared_ptr<UIListGeometry>> m_uiGeometry; // Per command list CPU snapshot of the frame's UI geometry
#endif

   // Touch UI overlay
   void UpdateTouchUI();
   bool m_showTouchOverlay;
   uint64_t m_touchOverlayHintEndMs = 0; // SDL_GetTicks end time of ShowTouchOverlayHint

   // VR: whether the last rendered frame had UI content (mirrors the state pushed
   // to VRDevice::SetShowUILayer, gating the controller pointer and its ray)
   bool m_uiLayerShown = false;
   // VR controller pointer: last trigger state fed to ImGui as the left mouse
   // button, and the 1D gradient texture of the pointer ray ribbon
   bool m_vrPointerClick = false;
   std::shared_ptr<Sampler> m_laserSampler;
   std::shared_ptr<Sampler> m_laserDotSampler;

   // Emulated plumb overlay
   PlumbOverlay m_plumbOverlay;

   // Notifications
   NotificationOverlay m_notificationOverlay;

   // MarkDown support
   ImGuiID markdown_start_id;
   static ImGui::MarkdownConfig markdown_config;
   static void MarkdownFormatCallback(const ImGui::MarkdownFormatInfo &markdownFormatInfo, bool start);
   static void MarkdownLinkCallback(ImGui::MarkdownLinkCallbackData data);
   static ImGui::MarkdownImageData MarkdownImageCallback(ImGui::MarkdownLinkCallbackData data);

   // UI Context
   Player   *m_player;
   InputManager *m_pininput;
   std::unique_ptr<Renderer>& m_renderer;

   // Rendering
   RenderDevice* const m_rd;
   int m_rotate = 0;
   void HandleTrackpadFingers(const SDL_TouchFingerEvent &e);
   float m_pendingPinchScale = 1.f;
   float m_pinchScale = 1.f;
   SDL_TouchID m_trackpadTouchID = 0;
   bool m_twistActive = false;
   bool m_twistStarted = false;
   float m_twistAngle = 0.f;
   float m_twistAccumulated = 0.f;
   float m_pendingTwistDegrees = 0.f;
   float m_twistDegrees = 0.f;
   Uint64 m_lastWheelTimestamp = 0;
   bool m_pendingTrackpadScroll = false;
   bool m_pendingTrackpadScrollInverted = false;
   bool m_trackpadScroll = false;
   bool m_trackpadScrollInverted = false;
   float m_uiScale = 0.f;
   ImFont *m_baseFont = nullptr;
   ImFont *m_overlayBoldFont = nullptr;
   ImFont *m_overlayFont = nullptr;
   ImFont *m_monoFont = nullptr;
};

namespace plog
{
Record &operator<<(Record &record, const ImVec2 &pt);
}
