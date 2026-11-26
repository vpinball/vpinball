// license:GPLv3+

#include "core/stdafx.h"
#include "LiveUI.h"

#include "core/VPApp.h"
#include "core/VPXPluginAPIImpl.h"
#include "IconsLucide.h"
#include "LucideFont.h"
#include "fonts/DroidSans.h"
#include "fonts/DroidSansBold.h"
#include "fonts/JetBrainsMono.h"
#include "math/matrix.h"
#include "math/ModelViewProj.h"
#include "plugins/VPXPlugin.h"
#include "renderer/Renderer.h"
#include "renderer/RenderCommand.h"
#include "renderer/Sampler.h"
#include "renderer/Texture.h"
#include "renderer/VRDevice.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_stdlib.h"
#include "imgui/imgui_internal.h" // ImGuiContext complete type (cross context IO access in NewOverlayFrame)
#include "imgui_markdown/imgui_markdown.h"


ImGui::MarkdownConfig LiveUI::markdown_config;

// ImGui draw lists are uploaded to the GPU without any conversion (VF_IMGUI vertex format)
static_assert(sizeof(ImDrawVert) == sizeof(VertexUI) && offsetof(ImDrawVert, pos) == offsetof(VertexUI, x) && offsetof(ImDrawVert, uv) == offsetof(VertexUI, tu)
   && offsetof(ImDrawVert, col) == offsetof(VertexUI, color));

// Implementation of supporting function for logging of all math objects
namespace plog
{
Record &operator<<(Record &record, const ImVec2 &pt) { return record << '(' << pt.x << ", " << pt.y << ')'; }
}

// Scoped switch of the current ImGui context: outside of these scopes, the main
// context is always the current one (input capture queries outside frame builds land on it)
namespace
{
struct ImGuiContextScope
{
   explicit ImGuiContextScope(ImGuiContext *ctx)
      : m_prev(ImGui::GetCurrentContext())
   {
      ImGui::SetCurrentContext(ctx);
   }
   ~ImGuiContextScope() { ImGui::SetCurrentContext(m_prev); }
   ImGuiContext *const m_prev;
};
}

LiveUI::LiveUI(RenderDevice *const rd)
   : m_ballControl(*this)
   , m_debuggerUI(*this)
   , m_inGameUI(*this)
   , m_perfUI(g_pplayer)
   , m_editorUI(*this)
   , m_rd(rd)
   , m_player(g_pplayer)
   , m_renderer(m_player->m_renderer)
{
   m_pininput = &(m_player->m_pininput);
   
   IMGUI_CHECKVERSION();
   m_mainContext = ImGui::CreateContext();
   ImGuiIO &io = ImGui::GetIO();
   io.IniFilename = nullptr; //don't use an ini file for configuration

   // using the specialized initializer is not needed
   // ImGui_ImplSDL3_InitForOpenGL(m_player->m_playfieldSdlWnd, rd->m_sdl_context);
   ImGui_ImplSDL3_InitForOther(m_player->m_playfieldWnd->GetCore());
   //int displayIndex = SDL_GetDisplayForWindow(m_player->m_playfieldWnd->GetCore());

   SetupImGuiStyle(false);

   UpdateScale();

   // UI fonts
   m_baseFont = io.Fonts->AddFontFromMemoryCompressedTTF(droidsans_compressed_data, droidsans_compressed_size, 13.0f * m_uiScale);
   // Lucide line icons for the live editor (24 px grid glyphs, drawn a bit larger than the text and centered on it)
   ImFontConfig lucide_config;
   lucide_config.MergeMode = true;
   lucide_config.PixelSnapH = true;
   lucide_config.FontDataOwnedByAtlas = false;
   lucide_config.GlyphMinAdvanceX = 15.0f * m_uiScale;
   lucide_config.GlyphOffset = ImVec2(0.f, 1.5f * m_uiScale);
   static constexpr ImWchar lucide_ranges[] = { ICON_MIN_LC, ICON_MAX_LC, 0 };
   io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char *>(lucide_subset_ttf), static_cast<int>(sizeof(lucide_subset_ttf)), 15.0f * m_uiScale, &lucide_config, lucide_ranges);
   // Monospace font for the script editor; merge the icons into it as well since the
   // editor's own popup UI (context menu, ...) renders under it
   const float monoSize = static_cast<float>(g_settingsService.GetAppSettings().GetEditor_ScriptEditorFontSize()) * m_uiScale;
   m_monoFont = io.Fonts->AddFontFromMemoryCompressedTTF(jetbrainsmono_compressed_data, jetbrainsmono_compressed_size, monoSize);
   ImFontConfig lucide_mono_config;
   lucide_mono_config.MergeMode = true;
   lucide_mono_config.PixelSnapH = true;
   lucide_mono_config.FontDataOwnedByAtlas = false;
   lucide_mono_config.GlyphMinAdvanceX = monoSize * 15.f / 13.f;
   lucide_mono_config.GlyphOffset = ImVec2(0.f, monoSize * 1.5f / 13.f);
   io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char *>(lucide_subset_ttf), static_cast<int>(sizeof(lucide_subset_ttf)), monoSize * 15.f / 13.f, &lucide_mono_config, lucide_ranges);

   const float overlaySize = 13.0f * m_uiScale;
   m_overlayFont = io.Fonts->AddFontFromMemoryCompressedTTF(droidsans_compressed_data, droidsans_compressed_size, overlaySize);
   m_overlayBoldFont = io.Fonts->AddFontFromMemoryCompressedTTF(droidsansbold_compressed_data, droidsansbold_compressed_size, overlaySize);
   ImFont *H1 = io.Fonts->AddFontFromMemoryCompressedTTF(droidsansbold_compressed_data, droidsansbold_compressed_size, overlaySize * 20.0f / 13.f);
   ImFont *H2 = io.Fonts->AddFontFromMemoryCompressedTTF(droidsansbold_compressed_data, droidsansbold_compressed_size, overlaySize * 18.0f / 13.f);
   ImFont *H3 = io.Fonts->AddFontFromMemoryCompressedTTF(droidsansbold_compressed_data, droidsansbold_compressed_size, overlaySize * 15.0f / 13.f);
   markdown_config.linkCallback = MarkdownLinkCallback;
   markdown_config.tooltipCallback = nullptr;
   markdown_config.imageCallback = MarkdownImageCallback;
   //markdown_config.linkIcon = ICON_LC_LINK;
   markdown_config.headingFormats[0] = { H1, true };
   markdown_config.headingFormats[1] = { H2, true };
   markdown_config.headingFormats[2] = { H3, false };
   markdown_config.userData = this;
   markdown_config.formatCallback = MarkdownFormatCallback;

   io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

   // The overlay layer lives in its own ImGui context, sharing the main context's font atlas
   // (so the fonts above are valid in both). It has no SDL backend and receives no input at
   // all: its windows are purely non interactive informational displays.
   m_overlayContext = ImGui::CreateContext(io.Fonts);
   ImGui::SetCurrentContext(m_overlayContext);
   ImGuiIO &overlayIO = ImGui::GetIO();
   overlayIO.IniFilename = nullptr;
   overlayIO.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
   SetupImGuiStyle(false);
   ImGui::SetCurrentContext(m_mainContext);

   NewFrame();
   NewOverlayFrame();

   m_showTouchOverlay = g_settingsService.GetAppSettings().GetPlayer_TouchOverlay();
}

// Releases the texture backing an ImGui texture, including its pinned texture manager entry (never evicted,
// so it would otherwise keep its GPU texture and budget share for the whole session)
void LiveUI::ReleaseImGuiTexture(ImTextureData *tex)
{
   auto texture = static_cast<std::shared_ptr<BaseTexture> *>(tex->BackendUserData);
   if (texture == nullptr)
      return;
   if (*texture && m_renderer && m_renderer->m_renderDevice)
      m_renderer->m_renderDevice->m_texMan.UnloadTexture(texture->get());
   delete texture;
   tex->BackendUserData = nullptr;
}

LiveUI::~LiveUI()
{
   HideUI();
   if (m_mainContext != nullptr)
   {
      // Both contexts are mid frame (each render restarts its frame for the next one)
      ImGui::SetCurrentContext(m_overlayContext);
      ImGui::EndFrame();
      ImGui::SetCurrentContext(m_mainContext);
      ImGui::EndFrame();

      ImGuiIO &io = ImGui::GetIO();
      io.BackendFlags &= ~ImGuiBackendFlags_RendererHasTextures;
      for (auto tex : ImGui::GetPlatformIO().Textures)
      {
         ReleaseImGuiTexture(tex);
         tex->SetTexID(ImTextureID_Invalid);
         tex->SetStatus(ImTextureStatus_Destroyed);
      }

      ImGui_ImplSDL3_Shutdown();

      // The overlay context shares the main context's font atlas: it must be destroyed first
      ImGui::DestroyContext(m_overlayContext);
      ImGui::DestroyContext(m_mainContext);
      m_overlayContext = nullptr;
      m_mainContext = nullptr;
   }
}

void LiveUI::Notify(const MsgSeverity severity, const string &title, const string &message)
{
   if (severity == MsgSeverity::Fatal)
   {
      // Fatal errors are reported through a blocking message box as the application is terminating
      SDL_Window *const wnd = (m_player != nullptr && m_player->m_playfieldWnd != nullptr) ? m_player->m_playfieldWnd->GetCore() : nullptr;
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), message.c_str(), wnd);
      return;
   }
   const int durationMs = severity == MsgSeverity::Error ? 10000 : severity == MsgSeverity::Warning ? 8000 : 5000;
   PushNotification(message, durationMs);
}

