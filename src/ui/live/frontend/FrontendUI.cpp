// license:GPLv3+

#include "core/stdafx.h"
#include "FrontendUI.h"

#include "core/player.h"
#include "core/Settings.h"
#include "core/SettingsService.h"
#include "core/extern.h"
#include "core/FileLocator.h"
#include "core/VPApp.h"
#include "parts/pintable.h"
#include "renderer/Sampler.h"
#include "renderer/Texture.h"
#include "renderer/VRDevice.h"
#include "ui/live/IconsLucide.h"
#include "ui/live/ImGuiHelpers.h"
#include "ui/live/ingameui/InGameUIWidgets.h"
#include "ui/live/LiveUI.h"
#include "ui/VPXFileFeedback.h"

#include <fstream>


namespace VPX::Frontend
{

FrontendUI::FrontendUI(LiveUI &liveUI)
   : m_liveUI(liveUI)
{
}

FrontendUI::~FrontendUI()
{
   if (m_auxSubscribed)
   {
      m_msgApi->UnsubscribeMsg(m_auxRendererMsgId, OnGetAuxRenderer, this);
      m_msgApi->BroadcastMsg(m_endpointId, m_auxRendererChgMsgId, nullptr); // Let the next session recollect renderers without us
      m_msgApi->ReleaseMsgID(m_auxRendererMsgId);
      m_msgApi->ReleaseMsgID(m_auxRendererChgMsgId);
   }
   if (m_vpxApi != nullptr)
   {
      for (const auto &[path, texture] : m_auxTextures)
         m_vpxApi->DeleteTexture(texture);
      for (const auto &[path, video] : m_auxVideos)
         m_vpxApi->DeleteTexture(video.texture);
   }
}

void FrontendUI::SetupAuxRenderer()
{
   if (m_auxSubscribed)
      return;
   m_msgApi = &m_player->m_pluginManager.GetMsgAPI();
   m_vpxApi = &m_player->m_pluginAPI.getAPI();
   m_endpointId = m_player->m_pluginAPI.GetVPXEndPointId();
   m_auxRendererMsgId = m_msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER);
   m_auxRendererChgMsgId = m_msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG);
   m_msgApi->SubscribeMsg(m_endpointId, m_auxRendererMsgId, OnGetAuxRenderer, this);
   // The initial renderer collection may have run before we subscribed: ask for a recollect
   m_msgApi->BroadcastMsg(m_endpointId, m_auxRendererChgMsgId, nullptr);
   m_auxSubscribed = true;
}

void FrontendUI::OnGetAuxRenderer(const unsigned int msgId, void *userData, void *msgData)
{
   auto *msg = static_cast<GetAncillaryRendererMsg *>(msgData);
   if (msg->window != VPXWindowId::VPXWINDOW_Backglass && msg->window != VPXWindowId::VPXWINDOW_ScoreView && msg->window != VPXWindowId::VPXWINDOW_Topper)
      return;
   AncillaryRendererDef entry { "VPX.Frontend", "Frontend", "Mini frontend table media", userData, &FrontendUI::RenderAncillary };
   if (msg->count < msg->maxEntryCount)
      msg->entries[msg->count] = entry;
   msg->count++;
}

int MSGPIAPI FrontendUI::RenderAncillary(VPXRenderContext2D *ctx, void *context)
{
   auto *me = static_cast<FrontendUI *>(context);
   const auto &entries = me->m_catalog.GetEntries();
   if (entries.empty() || me->m_selected < 0 || me->m_selected >= static_cast<int>(entries.size()))
      return 0;
   const TableCatalog::Entry &entry = entries[me->m_selected];
   VPXTexture texture = nullptr;
   switch (ctx->window)
   {
   case VPXWindowId::VPXWINDOW_Backglass:
      if (entry.HasMedia(TableCatalog::MediaType::Backglass))
         texture = me->GetAuxMedia(entry.GetMedia(TableCatalog::MediaType::Backglass));
      break;
   case VPXWindowId::VPXWINDOW_ScoreView:
      if (entry.HasMedia(TableCatalog::MediaType::FullDMD))
         texture = me->GetAuxMedia(entry.GetMedia(TableCatalog::MediaType::FullDMD));
      else if (entry.HasMedia(TableCatalog::MediaType::DMD))
         texture = me->GetAuxMedia(entry.GetMedia(TableCatalog::MediaType::DMD));
      break;
   case VPXWindowId::VPXWINDOW_Topper:
      if (entry.HasMedia(TableCatalog::MediaType::Topper))
         texture = me->GetAuxMedia(entry.GetMedia(TableCatalog::MediaType::Topper));
      break;
   default: break;
   }
   if (texture == nullptr)
      return 0;
   VPXTextureInfo *const info = me->m_vpxApi->GetTextureInfo(texture);
   if (info == nullptr)
      return 0;
   ctx->DrawImage(ctx, texture, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, static_cast<float>(info->width), static_cast<float>(info->height), 0.f, 0.f, 0.f, 0.f, 0.f, ctx->srcWidth, ctx->srcHeight);
   return 1;
}

VPXTexture FrontendUI::GetAuxMedia(const std::filesystem::path &path) { return TableCatalog::IsAnimatedMediaFile(path) ? GetAuxVideoTexture(path) : GetAuxTexture(path); }

// Video medias on ancillary windows: the decode session publishes its latest frame as a bitmap
// which is copied into the window's texture; the sequence check skips unchanged uploads
VPXTexture FrontendUI::GetAuxVideoTexture(const std::filesystem::path &path)
{
   if (m_media == nullptr)
      return nullptr;
   uint64_t seq = 0;
   const std::shared_ptr<const BaseTexture> frame = m_media->GetVideoFrame(path, &seq);
   if (frame == nullptr)
      return nullptr;
   std::lock_guard lock(m_auxTexMutex);
   AuxVideo &slot = m_auxVideos[path];
   if (slot.seq != seq)
   {
      m_vpxApi->UpdateTexture(&slot.texture, static_cast<int>(frame->width()), static_cast<int>(frame->height()), VPXTextureFormat::VPXTEXFMT_sRGBA8, frame->datac());
      slot.seq = seq;
   }
   return slot.texture;
}

