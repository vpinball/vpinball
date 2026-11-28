// license:GPLv3+

#pragma once

#include "core/TableCatalog.h"
#include "core/player.h"

#include "imgui/imgui.h"
#include "plugins/VPXPlugin.h"
#include "renderer/Window.h"

#include "FrontendMedia.h"
#include "FrontendTheme.h"

#include <deque>

class LiveUI;

namespace VPX::InGameUI
{
class LoadingPage;
}

namespace VPX::Frontend
{

class CatalogFolderWatcher;

// The mini frontend's main window, rendered with ImGui on top of a player session running on a
// synthetic empty table. For now it shows a text-only table wheel and a small action menu; media
// and presentation-specific layouts are layered on top in later steps.
class FrontendUI final
{
public:
   explicit FrontendUI(LiveUI &liveUI);
   ~FrontendUI();

   // Renders the frontend for the current frame (only called while a frontend session runs)
   void Update();

   // Back/escape request: closes the opened modals if any, returns true when the request was
   // consumed (a modal was open), false when the in-game UI chrome should be opened instead.
   bool OnNavigateBack();

   // Opens the in-game UI chrome on the TABLE view: the page proposing the actions
   // on the selected catalog entry (Play, Edit, media management, ...)
   void OpenActionsView();

   // Catalog actions (also the frontend's InGameUI "frontend/table" page)
   void Rescan();
   // Loads the selected catalog entry's table file asynchronously, the loading screen giving
   // feedback while it parses, then requests the host to end this frontend session and start
   // a play or edit session on it.
   void LaunchSession(Player::PlayMode playMode, const std::filesystem::path &tablePath = {});
   // Restarts the frontend session on the same synthetic table with a different VR mode, to enter
   // (ForceOn) or leave (ForceOff) XR. In XR, LiveUI is submitted to the HMD as a head locked quad
   // composition layer, so the whole frontend (wheel, media, overlays) shows inside the headset
   void RestartFrontendSession(Player::VROverride vr);
   bool IsVRAvailable() const { return m_vrAvailable || (m_player != nullptr && m_player->IsVR()); }
   bool HasSelection() const;
   const TableCatalog::Entry *GetSelected();
   void EnsureSelectedDetails();
   const TableCatalog::Entry *FindEntry(const std::filesystem::path &tablePath);

private:
   void HandleInput();

   // Edge detection with key repeat for a held action, following the InGameUI navigation pattern
   bool NavTriggered(const bool pressed, uint32_t &heldSince, uint32_t &lastTrigger);

   // Jumps to the first table of the previous/next title letter group (magna buttons)
   void JumpLetter(int dir);

   void SetSelectedIndex(int index, bool userInitiated);
   void RememberTablePath(const std::filesystem::path &tablePath, bool updateResumePath);
   void PersistNavigationPosition();

   // Applies a pointer drag delta to the wheel position and accumulates the release velocity
   void ApplyWheelDrag(float deltaX, float pxPerCell);

   // Presentation resolved from the view mode setting and the display orientation: cabinet shows
   // the playfield full window (other screens get media through the ancillary renderer), FSS/mobile
   // stacks backglass above the projected playfield, desktop keeps the three pane layout
   enum class Presentation
   {
      Desktop,
      Cabinet,
      FSS
   };
   Presentation ResolvePresentation() const;

   static string FormatRunTime(int64_t seconds);