bool LiveUI::Confirm(const string &title, const string &message, const bool fallback)
{
   SDL_Window *const wnd = (m_player != nullptr && m_player->m_playfieldWnd != nullptr) ? m_player->m_playfieldWnd->GetCore() : nullptr;
   const SDL_MessageBoxButtonData buttons[] = {
      { static_cast<Uint32>(fallback ? SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT : 0), 1, "Yes" },
      { static_cast<Uint32>(SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT | (fallback ? 0 : SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT)), 0, "No" },
   };
   const SDL_MessageBoxData data = { SDL_MESSAGEBOX_WARNING, wnd, title.c_str(), message.c_str(), SDL_arraysize(buttons), buttons, nullptr };
   int buttonId = fallback ? 1 : 0;
   if (!SDL_ShowMessageBox(&data, &buttonId))
      return fallback;
   return buttonId == 1;
}

void LiveUI::MarkdownFormatCallback(const ImGui::MarkdownFormatInfo &markdownFormatInfo, bool start)
{
   const LiveUI *const ui = static_cast<LiveUI*>(markdownFormatInfo.config->userData);
   switch (markdownFormatInfo.type)
   {
   case ImGui::MarkdownFormatType::EMPHASIS:
      ImGui::defaultMarkdownFormatCallback(markdownFormatInfo, start);
      if (markdownFormatInfo.level == 1)
      { // normal emphasis
         if (start)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
         else
            ImGui::PopStyleColor();
      }
      else
      { // strong emphasis
         if (start)
            ImGui::PushFont(ui->m_overlayBoldFont, ui->m_overlayBoldFont->LegacySize);
         else
            ImGui::PopFont();
      }
      break;
   case ImGui::MarkdownFormatType::HEADING:
   {
      ImGui::MarkdownHeadingFormat fmt;
      if (markdownFormatInfo.level > ImGui::MarkdownConfig::NUMHEADINGS)
         fmt = markdownFormatInfo.config->headingFormats[ImGui::MarkdownConfig::NUMHEADINGS - 1];
      else
         fmt = markdownFormatInfo.config->headingFormats[markdownFormatInfo.level - 1];
      if (start)
      {
         if (fmt.font)
            ImGui::PushFont(fmt.font, fmt.font->LegacySize);
         if (ImGui::GetItemID() != ui->markdown_start_id)
            ImGui::NewLine();
      }
      else
      {
         if (fmt.separator)
         {
            ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::Separator();
            ImGui::PopStyleColor();
            ImGui::NewLine();
         }
         else
         {
            ImGui::NewLine();
         }
         if (fmt.font)
            ImGui::PopFont();
      }
      break;
   }
   default: ImGui::defaultMarkdownFormatCallback(markdownFormatInfo, start); break;
   }
}

void LiveUI::MarkdownLinkCallback(ImGui::MarkdownLinkCallbackData data)
{
   if (!data.isImage)
   {
      std::string url(data.link, data.linkLength);
      SDL_OpenURL(url.c_str());
   }
}

ImGui::MarkdownImageData LiveUI::MarkdownImageCallback(ImGui::MarkdownLinkCallbackData data)
{
   LiveUI *const ui = (LiveUI *)data.userData;
   Texture *const ppi = ui->m_player->m_ptable->GetImage(std::string(data.link, data.linkLength));
   if (ppi == nullptr)
      return ImGui::MarkdownImageData {};
   std::shared_ptr<Sampler> sampler = ui->m_renderer->m_renderDevice->m_texMan.LoadTexture(ppi, false);
   if (sampler == nullptr)
      return ImGui::MarkdownImageData {};

   // Fixed image scaling corresponding to InGameUI overlay size for an image of 1024px width
   const ImGuiIO &io = ImGui::GetIO();
   const float imgScale = (io.DisplaySize.x > io.DisplaySize.y ? 0.7f : 1.5f) * io.DisplaySize.x / 1024.f;
   ImGui::MarkdownImageData imageData { true, false, sampler, ImVec2(static_cast<float>(sampler->GetWidth()), static_cast<float>(sampler->GetHeight())) * imgScale };
   ImVec2 const contentSize = ImGui::GetContentRegionAvail();
   if (imageData.size.x > contentSize.x)
   {
      float const ratio = imageData.size.y / imageData.size.x;
      imageData.size.x = contentSize.x;
      imageData.size.y = contentSize.x * ratio;
   }
   return imageData;
}

// SDL's window size can lag behind its pixel size (Wayland, rotated display, while loading: the window size is still the unrotated one),
// so the per axis pixel/window ratios disagree, and SDL_GetWindowPixelDensity (width only) is off. The true density is the same on both
// axes: the geometric mean of the ratios is exact for a transposed stale size, and unchanged when the sizes agree
static float GetIsotropicPixelDensity(const float scaleX, const float scaleY)
{
   return (scaleX > 0.f && scaleY > 0.f) ? sqrtf(scaleX * scaleY) : 1.f;
}

static float GetIsotropicPixelDensity(SDL_Window* const wnd)
{
   int w, h, pw, ph;
   if (wnd == nullptr || !SDL_GetWindowSize(wnd, &w, &h) || !SDL_GetWindowSizeInPixels(wnd, &pw, &ph) || w <= 0 || h <= 0)
      return 1.f;
   return GetIsotropicPixelDensity(static_cast<float>(pw) / static_cast<float>(w), static_cast<float>(ph) / static_cast<float>(h));
}

void LiveUI::UpdateScale()
{
   const float prevDPI = m_uiScale;
   float overlayScale;
   if (m_player->m_vrDevice)
   {
// The UI renders on a dedicated 16:9 surface in VR: scale content at 2x a
// desktop frame for readability (fallback to eye resolution before the
// surface is created)
#ifdef ENABLE_XR
      const unsigned int surfaceH = m_player->m_vrDevice->GetUISurfaceHeight();
      m_uiScale = surfaceH != 0 ? static_cast<float>(surfaceH) / 540.f : static_cast<float>(min(m_player->m_vrDevice->GetEyeWidth(), m_player->m_vrDevice->GetEyeHeight())) / 1000.f;
#else
      m_uiScale = static_cast<float>(min(m_player->m_vrDevice->GetEyeWidth(), m_player->m_vrDevice->GetEyeHeight())) / 2000.f;
#endif
      overlayScale = m_uiScale;
   }
   else
   {
      // Use display DPI setting
      // On macOS/iOS, keep m_uiScale at 1.0f. ImGui_ImplSDL3_NewFrame applies a 2.0f DisplayFramebufferScale
      // for SDL_WINDOW_HIGH_PIXEL_DENSITY windows. A m_uiScale of 2.0 would cause the UI to scale at 400%.
      // See: https://wiki.libsdl.org/SDL3/README/highdpi
      m_uiScale = SDL_GetWindowDisplayScale(m_player->m_playfieldWnd->GetCore()) / GetIsotropicPixelDensity(m_player->m_playfieldWnd->GetCore());
      overlayScale = m_uiScale;

      // For cabinet mode, the user is not standing in front of screen, so scale out the UI based on display size to be more readable (more "game like")
      if (m_player->m_ptable->GetViewMode() == ViewSetupID::BG_FULLSCREEN)
      {
         m_uiScale = max(m_uiScale, static_cast<float>(min(m_player->m_playfieldWnd->GetWidth(), m_player->m_playfieldWnd->GetHeight())) / 750.f);
      }
   }
   m_uiScale = min(m_uiScale, 10.f); // To avoid texture size overflows
   if (g_isMobile)
      m_uiScale *= 1.35f;
   if (m_uiScale != prevDPI)
   {
      m_perfUI.SetUIScale(overlayScale);
      m_plumbOverlay.SetUIScale(overlayScale);
      SetupImGuiStyle(m_editorUI.IsOpened());
      // The overlay context has its own independent style state (always the non editor variant)
      if (m_overlayContext != nullptr)
      {
         ImGuiContext *const prevContext = ImGui::GetCurrentContext();
         ImGui::SetCurrentContext(m_overlayContext);
         SetupImGuiStyle(false);
         ImGui::SetCurrentContext(prevContext);
      }
   }
}

void LiveUI::AddMousePosEvent(bool isTouch, float x, float y) const
{
   ImGuiIO &io = ImGui::GetIO();
   io.AddMouseSourceEvent(isTouch ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);
   switch (m_rotate)
   {
   case 0: io.AddMousePosEvent(x, y); break;
   case 1: io.AddMousePosEvent(y, io.DisplaySize.y - x); break;
   case 2: io.AddMousePosEvent(io.DisplaySize.x - x, io.DisplaySize.y - y); break;
   case 3: io.AddMousePosEvent(io.DisplaySize.x - y, x); break;
   default: assert(false); return;
   }
}

void LiveUI::HandleTrackpadFingers(const SDL_TouchFingerEvent &e)
{
   if (SDL_GetTouchDeviceType(e.touchID) == SDL_TOUCH_DEVICE_DIRECT)
      return;
   m_trackpadTouchID = e.touchID;
   int count = 0;
   SDL_Finger **fingers = SDL_GetTouchFingers(e.touchID, &count);
   if (fingers == nullptr || count != 2)
   {
      SDL_free(fingers);
      m_twistActive = false;
      return;
   }
   const float angle = atan2f(fingers[1]->y - fingers[0]->y, fingers[1]->x - fingers[0]->x);
   SDL_free(fingers);
   if (!m_twistActive)
   {
      m_twistActive = true;
      m_twistStarted = false;
      m_twistAngle = angle;
      m_twistAccumulated = 0.f;
      return;
   }
   float delta = angle - m_twistAngle;
   if (delta > M_PIf)
      delta -= 2.f * M_PIf;
   else if (delta < -M_PIf)
      delta += 2.f * M_PIf;
   m_twistAngle = angle;
   m_twistAccumulated -= RADTOANG(delta);
   if (!m_twistStarted && fabsf(m_twistAccumulated) < 5.f)
      return;
   m_twistStarted = true;
   m_pendingTwistDegrees += m_twistAccumulated;
   m_twistAccumulated = 0.f;
}