VPXTexture FrontendUI::GetAuxTexture(const std::filesystem::path &path)
{
   std::lock_guard lock(m_auxTexMutex);
   if (const auto it = m_auxTextures.find(path); it != m_auxTextures.end())
      return it->second;
   std::ifstream file(path, std::ios::binary | std::ios::ate);
   if (!file.is_open())
      return nullptr;
   const std::streamsize size = file.tellg();
   vector<uint8_t> data(static_cast<size_t>(size));
   file.seekg(0);
   if (!file.read(reinterpret_cast<char *>(data.data()), size))
      return nullptr;
   VPXTexture texture = m_vpxApi->CreateTexture(data.data(), static_cast<int>(size));
   return m_auxTextures.emplace(path, texture).first->second;
}

void FrontendUI::Rescan()
{
   vector<std::filesystem::path> roots;
   const string &folders = g_settingsService.GetAppSettings().GetFrontend_TableFolders();
   size_t start = 0;
   while (start <= folders.size())
   {
      const size_t end = folders.find(';', start);
      const string folder = folders.substr(start, end == string::npos ? string::npos : end - start);
      if (!folder.empty())
         roots.emplace_back(folder);
      if (end == string::npos)
         break;
      start = end + 1;
   }
   m_catalog.Scan(roots);
   m_selected = std::clamp(m_selected, 0, std::max(0, static_cast<int>(m_catalog.GetEntries().size()) - 1));
   m_scanned = true;
}

void FrontendUI::LaunchSession(const Player::PlayMode playMode)
{
   const auto &entries = m_catalog.GetEntries();
   if (entries.empty() || m_selected < 0 || m_selected >= static_cast<int>(entries.size()))
      return;

   CComObject<PinTable> *table;
   CComObject<PinTable>::CreateInstance(&table);
   table->AddRef();
   VPXFileFeedback feedback;
   const HRESULT hr = table->LoadGameFromFilename(entries[m_selected].tablePath, feedback);
   if (FAILED(hr) && (hr != APPX_E_BLOCK_HASH_INVALID) && (hr != APPX_E_CORRUPT_CONTENT))
   {
      table->Release();
      m_liveUI.PushNotification(std::format("Failed to load table '{}'", entries[m_selected].title), 4000);
      return;
   }

   // Keep the VR state when playing from an XR frontend; edit sessions stay on the desktop as the
   // live editor is not available inside the headset
   const Player::VROverride vr = m_player->IsVR() && playMode == Player::PlayMode::Play ? Player::VROverride::ForceOn : Player::VROverride::Auto;
   m_player->RequestSessionRestart(table, playMode, vr, false);
}

bool FrontendUI::OnNavigateBack()
{
   return false;
}

bool FrontendUI::NavTriggered(const bool pressed, uint32_t &heldSince, uint32_t &lastTrigger)
{
   const uint32_t now = msec();
   if (!pressed)
   {
      heldSince = 0;
      return false;
   }
   if (heldSince == 0)
   {
      heldSince = now;
      lastTrigger = now;
      return true;
   }
   const uint32_t repeat = (now - heldSince) < 500 ? 500 : 150;
   if (now - lastTrigger >= repeat)
   {
      lastTrigger = now;
      return true;
   }
   return false;
}

void FrontendUI::ApplyWheelDrag(const float deltaX, const float pxPerCell)
{
   m_wheelDragging = true;
   m_wheelPos -= deltaX / pxPerCell;
   // Smoothed release velocity for the inertial scroll, bounded to keep a violent flick sane
   const float v = -deltaX / pxPerCell / std::max(ImGui::GetIO().DeltaTime, 1e-4f);
   m_wheelVel = std::clamp(m_wheelVel * 0.7f + v * 0.3f, -20.f, 20.f);
   m_wheelNavDir = 0; // A drag cancels a held navigation scroll
   m_wheelNavSince = 0;
}

void FrontendUI::HandleInput()
{
   // The in-game UI chrome owns the input while it is open over the wheel
   if (m_liveUI.IsInGameUIOpened())
   {
      m_wheelNavDir = m_wheelNavSince = 0;
      return;
   }
   InputManager &input = m_player->m_pininput;
   const int count = static_cast<int>(m_catalog.GetEntries().size());

   // Navigation: keyboard arrows, cabinet flippers and UI nav actions. A press steps the
   // selection one entry; held past a short delay the wheel scrolls continuously, its speed
   // ramping up with the hold duration (integrated in DrawThemedLayout with the wheel position)
   const bool prevHeld = input.IsPressed(input.GetUILeftActionId()) || input.IsPressed(input.GetUIUpActionId()) || input.IsPressed(input.GetLeftFlipperActionId())
      || ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_UpArrow);
   const bool nextHeld = input.IsPressed(input.GetUIRightActionId()) || input.IsPressed(input.GetUIDownActionId()) || input.IsPressed(input.GetRightFlipperActionId())
      || ImGui::IsKeyDown(ImGuiKey_RightArrow) || ImGui::IsKeyDown(ImGuiKey_DownArrow);
   const int navDir = (nextHeld ? 1 : 0) - (prevHeld ? 1 : 0);
   if (navDir != m_wheelNavDir)
   {
      m_wheelNavDir = navDir;
      m_wheelNavSince = navDir != 0 ? msec() : 0;
      if (count > 0 && navDir != 0)
         m_selected = ((m_selected + navDir) % count + count) % count;
   }

   // Start (or Enter/Space) opens the table actions view, the launch ball button
   // starts the game directly on the selected table
   const bool tablePressed = input.IsPressed(input.GetStartActionId()) || ImGui::IsKeyDown(ImGuiKey_Enter) || ImGui::IsKeyDown(ImGuiKey_KeypadEnter) || ImGui::IsKeyDown(ImGuiKey_Space);
   const bool launchPressed = input.IsPressed(input.GetLaunchBallActionId());
   if (tablePressed && !m_confirmWasPressed)
      OpenActionsView();
   if (launchPressed && !m_launchWasPressed)
      LaunchSession(Player::PlayMode::Play);
   m_confirmWasPressed = tablePressed;
   m_launchWasPressed = launchPressed;

   // Magna buttons jump by one letter group in the catalog
   const bool magnaPrevPressed = input.IsPressed(input.GetLeftMagnaActionId());
   const bool magnaNextPressed = input.IsPressed(input.GetRightMagnaActionId());
   if (magnaPrevPressed && !m_leftMagnaWasPressed)
      JumpLetter(-1);
   if (magnaNextPressed && !m_rightMagnaWasPressed)
      JumpLetter(1);
   m_leftMagnaWasPressed = magnaPrevPressed;
   m_rightMagnaWasPressed = magnaNextPressed;
}