   // Theming: when a theme is selected and provides a layout map for the window's aspect ratio, it
   // drives the whole layout: underlay, zone media, wheel band, info band, then overlay. When no
   // theme provides a layout, a default one matching the presentation is generated instead.
   // Theme and variant are reloaded when the selection, the theme folders or the aspect changes
   void UpdateTheme();
   void MakeDefaultLayout();
   void DrawThemedLayout(const TableCatalog::Entry *selected);
   // The layout's info pane: table title (bold, underlined), manufacturer and year, play stats
   void DrawInfoPane(const TableCatalog::Entry *selected, const ImVec2 &pos, const ImVec2 &size);
   // The desktop chrome's bottom hint bar: contextual navigation hints drawn as input chips
   // followed by help text, sized and styled by the shared desktop rendering
   void DrawInfoBand();
   vector<FrontendTheme::Entry> m_themes;
   string m_themeKey;                          // Settings + aspect bucket of the loaded theme variant
   FrontendTheme::Layout m_themeLayout;
   int m_themeImgW = 0, m_themeImgH = 0;       // Selected layout variant size
   std::shared_ptr<Sampler> m_themeUnderlay;
   std::shared_ptr<Sampler> m_themeOverlay;
   std::shared_ptr<Sampler> m_themeWheelEmpty; // Placeholder drawn in wheel cells without media
   std::filesystem::path m_themeUnderlayPath, m_themeOverlayPath, m_themeWheelEmptyPath; // Kept for the async load fade ins

   // Table name drawn over wheel placeholder cells, word wrapped once at a reference font size
   // (wrap and font scale together with the cell so the breaks stay valid), then just rescaled
   struct WheelTitle
   {
      string title;
      float size0 = 0.f;
      vector<string> lines;
      vector<float> widths; // Line widths at size0
   };
   std::unordered_map<std::filesystem::path, WheelTitle> m_wheelTitles;

   // Draws a still media image inside a box, keeping its aspect ratio (letterboxed, centered)
   // autoRotate turns landscape playfield media 270 degrees (clockwise); portrait media is left
   // untouched
   void DrawImage(const std::shared_ptr<Sampler> &image, const ImVec2 &boxPos, const ImVec2 &boxSize, float alpha = 1.f, bool autoRotate = false) const;
   // Cover fit: scales the image to fill the box, cropping the overflow (used for backgrounds).
   // autoRotate applies the playfield rotation rule (landscape media is turned 270 degrees)
   void DrawImageCover(const std::shared_ptr<Sampler> &image, const ImVec2 &boxPos, const ImVec2 &boxSize, float alpha = 1.f, bool autoRotate = false) const;
   // The blurred cover fitted background: the backglass, falling back to the playfield image
   void DrawBackground(const TableCatalog::Entry *selected) const;
   // Media preview panes crossfade from the currently shown image to the entry's one (kept shown
   // while the target image is still being decoded)
   struct PreviewPane
   {
      std::filesystem::path mediaPath; // Path of the media currently shown (crossfade target)
      std::shared_ptr<Sampler> current, previous; // Crossfading images
      uint32_t fadeStart = 0;
   };
   void DrawPreviewImage(PreviewPane &pane, const TableCatalog::Entry *selected, TableCatalog::MediaType type, const ImVec2 &pos,
      const ImVec2 &size, bool autoRotate = false);
   void DrawPreviewImage(PreviewPane &pane, const std::filesystem::path &mediaPath, const ImVec2 &pos, const ImVec2 &size,
      bool autoRotate = false);

   // Cabinet presentation: contributes the selected table's media to the backglass/score view/topper
   // windows through the plugin auxiliary renderer discovery broadcast
   void SetupAuxRenderer();
   static void OnGetAuxRenderer(const unsigned int msgId, void *userData, void *msgData);
   static int MSGPIAPI RenderAncillary(VPXRenderContext2D *ctx, void *context);
   VPXTexture GetAuxMedia(const std::filesystem::path &path);
   VPXTexture GetAuxTexture(const std::filesystem::path &path);
   VPXTexture GetAuxVideoTexture(const std::filesystem::path &path);

   LiveUI &m_liveUI;
   Player *m_player = nullptr; // Resolved lazily, after g_pplayer is initialized
   std::unique_ptr<FrontendMedia> m_media; // Created lazily, once the render device exists