void LiveUI::HandleSDLEvent(SDL_Event &e)
{
   // The SDL backend is initialized on the main context only; the main context is also the one
   // input capture queries (ImGui::GetIO() outside of frame builds) are expected to land on
   ImGui::SetCurrentContext(m_mainContext);
   if (e.type == SDL_EVENT_MOUSE_MOTION)
   {
      // Custom implementation of ImGui_ImplSDL3_ProcessEvent supporting screen rotation and event filtering
      AddMousePosEvent(e.motion.which == SDL_TOUCH_MOUSEID, e.motion.x, e.motion.y);
   }
   else if (e.type == SDL_EVENT_PINCH_UPDATE)
   {
      m_pendingPinchScale *= e.pinch.scale;
   }
   else
   {
      // A touch press while no UI is open (menu or loading) goes to the touch regions and may open the InGame UI: drop its
      // SDL synthesized mouse clicks (sent before the finger event) until release, so they do not click the opened UI
      if (e.type == SDL_EVENT_FINGER_DOWN && SDL_GetTouchDeviceType(e.tfinger.touchID) == SDL_TOUCH_DEVICE_DIRECT && !IsOpened() && !m_player->m_isLoading)
         m_gameplayTouch = true;
      else if ((e.type == SDL_EVENT_MOUSE_BUTTON_DOWN || e.type == SDL_EVENT_MOUSE_BUTTON_UP) && e.button.which == SDL_TOUCH_MOUSEID)
      {
         if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && !IsOpened() && !m_player->m_isLoading)
            m_gameplayTouch = true;
         if (m_gameplayTouch)
         {
            if (e.type == SDL_EVENT_MOUSE_BUTTON_UP)
               m_gameplayTouch = false;
            return;
         }
      }
      else if ((e.type == SDL_EVENT_FINGER_UP || e.type == SDL_EVENT_FINGER_CANCELED) && m_gameplayTouch)
      {
         // Also ends without synthesized clicks (disabled by hint, or not the first finger)
         int count = 0;
         SDL_Finger **fingers = SDL_GetTouchFingers(e.tfinger.touchID, &count);
         SDL_free(fingers);
         if (count == 0)
            m_gameplayTouch = false;
      }
      if (e.type == SDL_EVENT_FINGER_DOWN || e.type == SDL_EVENT_FINGER_UP || e.type == SDL_EVENT_FINGER_MOTION || e.type == SDL_EVENT_FINGER_CANCELED)
         HandleTrackpadFingers(e.tfinger);
      else if (e.type == SDL_EVENT_MOUSE_WHEEL)
      {
         int count = 0;
         SDL_Finger **fingers = m_trackpadTouchID ? SDL_GetTouchFingers(m_trackpadTouchID, &count) : nullptr;
         SDL_free(fingers);
         m_pendingTrackpadScroll = count > 0 || (m_pendingTrackpadScroll && e.wheel.timestamp - m_lastWheelTimestamp < 100000000);
         m_pendingTrackpadScrollInverted = e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED;
         m_lastWheelTimestamp = e.wheel.timestamp;
      }
      ImGui_ImplSDL3_ProcessEvent(&e);
   }
}

// A windowed playfield gets a frame while editing or play testing so that it can be moved and resized like a regular editor window
void LiveUI::UpdatePlayfieldWindowFrame()
{
   VPX::Window* const wnd = m_player->m_playfieldWnd;
   if (wnd == nullptr || wnd->GetWindowMode() != VPX::Window::Windowed || m_player->IsVR() || g_isMobile)
      return;
   #ifdef __EMSCRIPTEN__
      return;
   #endif
   const bool framed = m_editorUI.IsOpened() || m_player->m_ptable->m_liveBaseTable != nullptr;
   if (framed == m_playfieldWindowFramed)
      return;
   m_playfieldWindowFramed = framed;
   SDL_SetWindowBordered(wnd->GetCore(), framed);
   SDL_SetWindowResizable(wnd->GetCore(), framed);
}

void LiveUI::NewFrame()
{
   UpdatePlayfieldWindowFrame();
   ImGuiIO &io = ImGui::GetIO();
   if (m_editorUI.IsOpened() || SDL_CursorVisible())
      io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
   else
      io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
   UpdateScale();
   ImGui_ImplSDL3_NewFrame();
   m_pinchScale = m_pendingPinchScale;
   m_pendingPinchScale = 1.f;
   m_twistDegrees = m_pendingTwistDegrees;
   m_pendingTwistDegrees = 0.f;
   m_trackpadScroll = m_pendingTrackpadScroll;
   m_trackpadScrollInverted = m_pendingTrackpadScrollInverted;

   // Adjust display size and scale eventually gathered in ImGui_ImplSDL3_NewFrame to our framebuffer rendering with rotation & stereo support
   {
      float renderWidth = m_rd->GetCurrentPass() ? static_cast<float>(m_rd->GetCurrentPass()->m_rt->GetWidth()) : 1920.f;
      float renderHeight = m_rd->GetCurrentPass() ? static_cast<float>(m_rd->GetCurrentPass()->m_rt->GetHeight()) : 1080.f;
#ifdef ENABLE_XR
      // In VR the UI is laid out at its virtual panel resolution (fixed 16:9), then executed
      // as world-space geometry on the quad VRDevice::UpdateUIPointer raycasts — the target
      // size is the eye resolution and does not reflect the panel's layout space
      if (m_player->m_vrDevice != nullptr && m_renderer->m_stereo3D == STEREO_VR && m_player->m_vrDevice->GetUISurfaceWidth() != 0)
      {
         renderWidth = static_cast<float>(m_player->m_vrDevice->GetUISurfaceWidth());
         renderHeight = static_cast<float>(m_player->m_vrDevice->GetUISurfaceHeight());
      }
#endif

      // The VR playfield window has no SDL window: the window size and scale gathered by ImGui_ImplSDL3_NewFrame
      // are undefined there (failed SDL calls leave their outputs untouched), so the framebuffer scale can be
      // anything, including positive garbage or NaN (issue #4294)
      if (m_player->m_playfieldWnd->GetCore() == nullptr)
         io.DisplayFramebufferScale = ImVec2(1.f, 1.f);
      if (io.DisplayFramebufferScale.x <= 0.f)
         io.DisplayFramebufferScale.x = 1.f;
      if (io.DisplayFramebufferScale.y <= 0.f)
         io.DisplayFramebufferScale.y = 1.f;
#ifndef __APPLE__ // Apple: ImGui_ImplSDL3_NewFrame already uses a single display scale
      io.DisplayFramebufferScale.x = io.DisplayFramebufferScale.y = GetIsotropicPixelDensity(io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
#endif
      const StereoMode stereoMode = m_player->m_renderer->m_stereo3Denabled ? m_player->m_renderer->m_stereo3D : STEREO_OFF;
      switch (stereoMode)
      {
      // Render is a vertically squashed view which is stretched back by the display
      case STEREO_TB:
      case STEREO_INT:
      case STEREO_FLIPPED_INT: io.DisplayFramebufferScale.y *= 0.5f; break;

      // Render is a horizontally squashed view which is stretched back by the display
      case STEREO_SBS: io.DisplayFramebufferScale.x *= 0.5f; break;

      default: break;
      }

      io.DisplaySize.x = renderWidth / io.DisplayFramebufferScale.x;
      io.DisplaySize.y = renderHeight / io.DisplayFramebufferScale.y;

      // Rotation from the render size like the 3D view (see RenderView), not from the display size above, which depends on SDL's window size
      if (m_renderer->m_stereo3D == STEREO_VR)
         m_rotate = 0;
      else
         m_rotate = static_cast<int>(m_player->m_ptable->GetViewSetup().GetRotation(stereoMode, static_cast<int>(renderWidth), static_cast<int>(renderHeight)) / 90.0f);
      if (m_rotate == 1 || m_rotate == 3)
      {
         std::swap(io.DisplaySize.x, io.DisplaySize.y);
         std::swap(io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
      }
   }

   // Enable mouse capture when dragging (needed when dragging main windows)
   {
      bool want_capture = false;
      for (int button_n = 0; button_n < ImGuiMouseButton_COUNT && !want_capture; button_n++)
         if (ImGui::IsMouseDragging(button_n, 1.0f))
            want_capture = true;
      SDL_CaptureMouse(want_capture);
   }

   // Late mouse position update to latest (async) global state (needed when dragging main windows)
   {
      SDL_Point windowPos { 0, 0 };
      SDL_FPoint globalMouse;
      SDL_GetGlobalMouseState(&globalMouse.x, &globalMouse.y);
      if (SDL_Window* const sdlWnd = m_player->m_playfieldWnd->GetCore())
         SDL_GetWindowPosition(sdlWnd, &windowPos.x, &windowPos.y);
      AddMousePosEvent(false, globalMouse.x - static_cast<float>(windowPos.x), globalMouse.y - static_cast<float>(windowPos.y));
   }

// VR controller pointer: the aim pose raycast against the UI quad
// is computed per frame by the VRDevice; it drives ImGui like a mouse, the
// front trigger acting as the left button
#ifdef ENABLE_XR
   if (m_player->m_vrDevice != nullptr)
   {
      const VRDevice::UIPointer &pointer = m_player->m_vrDevice->GetUIPointer();
      const bool pointerActive = m_uiLayerShown && m_inGameUI.IsOpened() && pointer.valid;
      if (pointerActive)
         AddMousePosEvent(false, pointer.u * io.DisplaySize.x, pointer.v * io.DisplaySize.y);
      const bool click = pointerActive && pointer.click;
      if (click != m_vrPointerClick)
      {
         io.AddMouseButtonEvent(ImGuiMouseButton_Left, click);
         m_vrPointerClick = click;
      }
      // No software cursor in VR: the world-space pick dot marks the hovered point
      io.MouseDrawCursor = false;
   }
#endif

   // We implement our own keyboard navigation using flipper keys/gamepad/VR controller
   io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;

   ImGui::NewFrame();
}

#ifdef ENABLE_XR
// Inset of the glass overlay footprint from the playfield edges, in cabinet units (1 inch per edge)
static constexpr float kGlassOverlayInset = MMTOVPU(25.4f);
// Logical resolution of the glass overlay surface, in UI pixels per inch of glass: sets the physical
// size of the overlay content (fonts are rasterized at the UI scale, their physical height on the
// glass is inversely proportional to this value)
static constexpr float kGlassOverlayPxPerInch = 200.f;

// Inset glass rectangle in playfield space (x0..x1 across, y0..y1 back to front, z at the glass level,
// linearly interpolated between the table's glass heights which may differ at the back and front edges)
static void GetGlassOverlayRect(const PinTable *const table, float &x0, float &y0, float &x1, float &y1, float &zBack, float &zFront)
{
   const float inset = min(kGlassOverlayInset, min(table->m_right - table->m_left, table->m_bottom - table->m_top) * 0.25f);
   x0 = table->m_left + inset;
   x1 = table->m_right - inset;
   y0 = table->m_top + inset;
   y1 = table->m_bottom - inset;
   const float depth = max(1e-6f, table->m_bottom - table->m_top);
   zBack = table->m_glassTopHeight + (y0 - table->m_top) / depth * (table->m_glassBottomHeight - table->m_glassTopHeight);
   zFront = table->m_glassTopHeight + (y1 - table->m_top) / depth * (table->m_glassBottomHeight - table->m_glassTopHeight);
}
#endif

void LiveUI::NewOverlayFrame()
{
   ImGuiContext *const prevContext = ImGui::GetCurrentContext();
   ImGui::SetCurrentContext(m_overlayContext);
   ImGuiIO &io = ImGui::GetIO();
   const ImGuiIO &mainIO = m_mainContext->IO;
   io.DeltaTime = mainIO.DeltaTime;
   io.DisplaySize = mainIO.DisplaySize;
   io.DisplayFramebufferScale = mainIO.DisplayFramebufferScale;
#ifdef ENABLE_XR
   // In VR, outside of loading, the overlay is laid out on its own portrait surface matching the
   // aspect ratio of the inset glass footprint it is rendered onto (so text is not stretched)
   if (m_player->m_vrDevice != nullptr && m_renderer->m_stereo3D == STEREO_VR && !m_player->m_isLoading && m_player->m_ptable != nullptr)
   {
      float x0, y0, x1, y1, zBack, zFront;
      GetGlassOverlayRect(m_player->m_ptable, x0, y0, x1, y1, zBack, zFront);
      const float widthInches = VPUTOMM(x1 - x0) / 25.4f;
      io.DisplaySize.x = max(1.f, widthInches * kGlassOverlayPxPerInch);
      io.DisplaySize.y = io.DisplaySize.x * (y1 - y0) / max(1e-6f, x1 - x0);
      io.DisplayFramebufferScale = ImVec2(1.f, 1.f);
   }
#endif
   ImGui::NewFrame();
   ImGui::SetCurrentContext(prevContext);
}

void LiveUI::PrepareScene()
{
   if (m_editorUI.IsOpened())
      m_editorUI.PrepareScene();
}

void LiveUI::Render3D()
{
   if (m_editorUI.IsOpened())
      m_editorUI.Render3D();
}

#ifdef ENABLE_XR
// Depth-only draw of the UI quad into the scene depth buffer: the VR color key passthrough
// (fs_pp_passthrough) punches through every pixel at far depth (== 1.0), but the UI draw list is
// executed post-process and writes no depth. Stamping the panel's footprint inside the scene pass
// keeps its pixels classified as scene geometry. The write is unconditional since the UI always
// draws on top (the stamp is invisible either way, so it only marks the keying mask).
void LiveUI::RenderVRUIDepthStamp(const Vertex3Ds corners[4], const ModelViewProj &mvp)
{
   m_rd->ResetRenderState();
   m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_TRUE);
   m_rd->SetRenderState(RenderState::ZFUNC, RenderState::Z_ALWAYS);
   m_rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_TRUE);
   m_rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   m_rd->SetRenderState(RenderState::COLORWRITEENABLE, RenderState::RGBMASK_NONE);
   m_rd->m_basicShader->SetTechnique(ShaderTechnique::unshaded_without_texture);
   const Matrix3D matIdentity = Matrix3D::MatrixIdentity();
   m_rd->m_basicShader->SetMatrix(ShaderUniform::matWorld, &matIdentity);
   m_rd->m_basicShader->SetMatrix(ShaderUniform::matView, &mvp.GetView(0), mvp.m_nEyes);
   m_rd->m_basicShader->SetMatrix(ShaderUniform::matWorldView, &mvp.GetModelView(0), mvp.m_nEyes);
   m_rd->m_basicShader->SetMatrix(ShaderUniform::matWorldViewInverseTranspose, &mvp.GetModelViewInverseTranspose(0), mvp.m_nEyes);
   m_rd->m_basicShader->SetMatrix(ShaderUniform::matRotViewProj, &mvp.GetRotViewProj(0), mvp.m_nEyes);
   m_rd->m_basicShader->SetVector(ShaderUniform::cameraPosWorld, &mvp.GetCameraPos(0), mvp.m_nEyes);
   m_rd->m_basicShader->SetVector(ShaderUniform::staticColor_Alpha, 1.f, 1.f, 1.f, 1.f);
   const Vertex3D_NoTex2 vertices[4] = { //
      { corners[0].x, corners[0].y, corners[0].z, 0.f, 0.f, 1.f, 0.f, 0.f }, // Top left
      { corners[1].x, corners[1].y, corners[1].z, 0.f, 0.f, 1.f, 1.f, 0.f }, // Top right
      { corners[2].x, corners[2].y, corners[2].z, 0.f, 0.f, 1.f, 0.f, 1.f }, // Bottom left
      { corners[3].x, corners[3].y, corners[3].z, 0.f, 0.f, 1.f, 1.f, 1.f }
   }; // Bottom right
   m_rd->DrawTexturedQuad(m_rd->m_basicShader, vertices, false);
}