void FrontendUI::JumpLetter(const int dir)
{
   const auto &entries = m_catalog.GetEntries();
   const int count = static_cast<int>(entries.size());
   if (count == 0)
      return;
   const auto letterOf = [&entries](const int index)
   {
      const string &title = entries[index].title;
      return title.empty() ? ' ' : lowerCase(title)[0];
   };
   const char current = letterOf(m_selected);
   int target = m_selected;
   do
      target = ((target + dir) % count + count) % count;
   while (target != m_selected && letterOf(target) == current);
   if (letterOf(target) == current)
      return; // A single letter group: nowhere to jump
   // A backward jump lands on the last entry of the previous group: walk to its first
   while (letterOf((target + dir + count) % count) == letterOf(target))
      target = (target + dir + count) % count;
   m_selected = target;
   m_wheelPos = static_cast<float>(target);
}

void FrontendUI::OpenActionsView()
{
   // The in-game UI opens on the frontend's navigation root (the chrome's table of contents)
   m_liveUI.OpenInGameUI();
}

bool FrontendUI::HasSelection() const { return m_selected >= 0 && m_selected < static_cast<int>(m_catalog.GetEntries().size()); }

const TableCatalog::Entry *FrontendUI::GetSelected() const { return HasSelection() ? &m_catalog.GetEntries()[m_selected] : nullptr; }

void FrontendUI::RestartFrontendSession(const Player::VROverride vr)
{
   PinTable *const table = m_player->m_ptable;
   table->AddRef(); // The session switch takes an owned reference
   m_player->RequestSessionRestart(table, Player::PlayMode::Frontend, vr, false);
}

void FrontendUI::DrawImage(const std::shared_ptr<Sampler> &image, const ImVec2 &boxPos, const ImVec2 &boxSize, const float alpha, const bool autoRotate) const
{
   // Landscape media is shown rotated by 270 degrees clockwise, portrait media as is
   const bool rotate = autoRotate && image->GetWidth() > image->GetHeight();
   const float imgAspect = rotate ? static_cast<float>(image->GetHeight()) / static_cast<float>(image->GetWidth())
                                  : static_cast<float>(image->GetWidth()) / static_cast<float>(image->GetHeight());
   const float boxAspect = boxSize.x / boxSize.y;
   const ImVec2 size = (imgAspect >= boxAspect) ? ImVec2(boxSize.x, boxSize.x / imgAspect) : ImVec2(boxSize.y * imgAspect, boxSize.y);
   const ImVec2 pos(boxPos.x + (boxSize.x - size.x) * 0.5f, boxPos.y + (boxSize.y - size.y) * 0.5f);
   const ImU32 col = IM_COL32(255, 255, 255, static_cast<int>(alpha * 255.f));
   if (rotate)
      ImGui::GetWindowDrawList()->AddImageQuad(image, pos, ImVec2(pos.x + size.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y), ImVec2(pos.x, pos.y + size.y),
         ImVec2(1.f, 0.f), ImVec2(1.f, 1.f), ImVec2(0.f, 1.f), ImVec2(0.f, 0.f), col);
   else
      ImGui::GetWindowDrawList()->AddImage(image, pos, ImVec2(pos.x + size.x, pos.y + size.y), ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), col);
}

void FrontendUI::DrawImageCover(const std::shared_ptr<Sampler> &image, const ImVec2 &boxPos, const ImVec2 &boxSize, const float alpha, const bool autoRotate) const
{
   // Same rotation rule as DrawImage: landscape media is turned by 270 degrees clockwise
   const bool rotate = autoRotate && image->GetWidth() > image->GetHeight();
   const float imgAspect = rotate ? static_cast<float>(image->GetHeight()) / static_cast<float>(image->GetWidth())
                                  : static_cast<float>(image->GetWidth()) / static_cast<float>(image->GetHeight());
   const float boxAspect = boxSize.x / boxSize.y;
   const ImVec2 size = (imgAspect >= boxAspect) ? ImVec2(boxSize.y * imgAspect, boxSize.y) : ImVec2(boxSize.x, boxSize.x / imgAspect);
   const ImVec2 pos(boxPos.x + (boxSize.x - size.x) * 0.5f, boxPos.y + (boxSize.y - size.y) * 0.5f);
   const ImU32 col = IM_COL32(255, 255, 255, static_cast<int>(alpha * 255.f));
   ImDrawList *const drawList = ImGui::GetWindowDrawList();
   drawList->PushClipRect(boxPos, ImVec2(boxPos.x + boxSize.x, boxPos.y + boxSize.y), true);
   if (rotate)
      drawList->AddImageQuad(image, pos, ImVec2(pos.x + size.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y), ImVec2(pos.x, pos.y + size.y),
         ImVec2(1.f, 0.f), ImVec2(1.f, 1.f), ImVec2(0.f, 1.f), ImVec2(0.f, 0.f), col);
   else
      drawList->AddImage(image, pos, ImVec2(pos.x + size.x, pos.y + size.y), ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), col);
   drawList->PopClipRect();
}

void FrontendUI::DrawBackground(const TableCatalog::Entry *const selected) const
{
   if (selected == nullptr)
      return;
   std::filesystem::path key;
   bool rotate = false;
   std::shared_ptr<Sampler> image;
   if (selected->HasMedia(TableCatalog::MediaType::Backglass))
   {
      key = selected->GetMedia(TableCatalog::MediaType::Backglass).wstring() + L".blur";
      image = m_media->GetBlurred(*selected, TableCatalog::MediaType::Backglass);
   }
   if (image == nullptr && !selected->HasMedia(TableCatalog::MediaType::Backglass) && selected->HasMedia(TableCatalog::MediaType::Playfield))
   {
      key = selected->GetMedia(TableCatalog::MediaType::Playfield).wstring() + L".blur";
      image = m_media->GetBlurred(*selected, TableCatalog::MediaType::Playfield);
      rotate = true;
   }
   if (image != nullptr)
   {
      const float fadeIn = std::clamp(static_cast<float>(msec() - m_media->GetReadyTime(key)) / 300.f, 0.f, 1.f);
      DrawImageCover(image, ImVec2(0.f, 0.f), ImGui::GetIO().DisplaySize, fadeIn, rotate);
   }
}

void FrontendUI::DrawPreviewImage(PreviewPane &pane, const TableCatalog::Entry *const selected, const TableCatalog::MediaType type, const ImVec2 &pos,
   const ImVec2 &size, const bool autoRotate)
{
   // Keep the current image while the target one is still being decoded, then crossfade to it
   const std::filesystem::path want = (selected != nullptr && selected->HasMedia(type)) ? selected->GetMedia(type) : std::filesystem::path {};
   DrawPreviewImage(pane, want, pos, size, autoRotate);
}