   // The catalog scan runs fully in the background. Path batches are published while folders are
   // enumerated, a final path update removes stale cached entries, then invalidated companion/media
   // details are published in batches. The foreground merges these updates around the stable
   // selection path instead of waiting for the scan to finish.
   struct CatalogScanUpdate
   {
      enum class Type
      {
         Upsert,
         Reconcile
      };
      Type type = Type::Upsert;
      vector<TableCatalog::Entry> entries;
   };
   struct PendingCatalogScan
   {
      TableCatalog initial;
      std::mutex updateMutex;
      std::deque<CatalogScanUpdate> updates;
      std::atomic<bool> detailsReady { false };
      std::atomic<bool> abandoned { false };
   };
   void PollCatalogScan();
   void AdoptCatalog(TableCatalog &&catalog, bool restoreResumeSelection);
   void ApplyCatalogScanUpdate(const CatalogScanUpdate &update);
   void UpdateCatalogWatcher(const vector<std::filesystem::path> &roots);
   void PollCatalogWatcher();

   TableCatalog m_catalog;
   std::shared_ptr<PendingCatalogScan> m_catalogScan;
   bool m_scanned = false;
   bool m_catalogCacheLoadAttempted = false;
   std::unique_ptr<CatalogFolderWatcher> m_catalogWatcher;
   uint32_t m_catalogWatchEventSeen = 0;
   uint32_t m_catalogWatchDirtySince = 0;
   uint32_t m_catalogScanStartedAt = 0;
   int m_selected = 0;
   bool m_navigationDirty = false;
   uint32_t m_navigationSaveAt = 0;

   // XR: one time OpenXR headset probe (lazily on first frame) gating the Enter VR action
   uint32_t m_vrProbeAt = 0; // Next time a VR availability probe may be performed (retried until a runtime and HMD are found)
   bool m_vrAvailable = false;

   uint32_t m_prevHeldSince = 0, m_prevLastTrigger = 0;
   uint32_t m_nextHeldSince = 0, m_nextLastTrigger = 0;
   bool m_confirmWasPressed = false;
   bool m_launchWasPressed = false;
   bool m_leftMagnaWasPressed = false;
   bool m_rightMagnaWasPressed = false;
   int m_wheelNavDir = 0; // Held wheel navigation direction, 0 when none
   uint32_t m_wheelNavSince = 0; // Hold start of the current navigation direction
   bool m_wheelDragging = false; // A pointer drag is moving the wheel this frame (set by the drag surfaces)
   float m_wheelVel = 0.f; // Inertial scroll speed after a flick (entries/s), decays to a cell snap
   float m_wheelWheelAccum = 0.f; // Accumulated mouse wheel delta, converted to whole selection steps
   float m_wheelPos = 0.f; // Continuous wheel position (entry index), eased toward m_selected or scrolled on held navigation

   PreviewPane m_panes[3]; // Playfield, backglass, instruction card

   // An asynchronous table file load, started by a launch action and polled each frame in Update:
   // the worker thread parses the file then publishes the loaded table (null on failure) under
   // `done`; `abandoned` tells it the frontend is gone so it must release the result itself
   struct PendingTableLoad
   {
      std::atomic<PinTable *> table { nullptr };
      std::atomic<bool> done { false };
      std::atomic<bool> abandoned { false };
      Player::PlayMode playMode = Player::PlayMode::Play;
      Player::VROverride vr = Player::VROverride::Auto;
      string title;
   };
   void BeginTableLoad(const TableCatalog::Entry &entry, Player::PlayMode playMode, Player::VROverride vr);
   std::shared_ptr<PendingTableLoad> m_tableLoad;
   // The loading screen is rendered by the frontend itself while the file parses (the in-game UI
   // chrome stays closed, it would else swallow the frontend's input)
   std::unique_ptr<InGameUI::LoadingPage> m_loadingPage;

   const MsgPluginAPI *m_msgApi = nullptr;
   const VPXPluginAPI *m_vpxApi = nullptr;
   uint32_t m_endpointId = 0;
   unsigned int m_auxRendererMsgId = 0, m_auxRendererChgMsgId = 0;
   bool m_auxSubscribed = false;
   std::mutex m_auxTexMutex;
   std::unordered_map<std::filesystem::path, VPXTexture> m_auxTextures;
   struct AuxVideo
   {
      VPXTexture texture = nullptr; // Window texture updated from the video's latest frame
      uint64_t seq = 0; // Last uploaded frame sequence
   };
   std::unordered_map<std::filesystem::path, AuxVideo> m_auxVideos;
};

}