// VR controller pointer, matching Virtual Desktop's visual language: a narrow white
// beam fading over a fixed reach, with a small opaque contact point lying in the UI plane.
void LiveUI::RenderVRPointer(const Vertex3Ds uiQuadScene[4], const ModelViewProj &uiMVP)
{
   if (m_player->m_vrDevice == nullptr || !m_uiLayerShown || !m_inGameUI.IsOpened())
      return;
   const VRDevice::UIPointer &pointer = m_player->m_vrDevice->GetUIPointer();
   if (!pointer.valid)
      return;

   if (m_laserSampler == nullptr)
   {
      // Alpha across the beam is a soft-edged strip; alpha along it is a fixed-length fade.
      // Encoding the fade against the maximum reach keeps the end visible when it lands on a near surface.
      const auto tex = BaseTexture::Create(64, 16, BaseTexture::Format::SRGBA);
      if (tex != nullptr)
      {
         uint32_t *const texels = static_cast<uint32_t *>(tex->data());
         for (unsigned int y = 0; y < 16; ++y)
         {
            const float across = fabsf((static_cast<float>(y) + 0.5f) * (2.f / 16.f) - 1.f);
            float edgeAlpha = clamp((1.f - across) / 0.65f, 0.f, 1.f);
            edgeAlpha = edgeAlpha * edgeAlpha * (3.f - 2.f * edgeAlpha);
            for (unsigned int x = 0; x < 64; ++x)
            {
               const float along = (static_cast<float>(x) + 0.5f) / 64.f;
               const float lengthAlpha = 1.f - along * along * along;
               const float alpha = lengthAlpha * edgeAlpha;
               texels[y * 64 + x] = 0x00FFFFFFu | (static_cast<uint32_t>(alpha * 255.f + 0.5f) << 24u);
            }
         }
         m_laserSampler = std::make_shared<Sampler>(m_rd, "UIPointerRay"s, tex, false);
      }
   }
   if (m_laserDotSampler == nullptr)
   {
      // Small filled contact point with a narrow antialiased edge.
      const auto tex = BaseTexture::Create(32, 32, BaseTexture::Format::SRGBA);
      if (tex != nullptr)
      {
         uint32_t *const texels = static_cast<uint32_t *>(tex->data());
         for (unsigned int y = 0; y < 32; ++y)
            for (unsigned int x = 0; x < 32; ++x)
            {
               const float dx = static_cast<float>(x) - 15.5f;
               const float dy = static_cast<float>(y) - 15.5f;
               const float alpha = clamp((14.5f - sqrtf(dx * dx + dy * dy)) / 1.2f, 0.f, 1.f);
               texels[y * 32 + x] = 0x00FFFFFFu | (static_cast<uint32_t>(alpha * 255.f + 0.5f) << 24u);
            }
         m_laserDotSampler = std::make_shared<Sampler>(m_rd, "UIPointerDot"s, tex, false);
      }
   }
   if (m_laserSampler == nullptr)
      return;

   m_rd->ResetRenderState();
   m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE); // Composited over the UI panel
   m_rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
   m_rd->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
   m_rd->SetRenderState(RenderState::DESTBLEND, RenderState::INVSRC_ALPHA);
   m_rd->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
   m_rd->m_basicShader->SetTechnique(ShaderTechnique::unshaded_with_texture);
   const Matrix3D matIdentity = Matrix3D::MatrixIdentity();
   m_rd->m_basicShader->SetMatrix(ShaderUniform::matWorld, &matIdentity);
   m_rd->m_basicShader->SetMatrix(ShaderUniform::matView, &uiMVP.GetView(0), uiMVP.m_nEyes);
   m_rd->m_basicShader->SetMatrix(ShaderUniform::matWorldView, &uiMVP.GetModelView(0), uiMVP.m_nEyes);
   m_rd->m_basicShader->SetMatrix(ShaderUniform::matWorldViewInverseTranspose, &uiMVP.GetModelViewInverseTranspose(0), uiMVP.m_nEyes);
   m_rd->m_basicShader->SetMatrix(ShaderUniform::matRotViewProj, &uiMVP.GetRotViewProj(0), uiMVP.m_nEyes);
   m_rd->m_basicShader->SetVector(ShaderUniform::cameraPosWorld, &uiMVP.GetCameraPos(0), uiMVP.m_nEyes);
   m_rd->m_basicShader->SetVector(ShaderUniform::staticColor_Alpha, 1.f, 1.f, 1.f, 1.f);

   const Vertex3Ds origin(pointer.sceneOrigin[0], pointer.sceneOrigin[1], pointer.sceneOrigin[2]);
   const Vertex3Ds hit(pointer.sceneHit[0], pointer.sceneHit[1], pointer.sceneHit[2]);
   const Vertex3Ds head(pointer.headScene[0], pointer.headScene[1], pointer.headScene[2]);
   Vertex3Ds dir = hit - origin;
   const float hitDistance = dir.Length();
   if (hitDistance > 1e-6f)
   {
      dir.Normalize();
      constexpr float maxLength = MMTOVPU(350.f); // Maximum visible beam reach, like Virtual Desktop
      const float beamLength = min(hitDistance, maxLength);
      const Vertex3Ds end = origin + dir * beamLength;

      // Ribbon billboarding: its side axis is perpendicular to both the
      // ray and the direction toward the head, so it faces the viewer
      const Vertex3Ds mid = (origin + end) * 0.5f;
      Vertex3Ds toHead = head - mid;
      toHead.Normalize();
      Vertex3Ds side = CrossProduct(dir, toHead);
      if (side.Length() <= 1e-6f)
      {
         side = CrossProduct(dir, Vertex3Ds(0.f, 0.f, 1.f));
         if (side.Length() <= 1e-6f)
            side = Vertex3Ds(1.f, 0.f, 0.f);
      }
      side.Normalize();
      constexpr float halfW = MMTOVPU(1.5f); // 3mm overall width, including the soft edge
      const Vertex3Ds edge = side * halfW;
      const float beamU = beamLength / maxLength;
      const Vertex3D_NoTex2 vertices[4] = { //
         { origin.x + edge.x, origin.y + edge.y, origin.z + edge.z, 0.f, 0.f, 1.f, 0.f, 0.f }, //
         { end.x + edge.x, end.y + edge.y, end.z + edge.z, 0.f, 0.f, 1.f, beamU, 0.f }, //
         { origin.x - edge.x, origin.y - edge.y, origin.z - edge.z, 0.f, 0.f, 1.f, 0.f, 1.f }, //
         { end.x - edge.x, end.y - edge.y, end.z - edge.z, 0.f, 0.f, 1.f, beamU, 1.f }
      };
      m_rd->m_basicShader->SetTexture(ShaderUniform::tex_base_color, m_laserSampler, SamplerFilter::SF_BILINEAR);
      m_rd->DrawTexturedQuad(m_rd->m_basicShader, vertices, true, -10001.f); // After the -10000 UI panel
   }

   // Contact point: a small opaque disk lying in the UI plane rather than a head-facing billboard.
   if (pointer.onQuad && m_laserDotSampler != nullptr)
   {
      Vertex3Ds dotRight = uiQuadScene[1] - uiQuadScene[0];
      Vertex3Ds dotDown = uiQuadScene[2] - uiQuadScene[0];
      if (dotRight.Length() > 1e-6f && dotDown.Length() > 1e-6f)
      {
         dotRight.Normalize();
         dotDown.Normalize();
         constexpr float r = MMTOVPU(2.1f); // Approximately 4mm visible diameter
         const Vertex3Ds a = dotRight * r;
         const Vertex3Ds b = dotDown * r;
         const Vertex3D_NoTex2 vertices[4] = { //
            { hit.x - a.x + b.x, hit.y - a.y + b.y, hit.z - a.z + b.z, 0.f, 0.f, 1.f, 0.f, 0.f }, //
            { hit.x + a.x + b.x, hit.y + a.y + b.y, hit.z + a.z + b.z, 0.f, 0.f, 1.f, 1.f, 0.f }, //
            { hit.x - a.x - b.x, hit.y - a.y - b.y, hit.z - a.z - b.z, 0.f, 0.f, 1.f, 0.f, 1.f }, //
            { hit.x + a.x - b.x, hit.y + a.y - b.y, hit.z + a.z - b.z, 0.f, 0.f, 1.f, 1.f, 1.f }
         };
         m_rd->m_basicShader->SetTexture(ShaderUniform::tex_base_color, m_laserDotSampler, SamplerFilter::SF_BILINEAR);
         m_rd->DrawTexturedQuad(m_rd->m_basicShader, vertices, true, -10002.f); // After the beam and UI panel
      }
   }
}