void FrontendUI::DrawPreviewImage(PreviewPane &pane, const std::filesystem::path &want, const ImVec2 &pos, const ImVec2 &size, const bool autoRotate)
{
   // The wanted media is requested on every frame, not only while it is being decoded: for
   // videos this keeps the decode session alive and the (persistent) sampler animates in place
   std::shared_ptr<Sampler> image;
   if (!want.empty())
      image = m_media->Get(want);
   if (want != pane.mediaPath)
   {
      if (image != nullptr || want.empty())
      {
         pane.previous = pane.current;
         pane.current = image;
         pane.mediaPath = want;
         pane.fadeStart = msec();
      }
   }
   const float t = std::clamp(static_cast<float>(msec() - pane.fadeStart) / 250.f, 0.f, 1.f);
   if (pane.previous != nullptr && t < 1.f)
      DrawImage(pane.previous, pos, size, 1.f - t, autoRotate);
   if (pane.current != nullptr)
      DrawImage(pane.current, pos, size, t, autoRotate);
}

void FrontendUI::Update()
{
   if (m_player == nullptr)
      m_player = g_pplayer;
   if (m_media == nullptr)
      m_media = std::make_unique<FrontendMedia>(m_player->m_renderer->m_renderDevice);
   SetupAuxRenderer();
   if (!m_scanned)
      Rescan();
   m_media->BeginFrame();

   UpdateTheme();

#ifdef ENABLE_XR
   // Headset probe, the same detection the player performs on session start: OpenXR runtime
   // present (instance created in VRDevice ctor) then a system for the HMD form factor. It is
   // retried every few seconds while unavailable so a headset connected or a runtime started
   // after the frontend is shown still enables the VR switch.
   if (!m_vrAvailable && !m_player->IsVR() && msec() >= m_vrProbeAt)
   {
      m_vrProbeAt = msec() + 3000;
      VRDevice probe(m_player->m_ptable->GetSettings());
      if (probe.IsOpenXRReady())
      {
         probe.SetupHMD();
         m_vrAvailable = probe.IsOpenXRHMDReady();
      }
   }
#endif

   HandleInput();

   const ImGuiIO &io = ImGui::GetIO();
   const auto &entries = m_catalog.GetEntries();
   const int count = static_cast<int>(entries.size());

   ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
   ImGui::SetNextWindowSize(io.DisplaySize);
   ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.02f, 0.02f, 0.04f, 1.f));
   ImGui::Begin(
      "##Frontend", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus);

   const TableCatalog::Entry *const selected = count > 0 ? &entries[m_selected] : nullptr;

   DrawThemedLayout(selected);

#ifdef ENABLE_XR
   // Direct switch to VR, the same action as the stereo toggle: a clickable button at the top
   // center of the screen, only in desktop mode (there is no mouse in VR; exiting is done from
   // the stereo toggle or the table actions page), when a VR session can actually run (runtime
   // and HMD probed above)
   if (m_vrAvailable && !m_player->IsVR() && !m_liveUI.IsInGameUIOpened())
   {
      const ImVec2 size = IconButtonInkSize(ICON_LC_RECTANGLE_GOGGLES);
      ImGui::SetCursorPos(ImVec2((io.DisplaySize.x - size.x) * 0.5f, 8.f * m_liveUI.GetDPI()));
      if (IconButtonInk(ICON_LC_RECTANGLE_GOGGLES, "##ToggleVR"))
         RestartFrontendSession(Player::VROverride::ForceOn);
      if (ImGui::IsItemHovered())
         ImGui::SetTooltip("Enter VR");
   }
#endif
   ImGui::End();
   ImGui::PopStyleColor();
}


FrontendUI::Presentation FrontendUI::ResolvePresentation() const
{
   const int bgSet = g_settingsService.GetActiveSettings().GetPlayer_BGSet();
   if (bgSet == 1)
      return Presentation::Cabinet;
   const ImGuiIO &io = ImGui::GetIO();
   // FSS is forced or auto-detected on portrait displays (phones, rotated screens); in XR the
   // frontend is a head locked quad layer in the HMD, where the full single screen layout fits best
   if (bgSet == 2 || io.DisplaySize.y > io.DisplaySize.x || m_player->IsVR())
      return Presentation::FSS;
   return Presentation::Desktop;
}

void FrontendUI::UpdateTheme()
{
   const ImGuiIO &io = ImGui::GetIO();
   const float aspect = io.DisplaySize.x / std::max(1.f, io.DisplaySize.y);
   const Settings &app = g_settingsService.GetAppSettings();
   const string key = app.GetFrontend_Theme() + '|' + app.GetFrontend_ThemeFolders() + '|' + std::format("{:.3f}", aspect) + '|' + std::to_string(static_cast<int>(ResolvePresentation()));
   if (key == m_themeKey)
      return;
   m_themeKey = key;

   // Theme roots: the bundled application assets first, then the user configured folders
   vector<std::filesystem::path> roots { g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets) / "frontend"s };
   const string &folders = app.GetFrontend_ThemeFolders();
   size_t start = 0;
   while (start <= folders.size())
   {
      const size_t end = folders.find(';', start);
      const string folder = folders.substr(start, end == string::npos ? string::npos : end - start);
      if (!folder.empty())
         roots.emplace_back(folder);
      if (end == string::npos)
         break;
      start = end + 1;
   }
   m_themes = FrontendTheme::Discover(roots);

   const auto it = std::ranges::find_if(m_themes, [&](const FrontendTheme::Entry &e) { return e.name == app.GetFrontend_Theme(); });
   const FrontendTheme::Entry *const theme = it != m_themes.end() ? &*it : (m_themes.empty() ? nullptr : &m_themes.front());

   m_themeLayout = FrontendTheme::Layout {};
   m_themeUnderlay.reset();
   m_themeOverlay.reset();
   m_themeWheelEmpty.reset();
   m_themeUnderlayPath.clear();
   m_themeOverlayPath.clear();
   m_themeWheelEmptyPath.clear();

   if (theme != nullptr)
   {
      if (const std::filesystem::path layoutPath = FrontendTheme::SelectVariant(theme->folder, "layout"s, aspect); !layoutPath.empty())
         if (const std::shared_ptr<BaseTexture> img = BaseTexture::CreateFromFile(layoutPath); img != nullptr)
         {
            m_themeLayout = FrontendTheme::ParseLayout(*img);
            m_themeImgW = static_cast<int>(img->width());
            m_themeImgH = static_cast<int>(img->height());
            PLOGI << "Frontend theme '" << theme->name << "' loaded from " << PathToUTF8(layoutPath);
         }
      // Theme underlay/overlay are drawn covering the theme layout fitted to the display:
      // request the decode bound that matches, they would else be limited to media cell size
      const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
      const unsigned int dispW = static_cast<unsigned int>(displaySize.x), dispH = static_cast<unsigned int>(displaySize.y);
      if (const std::filesystem::path underlay = FrontendTheme::SelectVariant(theme->folder, "underlay"s, aspect, true); !underlay.empty())
      {
         m_themeUnderlayPath = underlay;
         m_themeUnderlay = m_media->Get(underlay, dispW, dispH);
      }
      if (const std::filesystem::path overlay = FrontendTheme::SelectVariant(theme->folder, "overlay"s, aspect, true); !overlay.empty())
      {
         m_themeOverlayPath = overlay;
         m_themeOverlay = m_media->Get(overlay, dispW, dispH);
      }
   }

   // Wheel placeholder for cells without media: theme specific variant first, then the shared
   // 'wheel-empty.webp' sitting next to the theme folders
   if (theme != nullptr)
      m_themeWheelEmptyPath = FrontendTheme::SelectVariant(theme->folder, "wheel-empty"s, aspect, true);
   for (const std::filesystem::path &root : roots)
      if (m_themeWheelEmptyPath.empty())
         m_themeWheelEmptyPath = FrontendTheme::SelectVariant(root, "wheel-empty"s, aspect, true);
   if (!m_themeWheelEmptyPath.empty())
      m_themeWheelEmpty = m_media->Get(m_themeWheelEmptyPath);

   // No usable layout map (no theme or no zones parsed): generate a default one matching the
   // presentation, so the whole UI always renders through the same themed code path
   const FrontendTheme::Layout &l = m_themeLayout;
   if (!(l.playfield.valid || l.backglass.valid || l.card.valid || l.menuBand.valid || l.infoBand.valid || l.infoPane.valid || l.wheelFirstColumn >= 0))
      MakeDefaultLayout();
}