#endif

void LiveUI::RenderUI()
{
   // SDR white level to place UI white at, normalized to the 10000 nits PQ encodes (the white point counts in multiples of 80 nits). 0 for a sRGB output
   const bool wcg = (m_player != nullptr) && m_player->m_playfieldWnd->IsWCGBackBuffer();
   ImGui::SetCurrentContext(m_mainContext);
   RenderUIInternal(wcg ? m_player->m_playfieldWnd->GetSDRWhitePoint() * (float)(80. / 10000.) : 0.f, true);
}

#ifdef ENABLE_XR
void LiveUI::RenderUIVR(const Vertex3Ds uiQuadScene[4], const ModelViewProj &uiMVP)
{
   if (m_player == nullptr || m_player->GetCloseState() != Player::CS_PLAYING || m_rd->GetCurrentPass() == nullptr || m_player->m_playMode == Player::PlayMode::CaptureAttract)
      return;

   // World-space UI (VR): map the UI pixel space onto the quad VRDevice::UpdateUIPointer
   // raycasts (same geometry, top-left origin, row vectors applied left to right), then
   // through each eye's view/projection of the quad's space (m_rotate is forced to 0 for VR)
   ImGui::SetCurrentContext(m_mainContext);
   const ImGuiIO &io = ImGui::GetIO();
   const float right = io.DisplaySize.x;
   const float bottom = io.DisplaySize.y;
   const float invW = 1.f / right;
   const float invH = 1.f / bottom;
   Matrix3D uiToScene;
   uiToScene._11 = (uiQuadScene[1].x - uiQuadScene[0].x) * invW;
   uiToScene._12 = (uiQuadScene[1].y - uiQuadScene[0].y) * invW;
   uiToScene._13 = (uiQuadScene[1].z - uiQuadScene[0].z) * invW;
   uiToScene._14 = 0.f;
   uiToScene._21 = (uiQuadScene[2].x - uiQuadScene[0].x) * invH;
   uiToScene._22 = (uiQuadScene[2].y - uiQuadScene[0].y) * invH;
   uiToScene._23 = (uiQuadScene[2].z - uiQuadScene[0].z) * invH;
   uiToScene._24 = 0.f;
   uiToScene._31 = uiToScene._32 = uiToScene._33 = uiToScene._34 = 0.f;
   uiToScene._41 = uiQuadScene[0].x;
   uiToScene._42 = uiQuadScene[0].y;
   uiToScene._43 = uiQuadScene[0].z;
   uiToScene._44 = 1.f;
   Matrix3D matQuadView[2];
   for (int eye = 0; eye < m_rd->m_nEyes; eye++)
      matQuadView[eye] = uiToScene * uiMVP.GetView(eye) * uiMVP.GetProj(eye);

   // The draw list is rasterized to an offscreen mipmapped texture, then drawn back as a textured
   // quad (below): the XR surface is single sample (no MSAA) and the panel minifies on screen, so
   // in-place geometry aliases badly while the composition layer this replaces was texture-filtered
   RenderTarget* const uiTarget = m_rd->GetCurrentPass()->m_rt;
   const int panelWidth = static_cast<int>(right), panelHeight = static_cast<int>(bottom);
   if ((m_uiPanelTarget == nullptr) || (m_uiPanelTarget->GetWidth() != panelWidth) || (m_uiPanelTarget->GetHeight() != panelHeight))
      m_uiPanelTarget = std::make_unique<RenderTarget>(m_rd, SurfaceType::RT_DEFAULT, "LiveUI Panel"s, panelWidth, panelHeight, colorFormat::RGBA8,
         false, 1, "Fatal Error: unable to create LiveUI panel render target", nullptr, true);
   m_rd->SetRenderTarget("LiveUI Panel"s, m_uiPanelTarget.get(), false);
   m_rd->Clear(clearType::TARGET, 0xFF000000); // Opaque black backing: the world-space UI is a solid surface

   // The panel holds sRGB encoded values. While playing, the overlay layer is not part of the
   // panel: it is emitted as world space geometry on the table glass inside the scene pass (see
   // RenderOverlayVR). While loading there is no scene pass, so it falls back to the panel.
   RenderUIInternal(0.f, m_player->m_isLoading);

   // Composite the panel texture on the world-space quad, into the XR surface holding linear
   // values (see the tonemap pass' isLinearFrameBuffer). The panel is alpha-composited over
   // its opaque black backing, and the composite forces opaque output since the world-space
   // UI is a solid surface
   m_rd->SetRenderTarget("LiveUI"s, uiTarget, true);
   m_rd->AddRenderTargetDependency(m_uiPanelTarget.get());
   m_rd->m_uiShader->SetTechnique(uiTarget->m_nLayers == 1 ? ShaderTechnique::LiveUI_mono : ShaderTechnique::LiveUI);
   m_rd->m_uiShader->SetMatrix(ShaderUniform::matWorldView, &matQuadView[0], m_rd->m_nEyes);
   const vec4 quadParams(1.f, 0.f, 1.f, 0.f); // x = force opaque, z = linear output
   m_rd->m_uiShader->SetVector(ShaderUniform::staticColor_Alpha, &quadParams);
   m_rd->m_uiShader->SetVector(ShaderUniform::clip_plane, 0.f, 0.f, right, bottom);
   m_rd->m_uiShader->SetTexture(ShaderUniform::tex_base_color, m_uiPanelTarget->GetColorSampler(), SamplerFilter::SF_ANISOTROPIC);
   m_rd->SetRenderState(RenderState::SRCBLEND, RenderState::ONE);
   m_rd->SetRenderState(RenderState::DESTBLEND, RenderState::INVSRC_ALPHA);
   if (m_uiPanelQuad == nullptr)
      m_uiPanelQuad = std::make_shared<UIListGeometry>();
   m_uiPanelQuad->numVertices = 4;
   m_uiPanelQuad->numIndices = 6;
   m_uiPanelQuad->upload = UIListGeometry::Upload::None;
   m_uiPanelQuad->vtx.resize(4 * sizeof(ImDrawVert));
   m_uiPanelQuad->idx.resize(6 * sizeof(ImDrawIdx));
   ImDrawVert *const quadVtx = reinterpret_cast<ImDrawVert *>(m_uiPanelQuad->vtx.data());
   quadVtx[0].pos = ImVec2(0.f, 0.f);      quadVtx[0].uv = ImVec2(0.f, 0.f); quadVtx[0].col = IM_COL32_WHITE;
   quadVtx[1].pos = ImVec2(right, 0.f);    quadVtx[1].uv = ImVec2(1.f, 0.f); quadVtx[1].col = IM_COL32_WHITE;
   quadVtx[2].pos = ImVec2(0.f, bottom);   quadVtx[2].uv = ImVec2(0.f, 1.f); quadVtx[2].col = IM_COL32_WHITE;
   quadVtx[3].pos = ImVec2(right, bottom); quadVtx[3].uv = ImVec2(1.f, 1.f); quadVtx[3].col = IM_COL32_WHITE;
   ImDrawIdx *const quadIdx = reinterpret_cast<ImDrawIdx *>(m_uiPanelQuad->idx.data());
   quadIdx[0] = 0; quadIdx[1] = 2; quadIdx[2] = 1; quadIdx[3] = 1; quadIdx[4] = 2; quadIdx[5] = 3;
   m_rd->DrawTransientMesh(m_rd->m_uiShader, true, -10000.f, m_uiPanelQuad, RenderDevice::TRIANGLELIST, 0, 6);
   RenderVRPointer(uiQuadScene, uiMVP);
}

// Non interactive overlay layer in VR: the draw lists are executed as world space geometry lying
// on the table glass (inset footprint at the glass level), recorded inside the scene pass so they
// are depth tested against the physical scene. Unlike the interactive panel, there is no offscreen
// rasterization: the commands are pushed directly with the glass mapping as their MVP, alpha blended
// over the scene. Being part of the linear scene render, the overlay participates in the scene's
// exposure/tonemapping like the rest of the world geometry (isLinearFB in the UI shader).
void LiveUI::RenderOverlayVR(const ModelViewProj &playfieldMVP)
{
   if (m_player == nullptr || m_player->GetCloseState() != Player::CS_PLAYING || m_rd->GetCurrentPass() == nullptr || m_player->m_playMode == Player::PlayMode::CaptureAttract)
      return;
   if (m_player->m_isLoading || m_player->m_ptable == nullptr)
      return; // While loading there is no scene pass: the overlay falls back to the UI panel (RenderUIVR)

   ImGuiContextScope scope(m_overlayContext);

   BuildOverlayUI();

   ImGui::EndFrame();
   ImGui::Render();
   ImDrawData *const draw_data = ImGui::GetDrawData();
   UpdateImGuiTextures(draw_data);

   // Map the overlay pixel space onto the inset glass footprint (top-left origin, row vectors applied
   // left to right, same construction as the UI panel): x across the table, y from back to front, so
   // that desktop bottom anchored content lands near the player, at the interpolated glass height
   const ImGuiIO &io = ImGui::GetIO();
   float x0, y0, x1, y1, zBack, zFront;
   GetGlassOverlayRect(m_player->m_ptable, x0, y0, x1, y1, zBack, zFront);
   const float invW = 1.f / io.DisplaySize.x;
   const float invH = 1.f / io.DisplaySize.y;
   Matrix3D uiToScene;
   uiToScene._11 = (x1 - x0) * invW;
   uiToScene._12 = 0.f;
   uiToScene._13 = 0.f;
   uiToScene._14 = 0.f;
   uiToScene._21 = 0.f;
   uiToScene._22 = (y1 - y0) * invH;
   uiToScene._23 = (zFront - zBack) * invH;
   uiToScene._24 = 0.f;
   uiToScene._31 = uiToScene._32 = uiToScene._33 = uiToScene._34 = 0.f;
   uiToScene._41 = x0;
   uiToScene._42 = y0;
   uiToScene._43 = zBack;
   uiToScene._44 = 1.f;
   Matrix3D matGlassView[2];
   for (int eye = 0; eye < m_rd->m_nEyes; eye++)
      matGlassView[eye] = uiToScene * playfieldMVP.GetView(eye) * playfieldMVP.GetProj(eye);

   m_rd->m_uiShader->SetMatrix(ShaderUniform::matWorldView, &matGlassView[0], m_rd->m_nEyes);
#ifdef ENABLE_BGFX
   if (m_rd->GetCurrentPass()->m_rt->m_nLayers == 1)
      m_rd->m_uiShader->SetTechnique(ShaderTechnique::LiveUI_mono);
   else
#endif
      m_rd->m_uiShader->SetTechnique(ShaderTechnique::LiveUI);
   // z = output linear values (the scene pass holds linear values, unlike the display space UI paths)
   vec4 uiParams(0.f, 0.f, 1.f, 0.f);
   m_rd->ResetRenderState();
   m_rd->SetRenderState(RenderState::COLORWRITEENABLE, RenderState::RGBMASK_RGBA);
   m_rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
   m_rd->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
   m_rd->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
   m_rd->SetRenderState(RenderState::DESTBLEND, RenderState::INVSRC_ALPHA);
   m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_TRUE); // Physically depth tested against the scene
   m_rd->SetRenderState(RenderState::ZFUNC, RenderState::Z_LESSEQUAL);
   m_rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE); // But coplanar overlay primitives do not write depth
   // The overlay sorts after the scene's transparent parts (flasher DMDs sit at -10000): it is
   // the closest thing to the viewer, and transparent scene parts write depth anyway, so depth
   // testing keeps occlusion correct. Equal-depth commands keep their submission order (stable
   // sort), preserving ImGui's painter order within the layer.
   bool hasContent = false;
   EmitDrawListCommands(draw_data, uiParams, -20000.f, hasContent, m_overlayUIPool);

   NewOverlayFrame();
}
#endif

void LiveUI::BuildMainUI(const int width, const int height)
{
   UpdateTouchUI();

   ImGui::PushFont(m_baseFont, m_baseFont->LegacySize);

   // Tweak UI (aligned to playfield view, using custom flipper controls)
   m_inGameUI.Update();

   // Debugger tool window (may be opened along other UIs)
   m_debuggerUI.Update();

   if (m_editorUI.IsOpened())
   { // Editor UI (aligned to desktop, using traditional mouse interaction)
      SetupImGuiStyle(true);
      m_editorUI.RenderUI();
      SetupImGuiStyle(false);
   }
   else if (!m_inGameUI.IsOpened())
   { // No UI displayed: process ball control & throw balls
      m_ballControl.Update(width, height);
   }

   ImGui::PopFont();
}

void LiveUI::BuildOverlayUI()
{
   ImGui::PushFont(m_baseFont, m_baseFont->LegacySize);

   if (!m_player->IsPlaying() && !m_player->m_isLoading && !m_editorUI.IsOpened())
   {
      ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 24 * m_uiScale, 4 * m_uiScale));
      ImGui::Begin("PauseOverlay", nullptr,
         ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus // Prevent focus issues
            | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
      ImGui::Text(ICON_LC_PAUSE);
      ImGui::End();
   }

   // Display plumb state overlay
   m_plumbOverlay.Update();

   // Display notification overlays except when script has an unaligned rotation
   m_notificationOverlay.Update(true, m_overlayFont);

   // Display performance overlays
   m_perfUI.Update();

   ImGui::PopFont();
}

void LiveUI::UpdateImGuiTextures(ImDrawData *const draw_data)
{
   if (draw_data->Textures == nullptr)
      return;
   for (ImTextureData *tex : *draw_data->Textures)
   {
      if (tex->Status == ImTextureStatus_WantCreate || tex->Status == ImTextureStatus_WantUpdates)
      {
         assert(tex->GetPitch() == tex->Width * 4);
         assert(tex->Format == ImTextureFormat_RGBA32);
         if (tex->Status == ImTextureStatus_WantCreate)
            tex->BackendUserData = new std::shared_ptr<BaseTexture>();
         auto texture = static_cast<std::shared_ptr<BaseTexture> *>(tex->BackendUserData);
         BaseTexture::Update(*texture, tex->Width, tex->Height, BaseTexture::RGBA, static_cast<const uint8_t *>(tex->GetPixels()));
         (*texture)->SetName(std::format("ImGui.Tex{}", tex->UniqueID));
         tex->SetTexID(m_renderer->m_renderDevice->m_texMan.LoadPinnedTexture(texture->get()));
         tex->SetStatus(ImTextureStatus_OK);
      }
      else if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames > 0)
      {
         ReleaseImGuiTexture(tex);
         tex->SetTexID(ImTextureID_Invalid);
         tex->SetStatus(ImTextureStatus_Destroyed);
      }
   }
}