void FrontendUI::MakeDefaultLayout()
{
   const ImGuiIO &io = ImGui::GetIO();
   m_themeImgW = std::max(1, static_cast<int>(io.DisplaySize.x));
   m_themeImgH = std::max(1, static_cast<int>(io.DisplaySize.y));

   FrontendTheme::Layout layout;
   const auto region = [](const float x0, const float y0, const float x1, const float y1)
   {
      FrontendTheme::Region r;
      r.xMin = x0, r.yMin = y0, r.xMax = x1, r.yMax = y1;
      r.valid = true;
      return r;
   };
   layout.wheelFirstColumn = 0;
   layout.wheelLastColumn = m_themeImgW - 1;
   layout.wheelColumns.assign(m_themeImgW, { 0.80f, 0.94f });
   switch (ResolvePresentation())
   {
   case Presentation::Cabinet:
      // The playfield media fills the whole window; other displays are fed through the ancillary renderer
      layout.playfield = region(0.f, 0.f, 1.f, 1.f);
      layout.infoPane = region(0.56f, 0.06f, 0.99f, 0.16f);
      break;
   case Presentation::FSS:
      // Single screen: backglass stacked above the projected playfield
      layout.backglass = region(0.01f, 0.06f, 0.60f, 0.34f);
      layout.infoPane = region(0.62f, 0.06f, 0.99f, 0.34f);
      layout.playfield = region(0.01f, 0.36f, 0.99f, 0.78f);
      break;
   default:
      // Desktop: playfield centered, backglass top left, table info top right
      layout.backglass = region(0.01f, 0.06f, 0.26f, 0.15f);
      layout.infoPane = region(0.55f, 0.06f, 0.99f, 0.15f);
      layout.playfield = region(0.01f, 0.17f, 0.99f, 0.78f);
      break;
   }
   m_themeLayout = layout;
}