float LiveUI::EmitDrawListCommands(ImDrawData *const draw_data, vec4 uiParams, const float depthStart, bool &hasContent, UIGeometryPool &pool)
{
   if (static_cast<int>(pool.meshBuffers.size()) < draw_data->CmdListsCount)
      pool.meshBuffers.resize(draw_data->CmdListsCount);
   float depthSort = depthStart;
   hasContent = false;
   for (int n = 0; n < draw_data->CmdListsCount; n++)
   {
      const ImDrawList *const cmd_list = draw_data->CmdLists[n];
      const unsigned int numVertices = cmd_list->VtxBuffer.size();
      const unsigned int numIndices = cmd_list->IdxBuffer.size();
      hasContent |= numIndices > 0;

#if defined(ENABLE_BGFX)
      // Snapshot this command list's geometry: it is uploaded on the render thread, into BGFX's
      // per-frame transient buffers (single copy, no persistent buffer traffic)
      if (static_cast<int>(pool.uiGeometry.size()) <= n)
         pool.uiGeometry.resize(n + 1);
      if (pool.uiGeometry[n] == nullptr)
         pool.uiGeometry[n] = std::make_shared<UIListGeometry>();
      const std::shared_ptr<UIListGeometry> &geo = pool.uiGeometry[n];
      geo->numVertices = numVertices;
      geo->numIndices = numIndices;
      geo->upload = UIListGeometry::Upload::None;
      geo->vtx.resize(numVertices * sizeof(ImDrawVert));
      geo->idx.resize(numIndices * sizeof(ImDrawIdx));
      memcpy(geo->vtx.data(), cmd_list->VtxBuffer.Data, geo->vtx.size());
      memcpy(geo->idx.data(), cmd_list->IdxBuffer.Data, geo->idx.size());
#else
      if ((numVertices != 0) && (numIndices != 0))
      {
         if ((pool.meshBuffers[n] == nullptr) || (pool.meshBuffers[n]->m_ib->m_count < numIndices) || (pool.meshBuffers[n]->m_vb->m_count < numVertices))
         { // Grow at 1.5x the needed size to amortize reallocation when the UI keeps growing
            auto ib = std::make_shared<IndexBuffer>(m_rd, numIndices + (numIndices >> 1), true, IndexBuffer::Format::FMT_INDEX32);
            auto vb = std::make_shared<VertexBuffer>(m_rd, numVertices + (numVertices >> 1), nullptr, true, VertexFormat::VF_IMGUI);
            pool.meshBuffers[n] = std::make_shared<MeshBuffer>(std::format("ImGui.{}", n), vb, ib, false);
         }

         ImDrawVert *vb;
         pool.meshBuffers[n]->m_vb->Lock(vb, 0, numVertices * sizeof(ImDrawVert));
         memcpy(vb, cmd_list->VtxBuffer.Data, numVertices * sizeof(ImDrawVert));
         pool.meshBuffers[n]->m_vb->Unlock();

         uint32_t *ib;
         pool.meshBuffers[n]->m_ib->Lock(ib, 0, numIndices * sizeof(ImDrawIdx));
         memcpy(ib, cmd_list->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx));
         pool.meshBuffers[n]->m_ib->Unlock();
      }
#endif

      for (const ImDrawCmd *cmd = cmd_list->CmdBuffer.begin(), *cmdEnd = cmd_list->CmdBuffer.end(); cmd != cmdEnd; cmd++)
      {
         if (cmd->ElemCount != 0)
         {
            m_rd->m_uiShader->SetVector(ShaderUniform::clip_plane, cmd->ClipRect.x, cmd->ClipRect.y, cmd->ClipRect.z, cmd->ClipRect.w);
            m_rd->m_uiShader->SetTexture(ShaderUniform::tex_base_color, cmd->GetTexID(), m_player->IsVR() ? SamplerFilter::SF_ANISOTROPIC : SamplerFilter::SF_BILINEAR);
#if !defined(ENABLE_DX9) // DX9 samples raw values (see UIShader.hlsl)
            // ImGui works in sRGB: the shader re-encodes textures that sampling decodes (table images)
            uiParams.y = (cmd->GetTexID() != nullptr && cmd->GetTexID()->IsSRGB()) ? 1.f : 0.f;
            m_rd->m_uiShader->SetVector(ShaderUniform::staticColor_Alpha, &uiParams);
#endif
#if defined(ENABLE_BGFX)
            m_rd->DrawTransientMesh(m_rd->m_uiShader, true, depthSort, geo, RenderDevice::TRIANGLELIST, cmd->IdxOffset, cmd->ElemCount);
#else
            m_rd->DrawMesh(m_rd->m_uiShader, true, Vertex3Ds(0.f, 0.f, 0.f), depthSort, pool.meshBuffers[n], RenderDevice::TRIANGLELIST, cmd->IdxOffset, cmd->ElemCount);
#endif
            depthSort -= 1.f;
         }
      }
   }
   return depthSort;
}

float LiveUI::EmitDrawListsOrtho(ImDrawData *const draw_data, const float sdrWhitePQ, const float depthStart, bool &hasContent, UIGeometryPool &pool)
{
   const ImGuiIO &io = ImGui::GetIO();
   const Matrix3D matRotate = Matrix3D::MatrixRotateZ(static_cast<float>(m_rotate * (M_PI / 2.0)));
   Matrix3D matTranslate;
   switch (m_rotate)
   {
   case 0: matTranslate = Matrix3D::MatrixIdentity(); break;
   case 1: matTranslate = Matrix3D::MatrixTranslate(io.DisplaySize.y, 0, 0); break;
   case 2: matTranslate = Matrix3D::MatrixTranslate(io.DisplaySize.x, io.DisplaySize.y, 0); break;
   case 3: matTranslate = Matrix3D::MatrixTranslate(0, io.DisplaySize.x, 0); break;
   default: assert(false); return depthStart;
   }
   const float right = (m_rotate == 1 || m_rotate == 3) ? io.DisplaySize.y : io.DisplaySize.x;
   const float bottom = (m_rotate == 1 || m_rotate == 3) ? io.DisplaySize.x : io.DisplaySize.y;
   // Orthographic mapping of the UI pixel space onto the rasterization target
   Matrix3D matView[2];
   matView[0] = matRotate * matTranslate * Matrix3D::MatrixOrthoOffCenterRH(0.f, right, bottom, 0.f, 0.f, 1.f);
   if (m_rd->m_nEyes == 2)
      matView[1] = matView[0];
   m_rd->m_uiShader->SetMatrix(ShaderUniform::matWorldView, &matView[0], m_rd->m_nEyes);
   // UI shader parameters: y = texture is sRGB decoded (set per draw, unused on DX9), w = HDR white level (BGFX)
   vec4 uiParams(0.f, 0.f, 0.f, sdrWhitePQ);
   m_rd->ResetRenderState();
   m_rd->SetRenderState(RenderState::COLORWRITEENABLE, RenderState::RGBMASK_RGBA);
   m_rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
   m_rd->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
   m_rd->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
   m_rd->SetRenderState(RenderState::DESTBLEND, RenderState::INVSRC_ALPHA);
   m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
#ifdef ENABLE_BGFX
   if (m_rd->GetCurrentPass()->m_rt->m_nLayers == 1)
      m_rd->m_uiShader->SetTechnique(ShaderTechnique::LiveUI_mono);
   else
#endif
      m_rd->m_uiShader->SetTechnique(ShaderTechnique::LiveUI);
   return EmitDrawListCommands(draw_data, uiParams, depthStart, hasContent, pool);
}

void LiveUI::RenderUIInternal(const float sdrWhitePQ, const bool withOverlay)
{
   if (m_player == nullptr || m_player->GetCloseState() != Player::CS_PLAYING || m_rd->GetCurrentPass() == nullptr || m_player->m_playMode == Player::PlayMode::CaptureAttract)
      return;

   // Main (interactive) layer
   float depthSort = -10000.f;
   {
      ImGuiContextScope scope(m_mainContext);
      const ImGuiIO &io = ImGui::GetIO();
      const bool rotated = (m_rotate == 1 || m_rotate == 3);
      const int width = static_cast<int>(rotated ? io.DisplaySize.y : io.DisplaySize.x);
      const int height = static_cast<int>(rotated ? io.DisplaySize.x : io.DisplaySize.y);

      BuildMainUI(width, height);

      ImGui::EndFrame();
      ImGui::Render();
      ImDrawData *const draw_data = ImGui::GetDrawData();
      UpdateImGuiTextures(draw_data);
      bool hasContent = false;
      depthSort = EmitDrawListsOrtho(draw_data, sdrWhitePQ, depthSort, hasContent, m_mainUIPool);

      m_uiLayerShown = hasContent;
      if (m_player->m_vrDevice)
         m_player->m_vrDevice->SetShowUILayer(hasContent, m_inGameUI.IsOpened()); // Pointer interaction only with the InGame UI

      NewFrame();
   }

   // Overlay (non interactive) layer, always after the main one
   if (withOverlay)
   {
      ImGuiContextScope scope(m_overlayContext);

      BuildOverlayUI();

      ImGui::EndFrame();
      ImGui::Render();
      ImDrawData *const draw_data = ImGui::GetDrawData();
      UpdateImGuiTextures(draw_data);
      bool hasContent = false;
      EmitDrawListsOrtho(draw_data, sdrWhitePQ, depthSort, hasContent, m_overlayUIPool);

      NewOverlayFrame();
   }
}