void FrontendUI::DrawThemedLayout(const TableCatalog::Entry *const selected)
{
   const ImGuiIO &io = ImGui::GetIO();
   ImDrawList *const drawList = ImGui::GetWindowDrawList();
   const float uiScale = m_liveUI.GetDPI();
   float scale, offsetX, offsetY;
   FrontendTheme::ComputeFit(m_themeImgW, m_themeImgH, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y), scale, offsetX, offsetY);
   const float imgW = m_themeImgW * scale, imgH = m_themeImgH * scale;

   // Background: the backglass (or playfield) cover fitted and blurred, under the theme's underlay
   DrawBackground(selected);

   // Theme media are requested on every frame: the fetch retries while they decode, and for
   // videos it keeps the decode session alive so the theme keeps animating
   if (!m_themeUnderlayPath.empty())
      if (const std::shared_ptr<Sampler> sampler = m_media->Get(m_themeUnderlayPath, static_cast<unsigned int>(io.DisplaySize.x), static_cast<unsigned int>(io.DisplaySize.y)))
         m_themeUnderlay = sampler;
   if (!m_themeOverlayPath.empty())
      if (const std::shared_ptr<Sampler> sampler = m_media->Get(m_themeOverlayPath, static_cast<unsigned int>(io.DisplaySize.x), static_cast<unsigned int>(io.DisplaySize.y)))
         m_themeOverlay = sampler;
   if (!m_themeWheelEmptyPath.empty())
      if (const std::shared_ptr<Sampler> sampler = m_media->Get(m_themeWheelEmptyPath))
         m_themeWheelEmpty = sampler;
   if (m_themeUnderlay != nullptr)
   {
      const float fade = std::clamp(static_cast<float>(msec() - m_media->GetReadyTime(m_themeUnderlayPath)) / 300.f, 0.f, 1.f);
      drawList->AddImage(m_themeUnderlay, ImVec2(offsetX, offsetY), ImVec2(offsetX + imgW, offsetY + imgH), ImVec2(0.f, 0.f), ImVec2(1.f, 1.f),
         IM_COL32(255, 255, 255, static_cast<int>(fade * 255.f)));
   }

   const auto mapRegion = [&](const FrontendTheme::Region &r, ImVec2 &pos, ImVec2 &size)
   {
      pos = ImVec2(offsetX + r.xMin * imgW, offsetY + r.yMin * imgH);
      size = ImVec2((r.xMax - r.xMin) * imgW, (r.yMax - r.yMin) * imgH);
   };

   ImVec2 zonePos, zoneSize;
   if (m_themeLayout.playfield.valid)
   {
      mapRegion(m_themeLayout.playfield, zonePos, zoneSize);
      // Fall back to the backglass media when the table has no playfield image
      const std::filesystem::path want = (selected != nullptr && selected->HasMedia(TableCatalog::MediaType::Playfield))
         ? selected->GetMedia(TableCatalog::MediaType::Playfield)
         : ((selected != nullptr && selected->HasMedia(TableCatalog::MediaType::Backglass)) ? selected->GetMedia(TableCatalog::MediaType::Backglass) : std::filesystem::path {});
      DrawPreviewImage(m_panes[0], want, zonePos, zoneSize, true);
   }
   if (m_themeLayout.backglass.valid)
   {
      mapRegion(m_themeLayout.backglass, zonePos, zoneSize);
      DrawPreviewImage(m_panes[1], selected, TableCatalog::MediaType::Backglass, zonePos, zoneSize);
   }
   if (m_themeLayout.card.valid)
   {
      mapRegion(m_themeLayout.card, zonePos, zoneSize);
      DrawPreviewImage(m_panes[2], selected, TableCatalog::MediaType::GameHelp, zonePos, zoneSize);
   }
   if (m_themeLayout.infoPane.valid)
   {
      mapRegion(m_themeLayout.infoPane, zonePos, zoneSize);
      DrawInfoPane(selected, zonePos, zoneSize);
   }

   // The theme overlay sits between the media elements and the interactive ones (wheel, tabs, help)
   if (m_themeOverlay != nullptr)
   {
      const float overlayFade = std::clamp(static_cast<float>(msec() - m_media->GetReadyTime(m_themeOverlayPath)) / 300.f, 0.f, 1.f);
      drawList->AddImage(m_themeOverlay, ImVec2(offsetX, offsetY), ImVec2(offsetX + imgW, offsetY + imgH), ImVec2(0.f, 0.f), ImVec2(1.f, 1.f),
         IM_COL32(255, 255, 255, static_cast<int>(overlayFade * 255.f)));
   }

   // Table wheel inside the red band: unframed items sliding smoothly along the band. Each item is
   // centered horizontally at its (fractional) slot and scaled so its height fits the band's
   // interpolated Y bounds at that column, its middle resting on the band's center line
   const auto &entries = m_catalog.GetEntries();
   const int count = static_cast<int>(entries.size());
   const FrontendTheme::Layout &layout = m_themeLayout;
   if (count > 0)
   {
      // Held navigation scrolls the wheel continuously at a speed ramping up with the hold
      // duration, the selection following the position so the centered entry stays in sync.
      // Otherwise the position eases toward the selection, taking the shortest wrap around path
      m_wheelPos = fmodf(fmodf(m_wheelPos, static_cast<float>(count)) + static_cast<float>(count), static_cast<float>(count));
      const uint32_t navHeldFor = m_wheelNavDir != 0 ? msec() - m_wheelNavSince : 0;
      if (m_wheelDragging)
      {
         // The position was dragged by the pointer; the selection tracks the centered entry
         m_selected = ((static_cast<int>(roundf(m_wheelPos)) % count) + count) % count;
      }
      else if (fabsf(m_wheelVel) > 0.f)
      {
         // Inertial scroll after a flick release: exponential decay, then the ease snaps to a cell
         m_wheelPos += m_wheelVel * io.DeltaTime;
         m_wheelVel *= std::exp(-6.f * io.DeltaTime);
         if (fabsf(m_wheelVel) < 0.4f)
            m_wheelVel = 0.f;
         m_selected = ((static_cast<int>(roundf(m_wheelPos)) % count) + count) % count;
      }
      else if (navHeldFor > 400)
      {
         const float ramp = std::min(1.f, (navHeldFor - 400) / 1600.f);
         m_wheelPos += m_wheelNavDir * (2.5f + 9.5f * ramp * ramp) * io.DeltaTime;
         m_selected = ((static_cast<int>(roundf(m_wheelPos)) % count) + count) % count;
      }
      else
      {
         const float delta = fmodf(m_selected - m_wheelPos + count + count * 0.5f, static_cast<float>(count)) - count * 0.5f;
         m_wheelPos += delta * std::min(1.f, io.DeltaTime * 8.f);
      }
   }
   if (count > 0 && layout.wheelFirstColumn >= 0)
   {
      const int firstCol = layout.wheelFirstColumn, lastCol = layout.wheelLastColumn;
      const float bandCenter = (firstCol + lastCol + 1) * 0.5f;
      const float colsPerCell = (lastCol + 1 - firstCol) / 7.f;
      const float pxPerCell = colsPerCell * scale;
      const bool wheelInteractive = !m_liveUI.IsInGameUIOpened() && m_tableLoad == nullptr;

      m_wheelDragging = false;
      if (wheelInteractive)
      {
         // The whole screen is a drag surface for the wheel (flick scroll on touch, drag scroll
         // with a mouse). Overlappable: without it, the first submitted item under the cursor
         // wins the hover and every cell and chrome button beneath it goes dead
         ImGui::SetNextItemAllowOverlap();
         ImGui::SetCursorScreenPos(ImVec2(0.f, 0.f));
         ImGui::InvisibleButton("##wheelDrag", io.DisplaySize);
         if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 4.f * uiScale))
            ApplyWheelDrag(io.MouseDelta.x, pxPerCell);

         // The mouse wheel steps the selection one entry per notch, the position easing to it like
         // with keyboard navigation. Fractional trackpad deltas accumulate into steps
         if (!m_wheelDragging && ImGui::IsWindowHovered() && io.MouseWheel != 0.f)
         {
            m_wheelVel = 0.f; // A wheel scroll cancels an in-flight flick
            m_wheelWheelAccum += io.MouseWheel;
            while (m_wheelWheelAccum >= 1.f)
            {
               m_selected = (m_selected - 1 + count) % count;
               m_wheelWheelAccum -= 1.f;
            }
            while (m_wheelWheelAccum <= -1.f)
            {
               m_selected = (m_selected + 1) % count;
               m_wheelWheelAccum += 1.f;
            }
         }
      }


      const int kMin = -std::min(4, (count - 1) / 2), kMax = std::min(4, count / 2);
      drawList->PushClipRect(ImVec2(offsetX + firstCol * scale, 0.f), ImVec2(offsetX + (lastCol + 1) * scale, io.DisplaySize.y), true);
      // Farthest items first so the centered item draws on top
      for (int d = std::max(-kMin, kMax); d >= 0; d--)
         for (const int k : (d == 0 ? std::initializer_list<int> { 0 } : std::initializer_list<int> { d, -d }))
         {
            if (k < kMin || k > kMax)
               continue;
            const int index = ((m_selected + k) % count + count) % count;
            // Fractional slot offset on the circle: the eased position may sit just below 0 when the
            // selection wraps from the first to the last entry
            const float o = fmodf(static_cast<float>(m_selected + k) - m_wheelPos + count + count * 0.5f, static_cast<float>(count)) - count * 0.5f;
            const float alpha = std::clamp(3.6f - fabsf(o), 0.f, 1.f); // fade out at the band edges
            if (alpha <= 0.f)
               continue;
            const float centerCol = bandCenter + o * colsPerCell;
            const int c = std::clamp(static_cast<int>(floorf(centerCol)), firstCol, lastCol);
            const float yMin = layout.wheelColumns[c].first, yMax = layout.wheelColumns[c].second;
            if (yMax <= yMin)
               continue;
            const float h = (yMax - yMin) * imgH;
            const float cy = offsetY + (yMin + yMax) * 0.5f * imgH;
            const float cx = offsetX + centerCol * scale;

            const auto &entry = entries[index];
            const std::filesystem::path mediaPath = entry.HasMedia(TableCatalog::MediaType::Wheel) ? entry.GetMedia(TableCatalog::MediaType::Wheel) : std::filesystem::path {};
            const std::shared_ptr<Sampler> image = mediaPath.empty() ? nullptr : m_media->Get(mediaPath);
            // The wheel art fades in once decoded; the shared wheel-empty placeholder carries the
            // table name for media-less entries and stands in until the art is fully faded in
            const float artAlpha = image != nullptr ? std::clamp(static_cast<float>(msec() - m_media->GetReadyTime(mediaPath)) / 300.f, 0.f, 1.f) : 0.f;
            ImVec2 cellSize(h, h);
            if (m_themeWheelEmpty != nullptr && (image == nullptr || artAlpha < 1.f))
            {
               cellSize.x = h * static_cast<float>(m_themeWheelEmpty->GetWidth()) / static_cast<float>(m_themeWheelEmpty->GetHeight());
               const ImVec2 pos(cx - cellSize.x * 0.5f, cy - cellSize.y * 0.5f);
               drawList->AddImage(
                  m_themeWheelEmpty, pos, ImVec2(pos.x + cellSize.x, pos.y + cellSize.y), ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), IM_COL32(255, 255, 255, static_cast<int>(alpha * 255.f)));

               // Table name over the placeholder, word wrapped once per entry then just rescaled:
               // measuring text at an arbitrary, constantly changing font size while the wheel slides
               // continuously bakes new font variants, which stutters the navigation
               WheelTitle &wt = m_wheelTitles[entry.tablePath];
               if (wt.size0 == 0.f && !entry.title.empty())
               {
                  string title = entry.title;
                  std::ranges::replace(title, '_', ' '); // '_' is common in table names and wraps poorly
                  ImFont *const font = ImGui::GetFont();
                  wt.size0 = 20.f;
                  const float wrap0 = wt.size0 * (cellSize.x * 0.8f) / (cellSize.y * 0.14f);
                  for (const char *p = title.c_str(), *const end = p + title.size(); p < end;)
                  {
                     const char *lineEnd = font->CalcWordWrapPosition(wt.size0, p, end, wrap0);
                     while (lineEnd > p && lineEnd[-1] == ' ')
                        --lineEnd;
                     if (lineEnd <= p)
                        lineEnd = p + 1; // always progress, even for a narrow cell or unbreakable word
                     wt.lines.emplace_back(p, lineEnd);
                     p = lineEnd;
                     while (p < end && (*p == ' ' || *p == '\n'))
                        ++p;
                  }

                  // Ellipsize the last line when the wrapped title does not fit in the cell height
                  const int maxLines = static_cast<int>(floorf(0.9f / 0.14f));
                  if (static_cast<int>(wt.lines.size()) > maxLines)
                  {
                     wt.lines.resize(maxLines);
                     string &last = wt.lines.back();
                     const float dotsW = font->CalcTextSizeA(wt.size0, FLT_MAX, 0, "...").x;
                     while (!last.empty() && (last.back() == ' ' || font->CalcTextSizeA(wt.size0, FLT_MAX, 0, last.c_str()).x + dotsW > wrap0))
                        last.pop_back();
                     last += "...";
                  }
                  wt.widths.reserve(wt.lines.size());
                  for (const string &line : wt.lines)
                     wt.widths.push_back(font->CalcTextSizeA(wt.size0, FLT_MAX, 0, line.c_str()).x);
               }
               ImFont *const font = ImGui::GetFont();
               const float fontSize = std::max(8.f, roundf(cellSize.y * 0.14f)); // whole pixels bound the font bake variants
               const float scale = fontSize / wt.size0;
               const ImU32 col = IM_COL32(255, 255, 255, static_cast<int>(alpha * 255.f));
               float ty = cy - wt.lines.size() * fontSize * 0.5f;
               for (size_t i = 0; i < wt.lines.size(); ++i, ty += fontSize)
                  drawList->AddText(font, fontSize, ImVec2(cx - wt.widths[i] * scale * 0.5f, ty), col, wt.lines[i].c_str());
            }
            else if (image == nullptr)
            {
               const ImVec2 iconSize = ImGui::CalcTextSize(ICON_LC_IMAGE);
               drawList->AddText(ImVec2(cx - iconSize.x * 0.5f, cy - iconSize.y * 0.5f), IM_COL32(90, 90, 110, static_cast<int>(alpha * 200.f)), ICON_LC_IMAGE);
            }
            if (image != nullptr)
            {
               cellSize.x = h * static_cast<float>(image->GetWidth()) / static_cast<float>(image->GetHeight());
               const ImVec2 pos(cx - cellSize.x * 0.5f, cy - cellSize.y * 0.5f);
               drawList->AddImage(
                  image, pos, ImVec2(pos.x + cellSize.x, pos.y + cellSize.y), ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), IM_COL32(255, 255, 255, static_cast<int>(alpha * artAlpha * 255.f)));
            }

            ImGui::SetCursorScreenPos(ImVec2(cx - cellSize.x * 0.5f, cy - cellSize.y * 0.5f));
            ImGui::PushID(k); // Id by slot: stable and unique even when the catalog wraps
            ImGui::InvisibleButton("##cell", cellSize);
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 4.f * uiScale))
               ApplyWheelDrag(io.MouseDelta.x, pxPerCell);
            if (ImGui::IsItemDeactivated())
            {
               const ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
               if (fabsf(drag.x) < 8.f * uiScale && fabsf(drag.y) < 8.f * uiScale)
               {
                  if (index == m_selected)
                     OpenActionsView();
                  else
                     m_selected = index;
               }
            }
            ImGui::PopID();
         }
      drawList->PopClipRect();

      // Prefetch a small margin past the visible items so scrolling feels instant
      for (int k = kMin - 3; k <= kMax + 3; k++)
      {
         if (k >= kMin && k <= kMax)
            continue; // already requested by the draw loop
         const TableCatalog::Entry &e = entries[((m_selected + k) % count + count) % count];
         if (e.HasMedia(TableCatalog::MediaType::Wheel) && !TableCatalog::IsAnimatedMediaFile(e.GetMedia(TableCatalog::MediaType::Wheel)))
            m_media->Get(e.GetMedia(TableCatalog::MediaType::Wheel)); // Stills only: animated medias open when the cell actually shows
      }
   }

   // Bottom chrome bar: the InGameUI desktop renderer's hint bar, drawn by its shared
   // rendering at the display's bottom edge — the theme layout does not place it
   DrawInfoBand();
}

void FrontendUI::DrawInfoBand()
{
   // The in-game UI chrome draws its own hint bar at the same place when it is open
   if (m_liveUI.IsInGameUIOpened())
      return;
   ImDrawList *const drawList = ImGui::GetWindowDrawList();
   const ImGuiIO &io = ImGui::GetIO();

   // Contextual hints following the navigation state, drawn by the InGameUI
   // desktop renderer's shared bottom bar (same band height and chip styling)
   vector<InGameUI::DesktopHint> hints { { "Flippers", "navigate" }, { "Magna", "letter" }, { "Start", "table actions" }, { "Launch", "play" }, { "Esc", "quit" } };
   if (IsVRAvailable())
      hints.insert(hints.end() - 1, { "Stereo", m_player->IsVR() ? "desktop" : "VR" });   InGameUI::RenderDesktopHintBar(m_player, drawList, ImVec2(0.f, io.DisplaySize.y - InGameUI::DesktopHintBarHeight(m_player)), io.DisplaySize.x, hints);
}

void FrontendUI::DrawInfoPane(const TableCatalog::Entry *const selected, const ImVec2 &pos, const ImVec2 &size)
{
   if (selected == nullptr)
      return;
   ImDrawList *const drawList = ImGui::GetWindowDrawList();
   const float uiScale = m_liveUI.GetDPI();
   const float pad = 10.f * uiScale;
   drawList->PushClipRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), true);
   float y = pos.y + pad;
   const float textW = size.x - 2.f * pad;

   // Table title: bold, underlined, shrunk until it fits on one line
   ImFont *const bold = m_liveUI.GetOverlayBoldFont();
   float fontSize = std::min(30.f * uiScale, size.y * 0.22f);
   ImVec2 titleSize = bold->CalcTextSizeA(fontSize, FLT_MAX, 0.f, selected->title.c_str());
   while (fontSize > 10.f * uiScale && titleSize.x > textW)
   {
      fontSize *= 0.85f;
      titleSize = bold->CalcTextSizeA(fontSize, FLT_MAX, 0.f, selected->title.c_str());
   }
   const ImVec2 titlePos(pos.x + pad, y);
   const ImU32 titleCol = IM_COL32(255, 255, 255, 255);
   drawList->AddText(bold, fontSize, titlePos, titleCol, selected->title.c_str());
   const float underlineY = titlePos.y + titleSize.y + 2.f * uiScale;
   drawList->AddLine(ImVec2(titlePos.x, underlineY), ImVec2(titlePos.x + std::min(titleSize.x, textW), underlineY), titleCol, std::max(1.f, 1.5f * uiScale));
   y = underlineY + pad;

   // Manufacturer and year of publishing
   ImFont *const font = ImGui::GetFont();
   const float lineSize = std::min(18.f * uiScale, size.y * 0.14f);
   const ImU32 dimCol = IM_COL32(170, 170, 190, 230);
   string sub;
   if (!selected->info.manufacturer.empty())
      sub = selected->info.manufacturer;
   if (selected->info.year != 0)
      sub += (sub.empty() ? ""s : " "s) + std::format("({})", selected->info.year);
   if (!sub.empty())
   {
      drawList->AddText(font, lineSize, ImVec2(pos.x + pad, y), dimCol, sub.c_str());
      y += lineSize * 1.3f;
   }

   if (!selected->info.authors.empty())
   {
      string authors = "by " + selected->info.authors.front();
      for (size_t i = 1; i < selected->info.authors.size(); i++)
         authors += ", " + selected->info.authors[i];
      drawList->AddText(font, lineSize, ImVec2(pos.x + pad, y), dimCol, authors.c_str());
      y += lineSize * 1.3f;
   }
   if (selected->vpx.startCount > 0)
   {
      const string runTime = FormatRunTime(selected->vpx.runTime);
      const string line = std::format("Played {} time{}{}{}", selected->vpx.startCount, selected->vpx.startCount == 1 ? "" : "s", runTime.empty() ? "" : " - total ", runTime);
      drawList->AddText(font, lineSize, ImVec2(pos.x + pad, y), dimCol, line.c_str());
      y += lineSize * 1.3f;
   }
   if (!selected->vpx.lastRun.empty())
   {
      const string line = "Last played: "s + selected->vpx.lastRun.substr(0, 10);
      drawList->AddText(font, lineSize, ImVec2(pos.x + pad, y), dimCol, line.c_str());
      y += lineSize * 1.3f;
   }
   if (selected->user.favorite || selected->user.rating > 0)
   {
      string line;
      if (selected->user.favorite)
         line = "Favorite";
      if (selected->user.rating > 0)
         line += (line.empty() ? ""s : "  "s) + std::format("Rating {}/10", selected->user.rating);
      drawList->AddText(font, lineSize, ImVec2(pos.x + pad, y), dimCol, line.c_str());
   }
   drawList->PopClipRect();
}

string FrontendUI::FormatRunTime(const int64_t seconds)
{
   if (seconds <= 0)
      return { };
   const int64_t hours = seconds / 3600;
   const int64_t minutes = (seconds % 3600) / 60;
   if (hours > 0)
      return std::format("{}h {:02}m", hours, minutes);
   return std::format("{}m", std::max<int64_t>(minutes, 1)); // Sub-minute sessions round up to avoid "0m"
}

}