void LiveUI::UpdateTouchUI()
{
   if (!m_player->m_pininput.HasTouchInput())
      return;

   if (m_player->m_vrDevice)
      return;

   // Always shown when enabled, else during ShowTouchOverlayHint (fading out over its last 2 s)
   // Hidden while a menu is open, as it covers the regions
   const uint64_t now = SDL_GetTicks();
   float opacity = 0.f;
   if (m_inGameUI.IsOpened() || m_editorUI.IsOpened())
      opacity = 0.f;
   else if (m_showTouchOverlay)
   {
      // Full opacity when it shows up (table start, back from a menu), then dimmed to 60% after 10 s, over 1 s (the first-launch hint below has its own fade out)
      if (m_touchOverlayShownMs == 0)
         m_touchOverlayShownMs = now;
      opacity = 1.f - 0.4f * clamp(static_cast<float>(now - m_touchOverlayShownMs) - 10000.f, 0.f, 1000.f) / 1000.f;
   }
   else if (now < m_touchOverlayHintEndMs)
      opacity = min(1.f, static_cast<float>(m_touchOverlayHintEndMs - now) / 2000.f);
   if (opacity <= 0.f)
   {
      m_touchOverlayShownMs = 0;
      return;
   }

   // ImGui coordinates share the orientation of the touch regions (the UI is rotated with the view)
   const ImGuiIO &io = ImGui::GetIO();
   const float screenWidth = io.DisplaySize.x;
   const float screenHeight = io.DisplaySize.y;
   const auto alpha = [opacity](int a) { return static_cast<int>(static_cast<float>(a) * opacity); };
   ImDrawList *drawList = ImGui::GetBackgroundDrawList();
   ImFont *const font = m_overlayBoldFont ? m_overlayBoldFont : ImGui::GetFont();
   // Boxes drawn inset in their regions (the touch area itself is the whole region)
   const float unit = min(screenWidth, screenHeight);
   const float inset = max(6.f, unit * 0.015f);
   const float lineWidth = max(1.5f, unit * 0.0024f);
   for (const auto& region : m_pininput->GetTouchState())
   {
      if (region.label.empty()) // Same area as a labeled region (staged flippers)
         continue;
      const RECT& rect = region.region;
      const ImVec2 topLeft((float)rect.left * screenWidth / 100.0f + inset, (float)rect.top * screenHeight / 100.0f + inset);
      const ImVec2 bottomRight((float)rect.right * screenWidth / 100.0f - inset, (float)rect.bottom * screenHeight / 100.0f - inset);
      const ImVec2 size = bottomRight - topLeft;
      if (size.x <= 0.f || size.y <= 0.f)
         continue;
      const float rounding = min(min(size.x, size.y) * 0.08f, unit * 0.007f); // Capped, so large boxes are not much rounder than small ones

      const bool pressed = m_pininput->IsTouchRegionPressed(region);
      // Light tint only, so the playfield stays visible below (the outline and the label shadow keep the regions readable)
      drawList->AddRectFilled(topLeft, bottomRight, pressed ? IM_COL32(255, 255, 255, alpha(8)) : IM_COL32(0, 0, 0, alpha(52)), rounding);
      drawList->AddRect(topLeft, bottomRight, IM_COL32(255, 255, 255, alpha(pressed ? 105 : 80)), rounding, ImDrawFlags_None, lineWidth);

      // Label centered in the region, scaled down to fit its width
      float fontSize = min(font->LegacySize, size.y * 0.4f);
      ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, region.label.c_str());
      if (textSize.x > size.x * 0.9f)
      {
         fontSize *= size.x * 0.9f / textSize.x;
         textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, region.label.c_str());
      }
      const ImVec2 textPos = topLeft + (size - textSize) * 0.5f;
      const float shadow = max(1.f, fontSize * 0.06f);
      drawList->AddText(font, fontSize, textPos + ImVec2(shadow, shadow), IM_COL32(0, 0, 0, alpha(200)), region.label.c_str());
      drawList->AddText(font, fontSize, textPos, IM_COL32(255, 255, 255, alpha(pressed ? 255 : 220)), region.label.c_str());
   }
}

void LiveUI::OpenInGameUI(const string& page)
{
   HideUI();
   m_inGameUI.Open(page);
}

void LiveUI::HideUI()
{
   m_renderer->InvalidateLayout();
   if (m_inGameUI.IsOpened())
      m_inGameUI.Close();
   m_editorUI.Close();
   g_settingsService.GetActiveSettings().Save();
   g_settingsService.GetAppSettings().Save();
   m_player->SetPlayState(true);
}

extern ImGuiKey ImGui_ImplSDL3_KeyEventToImGuiKey(SDL_Keycode keycode, SDL_Scancode scancode);
ImGuiKey LiveUI::GetImGuiKeyFromSDLScancode(const SDL_Scancode sdlk)
{
   return ImGui_ImplSDL3_KeyEventToImGuiKey(SDL_GetKeyFromScancode(sdlk, SDL_KMOD_NONE, false), sdlk);
}

void LiveUI::CenteredText(const string &text)
{
   const ImVec2 win_size = ImGui::GetWindowSize();
   const ImVec2 text_size = ImGui::CalcTextSize(text.c_str());

   // calculate the indentation that centers the text on one line, relative
   // to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
   float text_indentation = (win_size.x - text_size.x) * 0.5f;

   // if text is too long to be drawn on one line, `text_indentation` can
   // become too small or even negative, so we check a minimum indentation
   constexpr float min_indentation = 20.0f;
   if (text_indentation <= min_indentation)
      text_indentation = min_indentation;

   ImGui::SameLine(text_indentation);
   ImGui::PushTextWrapPos(win_size.x - text_indentation);
   ImGui::TextWrapped("%s", text.c_str());
   ImGui::PopTextWrapPos();
}

void LiveUI::SetupImGuiStyle(const bool isEditor) const
{
   // Theme looking somewhat like Blender's style, based on 'Rounded Visual Studio' style by RedNicStone from ImThemes
   ImGuiStyle &style = ImGui::GetStyle();

   // Reset to defaults to be able to use ScaleAllSize while not defining all style sizes
   ImGuiStyle defaultStyle;
   style = defaultStyle;

   style.Alpha = 1.0f;
   style.DisabledAlpha = 0.6f;
   style.WindowPadding = ImVec2(8.0f, 8.0f);
   style.WindowRounding = 4.0f;
   style.WindowBorderSize = 0.0f;
   style.WindowMinSize = ImVec2(32.0f, 32.0f);
   style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
   style.WindowMenuButtonPosition = ImGuiDir_Left;
   style.ChildRounding = 0.0f;
   style.ChildBorderSize = 1.0f;
   style.PopupRounding = 4.0f;
   style.PopupBorderSize = 1.0f;
   style.FramePadding = ImVec2(4.0f, 3.0f);
   style.FrameRounding = 2.5f;
   style.FrameBorderSize = 0.0f;
   style.ItemSpacing = ImVec2(8.0f, 4.0f);
   style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
   style.CellPadding = ImVec2(4.0f, 2.0f);
   style.IndentSpacing = 21.0f;
   style.ColumnsMinSpacing = 6.0f;
   style.ScrollbarSize = 11.0f;
   style.ScrollbarRounding = 2.5f;
   style.GrabMinSize = 10.0f;
   style.GrabRounding = 2.0f;
   style.TabRounding = 3.5f;
   style.TabBorderSize = 0.0f;
   style.TabCloseButtonMinWidthUnselected = 0.0f;
   style.ColorButtonPosition = ImGuiDir_Right;
   style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
   style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

   ImGui::GetStyle().ScaleAllSizes(m_uiScale);

   style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
   style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.592f, 0.592f, 0.592f, 1.0f);
   style.Colors[ImGuiCol_WindowBg] = isEditor ? ImColor(0xFF363636) : ImColor(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_ChildBg] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_PopupBg] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_Border] = ImVec4(0.306f, 0.306f, 0.306f, 1.0f);
   style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.306f, 0.306f, 0.306f, 1.0f);
   style.Colors[ImGuiCol_FrameBg] = isEditor ? ImColor(0xFF545454) : ImColor(0.2f, 0.2f, 0.216f, 1.0f);
   style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.10f, 0.28f, 0.45f, 1.0f);
   style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.12f, 0.34f, 0.55f, 1.0f);
   style.Colors[ImGuiCol_TitleBg] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2f, 0.2f, 0.216f, 1.0f);
   style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.2f, 0.2f, 0.216f, 1.0f);
   style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.322f, 0.322f, 0.333f, 1.0f);
   style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.353f, 0.353f, 0.373f, 1.0f);
   style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.353f, 0.353f, 0.373f, 1.0f);
   style.Colors[ImGuiCol_CheckMark] = isEditor ? ImColor(0xFFdddddd) : ImColor(0.0f, 0.467f, 0.784f, 1.0f);
   style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.09f, 0.42f, 0.74f, 1.0f);
   style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.11f, 0.46f, 0.78f, 1.0f);
   style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.2f, 0.216f, 1.0f);
   style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.114f, 0.592f, 0.925f, 1.0f);
   style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.114f, 0.592f, 0.925f, 1.0f);
   style.Colors[ImGuiCol_Header] = isEditor ? ImColor(0xFF2B5A8C) : ImColor(0.2f, 0.2f, 0.216f, 1.0f);
   style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.114f, 0.592f, 0.925f, 1.0f);
   style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.467f, 0.784f, 1.0f);
   style.Colors[ImGuiCol_Separator] = ImVec4(0.306f, 0.306f, 0.306f, 1.0f);
   style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.306f, 0.306f, 0.306f, 1.0f);
   style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.306f, 0.306f, 0.306f, 1.0f);
   style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2f, 0.2f, 0.216f, 1.0f);
   style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.322f, 0.322f, 0.333f, 1.0f);
   style.Colors[ImGuiCol_Tab] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_TabHovered] = ImVec4(0.114f, 0.592f, 0.925f, 1.0f);
   style.Colors[ImGuiCol_TabActive] = ImVec4(0.0f, 0.467f, 0.784f, 1.0f);
   style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0f, 0.467f, 0.784f, 1.0f);
   style.Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.467f, 0.784f, 1.0f);
   style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.114f, 0.592f, 0.925f, 1.0f);
   style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.467f, 0.784f, 1.0f);
   style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.114f, 0.592f, 0.925f, 1.0f);
   style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.188f, 0.188f, 0.2f, 1.0f);
   style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.310f, 0.310f, 0.349f, 1.0f);
   style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.227f, 0.227f, 0.247f, 1.0f);
   style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f * 1.0f);
   style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.060f * 1.0f);
   style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.467f, 0.784f, 1.0f);
   style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.145f, 0.145f, 0.149f, 1.0f);
   style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.700f);
   style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800f, 0.800f, 0.800f, 0.20f);
   style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.145f, 0.145f, 0.149f, 0.35f);
}
